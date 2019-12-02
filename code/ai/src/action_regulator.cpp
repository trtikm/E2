#include <ai/action_regulator.hpp>
#include <ai/action_controller.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai {


action_regulator::action_regulator(action_controller const* const  controller_ptr)
    : m_controller(controller_ptr)
    , m_motion_desire_props()
    , m_look_at_target_info()
{
    ASSUMPTION(controller_ptr != nullptr);
}


void  action_regulator::initialise()
{
    set_stationary_desire(m_motion_desire_props, m_controller->get_blackboard());
    m_look_at_target_info.target_pose_reached = true;
}


void  action_regulator::next_round(motion_desire_props const&  motion_desire_props_of_the_cortex)
{
    TMPROF_BLOCK();

    m_motion_desire_props.forward_unit_vector_in_local_space = motion_desire_props_of_the_cortex.forward_unit_vector_in_local_space;
    m_motion_desire_props.linear_velocity_unit_direction_in_local_space = motion_desire_props_of_the_cortex.linear_velocity_unit_direction_in_local_space;
    m_motion_desire_props.linear_speed = motion_desire_props_of_the_cortex.linear_speed;
    m_motion_desire_props.angular_velocity_unit_axis_in_local_space = motion_desire_props_of_the_cortex.angular_velocity_unit_axis_in_local_space;
    m_motion_desire_props.angular_speed = motion_desire_props_of_the_cortex.angular_speed;

    update_look_at_target(motion_desire_props_of_the_cortex.look_at_target_in_local_space);
}


void  action_regulator::update_look_at_target(vector3 const&  look_at_target_from_cortex)
{
    ai::sensory_controller_sight_ptr const  sight = m_controller->get_blackboard()->m_sensory_controller->get_sight();
    if (sight == nullptr)
        return;
    ai::sensory_controller_sight::camera_perspective_ptr const  camera = sight->get_camera();
    if (camera == nullptr)
        return;

    matrix44  M;
    angeo::to_base_matrix(m_controller->get_motion_object_motion().frame, M);

    vector3 const  camera_origin_in_local_space = transform_point(camera->coordinate_system()->origin(), M);
    if (length(look_at_target_from_cortex - camera_origin_in_local_space) < camera->near_plane())
        return;

    if (m_look_at_target_info.target_pose_reached)
    {
        m_look_at_target_info.target_pose_reached = false;
        m_motion_desire_props.look_at_target_in_local_space = look_at_target_from_cortex;
        return;
    }

    vector3 const  target_dir = m_motion_desire_props.look_at_target_in_local_space - camera_origin_in_local_space;
    vector3 const  camera_forward_dir = transform_vector(-angeo::axis_z(*camera->coordinate_system()), M);
    vector3 const  rot_axis = cross_product(camera_forward_dir, target_dir);
    vector3 const  clip_planes_normals[2] = { cross_product(rot_axis, camera_forward_dir), cross_product(target_dir, rot_axis) };
    vector3 const  cortex_target_dir = look_at_target_from_cortex - camera_origin_in_local_space;

    if (dot_product(clip_planes_normals[0], cortex_target_dir) <= 0.0f)
        return; // The cortex's target is on the opposite way to the current target.
    if (dot_product(clip_planes_normals[1], cortex_target_dir) <= 0.0f)
        return; // The cortex's target will be reached AFTER the current target.

    float_32_bit const  rot_axes_difference = angle(rot_axis, cross_product(camera_forward_dir, cortex_target_dir));

    if (rot_axes_difference > PI() * 5.0f / 180.0f)
        return; // The cortex's target is too far from the rotation plane of camera to the current targer.

    // The cortex's target lies between camera and current target in the proximity of the rotation plane.
    // So, it seems to be useful to change to the proposed cortex's target.
    m_look_at_target_info.target_pose_reached = false;
    m_motion_desire_props.look_at_target_in_local_space = look_at_target_from_cortex;
}


}
