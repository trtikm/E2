#include <ai/detail/action_controller_interpolator_look_at.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_look_at::action_controller_interpolator_look_at(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
        )
    : action_controller_interpolator_base(motion_templates, binding)
    , m_joint_rotations()
    , m_look_at_target_in_local_space(vector3_zero())
    , m_target_pose_reached(true)
{
}


void  action_controller_interpolator_look_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  interpolation_param,
        vector3 const&  look_at_target_in_agent_space,
        std::vector<angeo::coordinate_system>&  frames_to_update
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames_to_update.size() == get_motion_templates().names().size());

    auto const&  parents = get_motion_templates().parents();

    std::vector<angeo::skeleton_kinematics_of_chain_of_bones>  kinematics;
    std::vector< angeo::look_at_target>  look_at_targets;
    std::unordered_map<natural_32_bit, angeo::coordinate_system*>  output_frame_pointers;
    for (auto const&  name_and_info : get_motion_templates().look_at())
    {
        std::unordered_map<natural_32_bit, angeo::coordinate_system const*>  pose_frame_pointers;
        for (natural_32_bit  bone : name_and_info.second->all_bones)
        {
            pose_frame_pointers.insert({ bone, &get_motion_templates().pose_frames().get_coord_systems().at(bone) });
            output_frame_pointers.insert({ bone, &frames_to_update.at(bone) });
        }

        kinematics.push_back({
                pose_frame_pointers,
                name_and_info.second->all_bones,
                name_and_info.second->end_effector_bone,
                get_motion_templates().joints(),
                parents,
                get_motion_templates().lengths()
                });

        angeo::look_at_target  target;
        {
            angeo::coordinate_system_explicit  chain_world_frame;
            if (parents.at(name_and_info.second->root_bone) != -1)
                chain_world_frame = ctx().frame_explicit_coord_system_in_world_space(
                        get_frame_guids_of_bones().at(parents.at(name_and_info.second->root_bone))
                        );

            target.target = angeo::point3_to_coordinate_system(
                                    angeo::point3_from_coordinate_system(
                                            look_at_target_in_agent_space,
                                            ctx().frame_coord_system_in_world_space(get_frame_guid_of_skeleton())
                                            ),
                                    chain_world_frame
                                    );
            target.direction = name_and_info.second->direction;
        }
        look_at_targets.push_back(target);
    }

    angeo::skeleton_look_at(kinematics, look_at_targets);

    angeo::joint_angle_deltas_of_bones  angle_deltas;
    angeo::joint_rotation_props_of_bones  joint_definitions;
    for (angeo::skeleton_kinematics_of_chain_of_bones const&  kin : kinematics)
        for (auto const&  bone_and_states : kin.joint_states)
        {
            auto  it = m_joint_rotations.find(bone_and_states.first);
            if (it == m_joint_rotations.end())
                it = m_joint_rotations.insert(bone_and_states).first;
            auto const  def_it = kin.joint_definitions.find(bone_and_states.first);
            angeo::skeleton_interpolate_joint_rotation_states(
                    angle_deltas[bone_and_states.first],
                    it->second,
                    bone_and_states.second,
                    def_it->second,
                    time_step_in_seconds
                    );
            joint_definitions.insert(*def_it);
        }
    angeo::skeleton_apply_angle_deltas(m_joint_rotations, angle_deltas, joint_definitions);
    angeo::skeleton_commit_joint_states_to_frames(output_frame_pointers, m_joint_rotations);
}


void  action_controller_interpolator_look_at::update_look_at_target_in_local_space(
        vector3 const&  look_at_target_from_cortex,
        angeo::coordinate_system_explicit const&  agent_frame,
        ai::sight_controller::camera_perspective_ptr const  camera
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
