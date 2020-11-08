#include <ai/skeleton_interpolators.hpp>
#include <ai/skeleton_utils.hpp>
#include <com/simulation_context.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


skeleton_interpolator_animation::skeleton_interpolator_animation()
    : m_src_frames()
    , m_current_frames()
    , m_dst_frames()
    , m_src_offset(vector3_zero())
    , m_current_offset(vector3_zero())
    , m_dst_offset(vector3_zero())
{}


void  skeleton_interpolator_animation::interpolate(float_32_bit const  interpolation_param)
{
    interpolate_keyframes_spherical(m_src_frames, m_dst_frames, interpolation_param, m_current_frames);
    m_current_offset = interpolate_linear(m_src_offset, m_dst_offset, interpolation_param);
}


void  skeleton_interpolator_animation::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        skeletal_motion_templates const  motion_templates
        )
{
    m_src_frames = m_current_frames;
    transform_keyframes_to_reference_frame(
            motion_templates.at(cursor.motion_name).keyframes.get_keyframes().at(cursor.keyframe_index).get_coord_systems(),
            *motion_templates.at(cursor.motion_name).keyframes.from_bones_to_indices(),
            motion_templates.at(cursor.motion_name).reference_frames.at(cursor.keyframe_index),
            motion_templates.pose_frames().get_coord_systems(),
            motion_templates.parents(),
            m_dst_frames,
            true
            );

    m_src_offset = m_current_offset;
    m_dst_offset = motion_templates.at(cursor.motion_name).keyframes.coord_system_at(cursor.keyframe_index, 0U).origin() -
                   motion_templates.at(cursor.motion_name).reference_frames.at(cursor.keyframe_index).origin();
}


void  skeleton_interpolator_animation::move_to_target()
{
    m_src_frames = m_current_frames = m_dst_frames;
    m_src_offset = m_current_offset = m_dst_offset;
}


void  skeleton_interpolator_animation::commit(
        skeletal_motion_templates const  motion_templates,
        scene_binding const&  binding
        ) const
{
    auto const&  parents = motion_templates.parents();
    for (natural_32_bit bone = 0; bone != m_current_frames.size(); ++bone)
        binding.context->request_relocate_frame(
                binding.frame_guids_of_bones.at(bone),
                parents.at(bone) < 0 ? m_current_offset : m_current_frames.at(bone).origin(),
                m_current_frames.at(bone).orientation()
                );
}


skeleton_interpolator_look_at::skeleton_interpolator_look_at()
    : m_joint_rotations()
    , m_look_at_target_in_local_space(vector3_zero())
    , m_target_pose_reached(true)
{}


void  skeleton_interpolator_look_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        vector3 const&  look_at_target_in_skeleton_space,
        std::vector<angeo::coordinate_system>&  frames_to_update,
        skeletal_motion_templates const  motion_templates
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames_to_update.size() == motion_templates.names().size());

    auto const&  parents = motion_templates.parents();

    tranform_matrices_of_skeleton_bones const  bone_matrices(&frames_to_update, &parents);

    std::vector<angeo::skeleton_kinematics_of_chain_of_bones>  kinematics;
    std::vector< angeo::look_at_target>  look_at_targets;
    std::unordered_map<natural_32_bit, angeo::coordinate_system*>  output_frame_pointers;
    for (auto const&  name_and_info : motion_templates.look_at())
    {
        std::unordered_map<natural_32_bit, angeo::coordinate_system const*>  pose_frame_pointers;
        for (natural_32_bit  bone : name_and_info.second->all_bones)
        {
            pose_frame_pointers.insert({ bone, &motion_templates.pose_frames().get_coord_systems().at(bone) });
            output_frame_pointers.insert({ bone, &frames_to_update.at(bone) });
        }

        kinematics.push_back({
                pose_frame_pointers,
                name_and_info.second->all_bones,
                name_and_info.second->end_effector_bone,
                motion_templates.joints(),
                parents,
                motion_templates.lengths()
                });

        angeo::look_at_target  target;
        {
            target.target = parents.at(name_and_info.second->root_bone) == -1 ?
                    look_at_target_in_skeleton_space :
                    transform_point(
                            look_at_target_in_skeleton_space,
                            bone_matrices.from_skeleton_to_bone_space_matrix(parents.at(name_and_info.second->root_bone))
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


skeleton_interpolator_aim_at::skeleton_interpolator_aim_at()
{}


void  skeleton_interpolator_aim_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        vector3 const&  aim_at_target_in_skeleton_space,
        std::vector<angeo::coordinate_system>&  frames_to_update,
        skeletal_motion_templates const  motion_templates
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames_to_update.size() == motion_templates.names().size());
    
    auto const&  parents = motion_templates.parents();

    tranform_matrices_of_skeleton_bones const  bone_matrices(&frames_to_update, &parents);

    for (auto const&  name_and_info : motion_templates.aim_at())
    {
        std::unordered_map<natural_32_bit, angeo::coordinate_system const*>  pose_frame_pointers;
        std::unordered_map<natural_32_bit, angeo::coordinate_system*>  output_frame_pointers;
        for (natural_32_bit  bone : name_and_info.second->all_bones)
        {
            pose_frame_pointers.insert({ bone, &motion_templates.pose_frames().get_coord_systems().at(bone) });
            output_frame_pointers.insert({ bone, &frames_to_update.at(bone) });
        }

        angeo::skeleton_kinematics_of_chain_of_bones  kinematics(
                pose_frame_pointers,
                name_and_info.second->all_bones,
                name_and_info.second->end_effector_bone,
                motion_templates.joints(),
                parents,
                motion_templates.lengths()
                );

        angeo::bone_aim_at_targets  aim_at_targets;
        {
            vector3 const  target = parents.at(name_and_info.second->root_bone) == -1 ?
                    aim_at_target_in_skeleton_space :
                    transform_point(
                            aim_at_target_in_skeleton_space,
                            bone_matrices.from_skeleton_to_bone_space_matrix(parents.at(name_and_info.second->root_bone))
                            );
            aim_at_targets.push_back(angeo::aim_at_target{ target, name_and_info.second->touch_points.at("pointer") });
        }

        angeo::skeleton_aim_at(kinematics, aim_at_targets);

        kinematics.commit_target_frames(output_frame_pointers);
    }
}


}