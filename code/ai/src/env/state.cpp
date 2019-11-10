#include <ai/env/state.hpp>
#include <ai/cortex.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_sight.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <angeo/tensor_math.hpp>
#include <angeo/coordinate_system.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <vector>
#include <algorithm>

namespace ai { namespace env {


state::state(blackboard_const_ptr const  blackboard_ptr)
    : is_valid(true)
{
    TMPROF_BLOCK();

    // First we collect refs to source data.

    vector3 const*  desired_forward_unit_vector_in_world_space_ptr;
    vector3 const*  desired_linear_velocity_unit_direction_in_world_space_ptr;
    float_32_bit  desired_linear_speed;
    vector3 const*  desired_angular_velocity_unit_axis_in_world_space_ptr;
    float_32_bit  desired_angular_speed;
    angeo::coordinate_system const*  frame_in_world_space_ptr;
    vector3 const*  forward_in_world_space_ptr;
    vector3 const*  up_in_world_space_ptr;
    vector3 const*  linear_velocity_in_world_space_ptr;
    vector3 const*  angular_velocity_in_world_space_ptr;
    vector3 const*  linear_acceleration_in_world_space_ptr;
    vector3 const*  angular_acceleration_in_world_space_ptr;
    vector3 const*  gravity_in_world_space_ptr;
    vector3 const*  ideal_linear_velocity_in_world_space_ptr;
    vector3 const*  ideal_angular_velocity_in_world_space_ptr;
    angeo::coordinate_system const*  camera_frame_in_world_space_ptr;
    std::vector<sensory_controller_ray_cast_sight::ray_cast_info const*>  sight_ray_casts_in_world_space;
    std::vector<scene::collicion_contant_info const*>  contacts_in_world_space;
    {
        detail::motion_desire_props const&  desire = blackboard_ptr->m_cortex->get_motion_desire_props();
        desired_forward_unit_vector_in_world_space_ptr = &desire.forward_unit_vector_in_world_space;
        desired_linear_velocity_unit_direction_in_world_space_ptr = &desire.linear_velocity_unit_direction_in_world_space;
        desired_linear_speed = desire.linear_speed;
        desired_angular_velocity_unit_axis_in_world_space_ptr = &desire.angular_velocity_unit_axis_in_world_space;
        desired_angular_speed = desire.angular_speed;

        detail::rigid_body_motion const&  motion = blackboard_ptr->m_action_controller->get_motion_object_motion();
        frame_in_world_space_ptr = &motion.frame;
        forward_in_world_space_ptr = &motion.forward;
        up_in_world_space_ptr = &motion.up;
        linear_velocity_in_world_space_ptr = &motion.velocity.m_linear;
        angular_velocity_in_world_space_ptr = &motion.velocity.m_angular;
        linear_acceleration_in_world_space_ptr = &motion.acceleration.m_linear;
        angular_acceleration_in_world_space_ptr = &motion.acceleration.m_angular;
        gravity_in_world_space_ptr = &blackboard_ptr->m_action_controller->get_gravity_acceleration();
        ideal_linear_velocity_in_world_space_ptr = &blackboard_ptr->m_action_controller->get_ideal_linear_velocity_in_world_space();
        ideal_angular_velocity_in_world_space_ptr = &blackboard_ptr->m_action_controller->get_ideal_angular_velocity_in_world_space();

        ASSUMPTION(blackboard_ptr->m_sensory_controller->get_sight() != nullptr);
        sensory_controller_ray_cast_sight_const_ptr const  sight_ptr = as<sensory_controller_ray_cast_sight const>(blackboard_ptr->m_sensory_controller->get_sight());
        ASSUMPTION(sight_ptr != nullptr);
        camera_frame_in_world_space_ptr = sight_ptr->get_camera()->coordinate_system().get();
        for (auto const&  time_and_ray_cast_info : sight_ptr->get_ray_casts_as_performed_in_time())
            sight_ray_casts_in_world_space.push_back(&time_and_ray_cast_info.second);

        auto const  begin_and_end = blackboard_ptr->m_collision_contacts.equal_range(motion.nid);
        for (auto it = begin_and_end.first; it != begin_and_end.second; ++it)
            if (it->second.normal_force_magnitude > 0.001f)
                contacts_in_world_space.push_back(&it->second);
        std::sort(contacts_in_world_space.begin(), contacts_in_world_space.end(),
                  [](scene::collicion_contant_info const* const  left, scene::collicion_contant_info const* const  right) {
                        return left->normal_force_magnitude > right->normal_force_magnitude;
                        });
    }

    // Next we express all the data in the local frame of the agent.

    vector3  desired_forward_unit_vector;
    vector3  desired_linear_velocity_unit_direction;
    //float_32_bit  desired_linear_speed;   // we already have this one
    vector3  desired_angular_velocity_unit_axis;
    //float_32_bit  desired_angular_speed;  // we already have this one
    vector3  forward;
    vector3  up;
    vector3  linear_velocity;
    vector3  angular_velocity;
    vector3  linear_acceleration;
    vector3  angular_acceleration;
    vector3  gravity;
    vector3  ideal_linear_velocity;
    vector3  ideal_angular_velocity;
    vector3  camera_origin;
    vector3  camera_x_axis;
    vector3  camera_y_axis;
    vector3  camera_z_axis;
    std::vector<vector3>  ray_cast_targets;
    std::vector<vector3>  contact_points;
    {
        matrix44  to_agent_space_matrix;
        angeo::to_base_matrix(*frame_in_world_space_ptr, to_agent_space_matrix);

        desired_forward_unit_vector = transform_vector(*desired_forward_unit_vector_in_world_space_ptr, to_agent_space_matrix);
        desired_linear_velocity_unit_direction = transform_vector(*desired_linear_velocity_unit_direction_in_world_space_ptr, to_agent_space_matrix);
        desired_angular_velocity_unit_axis = transform_vector(*desired_angular_velocity_unit_axis_in_world_space_ptr, to_agent_space_matrix);
        forward = transform_vector(*forward_in_world_space_ptr, to_agent_space_matrix);
        up = transform_vector(*up_in_world_space_ptr, to_agent_space_matrix);
        linear_velocity = transform_vector(*linear_velocity_in_world_space_ptr, to_agent_space_matrix);
        angular_velocity = transform_vector(*angular_velocity_in_world_space_ptr, to_agent_space_matrix);
        linear_acceleration = transform_vector(*linear_acceleration_in_world_space_ptr, to_agent_space_matrix);
        angular_acceleration = transform_vector(*angular_acceleration_in_world_space_ptr, to_agent_space_matrix);
        gravity = transform_vector(*gravity_in_world_space_ptr, to_agent_space_matrix);
        ideal_linear_velocity = transform_vector(*ideal_linear_velocity_in_world_space_ptr, to_agent_space_matrix);
        ideal_angular_velocity = transform_vector(*ideal_angular_velocity_in_world_space_ptr, to_agent_space_matrix);
        camera_origin = transform_point(camera_frame_in_world_space_ptr->origin(), to_agent_space_matrix);
        camera_x_axis = transform_vector(angeo::axis_x(*frame_in_world_space_ptr), to_agent_space_matrix);
        camera_y_axis = transform_vector(angeo::axis_y(*frame_in_world_space_ptr), to_agent_space_matrix);
        camera_z_axis = transform_vector(angeo::axis_z(*frame_in_world_space_ptr), to_agent_space_matrix);
        for (auto  ptr : sight_ray_casts_in_world_space)
            ray_cast_targets.push_back(
                    transform_point(ptr->ray_origin + ptr->parameter_to_coid * ptr->ray_unit_direction, to_agent_space_matrix)
                    );
        for (auto  ptr : contacts_in_world_space)
            contact_points.push_back(transform_point(ptr->contact_point, to_agent_space_matrix));
    }

    // TODO...
}


}}
