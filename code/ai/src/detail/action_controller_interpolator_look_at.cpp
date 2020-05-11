#include <ai/detail/action_controller_interpolator_look_at.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_look_at::action_controller_interpolator_look_at(
        action_controller_interpolator const* const  interpolator_,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
        )
    : action_controller_interpolator_shared(interpolator_)

    , m_src_bones()
    , m_current_bones()
    , m_dst_bones()

    , m_look_at_target_in_local_space(vector3_zero())
    , m_target_pose_reached(true)
{
    set_target(initial_template_cursor);
    m_src_bones = m_current_bones = m_dst_bones;
}


void  action_controller_interpolator_look_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  interpolation_param,
        vector3 const&  look_at_target_in_agent_space,
        angeo::coordinate_system_explicit const&  agent_frame,
        std::vector<angeo::coordinate_system>&  frames_to_update
        )
{
    TMPROF_BLOCK();

    m_current_bones = (interpolation_param < 0.5f) ? m_src_bones : m_dst_bones;

    if (m_current_bones == nullptr || m_dst_bones == nullptr)
        return;
    if (m_current_bones->all_bones.empty() && m_dst_bones->all_bones.empty())
        return;

    ai::sensory_controller_sight_ptr const  sight = get_blackboard()->m_sensory_controller->get_sight();
    if (sight == nullptr)
        return;
    ai::sensory_controller_sight::camera_perspective_ptr const  camera = sight->get_camera();
    if (camera == nullptr)
        return;

    update_look_at_target_in_local_space(look_at_target_in_agent_space, agent_frame, camera);

    // We setup data for the look-at algo.

    std::unordered_set<integer_32_bit>  bones_to_consider;
    for (auto bone : m_current_bones->all_bones)
        bones_to_consider.insert(bone);
    if (m_current_bones != m_dst_bones)
        for (auto bone : m_dst_bones->all_bones)
            bones_to_consider.insert(bone);

    auto const&  parents = get_blackboard()->m_motion_templates.parents();

    vector3  target;
    {
        target = m_look_at_target_in_local_space;

        // The target is now in agent's local space, but we need the target in the space of the bone which is the closest parent bone
        // to all bones in 'bones_to_consider'; note that the parent bone cannot be in 'bones_to_consider'.

        integer_32_bit  closest_parent_bone = *bones_to_consider.begin();
        for (; bones_to_consider.count(closest_parent_bone) != 0ULL; closest_parent_bone = parents.at(closest_parent_bone))
            ;

        angeo::coordinate_system  closest_parent_bone_frame;
        get_blackboard()->m_scene->get_frame_of_scene_node(
                get_blackboard()->m_bone_nids.at(closest_parent_bone),
                false,
                closest_parent_bone_frame
                );

        matrix44  T;
        angeo::to_base_matrix(closest_parent_bone_frame, T);
        matrix44  F;
        angeo::from_base_matrix(agent_frame, F);

        target = transform_point(target, T * F);
    }

    angeo::bone_look_at_targets  look_at_targets;
    for (auto bone : m_current_bones->end_effector_bones)
        look_at_targets.insert({ (integer_32_bit)bone, { vector3_unit_y(), target } });
    if (m_current_bones != m_dst_bones)
        for (auto bone : m_dst_bones->end_effector_bones)
            look_at_targets.insert({ (integer_32_bit)bone, { vector3_unit_y(), target } });

    // Now execute the look-at algo for the prepared data.

    std::vector<angeo::coordinate_system>  target_frames;
    std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >  bones_to_rotate;
    angeo::skeleton_look_at(
            target_frames,
            look_at_targets,
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            parents,
            get_blackboard()->m_motion_templates.joints(),
            bones_to_consider,
            &bones_to_rotate
            );

    std::vector<angeo::coordinate_system>  frames;
    frames.resize(get_blackboard()->m_motion_templates.names().size());
    for (auto const& bone_and_anges : bones_to_rotate)
        get_blackboard()->m_scene->get_frame_of_scene_node(
            get_blackboard()->m_bone_nids.at(bone_and_anges.first),
            true,
            frames.at(bone_and_anges.first)
            );

    bool const target_pose_reached = angeo::skeleton_rotate_bones_towards_target_pose(
            frames,
            target_frames,
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            get_blackboard()->m_motion_templates.joints(),
            bones_to_rotate,
            time_step_in_seconds
            );
    if (target_pose_reached)
        m_target_pose_reached = true;;

    // And write results to the vector 'm_current_intepolation_state.frames' of final frames.

    float_32_bit const  src_param = m_current_bones->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  dst_param = m_dst_bones->all_bones.empty() ? 0.0f : 1.0f;
    float_32_bit const  param = src_param + interpolation_param * (dst_param - src_param);
    for (auto const&  bone_and_rotations : bones_to_rotate)
        angeo::interpolate_spherical(
            frames_to_update.at(bone_and_rotations.first),
            frames.at(bone_and_rotations.first),
            param,
            frames_to_update.at(bone_and_rotations.first)
            );
}


void  action_controller_interpolator_look_at::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor
        )
{
    m_src_bones = m_current_bones;
    m_dst_bones = get_blackboard()->m_motion_templates.motions_map().at(cursor.motion_name).look_at.at(cursor.keyframe_index);
}


void  action_controller_interpolator_look_at::update_look_at_target_in_local_space(
        vector3 const&  look_at_target_from_cortex,
        angeo::coordinate_system_explicit const&  agent_frame,
        ai::sensory_controller_sight::camera_perspective_ptr const  camera
        )
{
    matrix44  M;
    angeo::to_base_matrix(agent_frame, M);

    vector3 const  camera_origin_in_local_space = transform_point(camera->coordinate_system()->origin(), M);
    if (length(look_at_target_from_cortex - camera_origin_in_local_space) < camera->near_plane())
        return;

    if (m_target_pose_reached)
    {
        m_target_pose_reached = false;
        m_look_at_target_in_local_space = look_at_target_from_cortex;
        return;
    }

    vector3 const  target_dir = m_look_at_target_in_local_space - camera_origin_in_local_space;
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
    m_target_pose_reached = false;
    m_look_at_target_in_local_space = look_at_target_from_cortex;
}


}}
