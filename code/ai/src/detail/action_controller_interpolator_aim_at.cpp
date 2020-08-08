#include <ai/detail/action_controller_interpolator_aim_at.hpp>
#include <ai/skeleton_utils.hpp>
#include <com/simulation_context.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_aim_at::action_controller_interpolator_aim_at(
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
        )
    : action_controller_interpolator_base(motion_templates, binding)
    , m_src_infos()
    , m_current_infos()
    , m_dst_infos()
{
    set_target(initial_template_cursor);
    m_src_infos = m_current_infos = m_dst_infos;
}


void  action_controller_interpolator_aim_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  interpolation_param,
        vector3 const&  aim_at_target_in_agent_space,
        std::vector<angeo::coordinate_system>&  frames_to_update
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(frames_to_update.size() == get_motion_templates().names().size());
    
    auto const&  parents = get_motion_templates().parents();

    m_current_infos = (interpolation_param < 0.5f) ? m_src_infos : m_dst_infos;
    for (skeletal_motion_templates::aim_at_info_ptr  aim_at_ptr : m_current_infos)
    {
        if (aim_at_ptr->touch_points.count("pointer") == 0UL)
            continue;

        std::unordered_map<natural_32_bit, angeo::coordinate_system const*>  pose_frame_pointers;
        std::unordered_map<natural_32_bit, angeo::coordinate_system*>  output_frame_pointers;
        for (natural_32_bit  bone : aim_at_ptr->all_bones)
        {
            pose_frame_pointers.insert({ bone, &get_motion_templates().pose_frames().get_coord_systems().at(bone) });
            output_frame_pointers.insert({ bone, &frames_to_update.at(bone) });
        }

        angeo::skeleton_kinematics_of_chain_of_bones  kinematics(
                pose_frame_pointers,
                aim_at_ptr->all_bones,
                aim_at_ptr->end_effector_bone,
                get_motion_templates().joints(),
                parents,
                get_motion_templates().lengths()
                );

        angeo::bone_aim_at_targets  aim_at_targets;
        {
            angeo::coordinate_system_explicit  chain_world_frame;
            if (parents.at(aim_at_ptr->root_bone) != -1)
                chain_world_frame = ctx().frame_explicit_coord_system_in_world_space(
                        get_frame_guids_of_bones().at(parents.at(aim_at_ptr->root_bone))
                        );

            vector3  target = angeo::point3_to_coordinate_system(
                                    angeo::point3_from_coordinate_system(
                                            aim_at_target_in_agent_space,
                                            ctx().frame_coord_system_in_world_space(get_frame_guid_of_skeleton())
                                            ),
                                    chain_world_frame
                                    );

            aim_at_targets.push_back(angeo::aim_at_target{ target, aim_at_ptr->touch_points.at("pointer") });
        }

        angeo::skeleton_aim_at(kinematics, aim_at_targets);

        kinematics.commit_target_frames(output_frame_pointers);
    }
}


void  action_controller_interpolator_aim_at::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor
        )
{
    m_src_infos = m_current_infos;
    m_dst_infos.clear();
    for (std::string const&  name :
         get_motion_templates().motions_map().at(cursor.motion_name).aim_at.at(cursor.keyframe_index))
    {
        auto const  it = get_motion_templates().aim_at().find(name);
        if (it != get_motion_templates().aim_at().end())
            m_dst_infos.push_back(it->second);
    }
}


}}
