#include <ai/action_controller_human.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


action_controller_human::action_controller_human(blackboard_ptr const  blackboard_)
    : action_controller(blackboard_)
    , m_template_cursor({
            {},                 // frames_in_world_space (initialised below)
            "idle.stand",       // template_name
            1U,                 // keyframe_index
            true                // repeat
            })
    , m_template_motion_info()  // initialised below
{
    skeletal_motion_templates::keyframes const&  animation =
            get_blackboard()->m_motion_templates->motions_map.at(m_template_cursor.name);

    infer_parent_frame_from_local_and_world_frames(
            get_blackboard()->m_skeleton_composition->pose_frames.front(),
            get_blackboard()->m_frames.front(),
            m_template_cursor.frame_in_world_space
            );

    m_template_motion_info.current_frames_in_world_space = get_blackboard()->m_frames;
    transform_skeleton_coord_systems_from_common_reference_frame_to_world(
            animation.keyframe_at(m_template_cursor.keyframe_index - 1U).get_coord_systems(),
            m_template_cursor.frame_in_world_space,
            m_template_motion_info.current_frames_in_world_space
            );
    transform_skeleton_coord_systems_from_common_reference_frame_to_world(
            animation.keyframe_at(m_template_cursor.keyframe_index).get_coord_systems(),
            m_template_cursor.frame_in_world_space,
            m_template_motion_info.dst_frames_in_world_space
            );
    m_template_motion_info.time_to_reach_dst_frames_in_seconds =
            animation.keyframe_at(m_template_cursor.keyframe_index).get_time_point() -
            animation.keyframe_at(m_template_cursor.keyframe_index - 1U).get_time_point();
}


void  action_controller_human::next_round(float_32_bit  time_step_in_seconds)
{
    TMPROF_BLOCK();

    update_motion_using_templates(
            time_step_in_seconds,
            *get_blackboard()->m_motion_templates,
            m_template_motion_info,
            m_template_cursor
            );
    transform_skeleton_coord_systems_from_world_to_local_space(
            m_template_motion_info.current_frames_in_world_space,
            get_blackboard()->m_skeleton_composition->parents,
            get_blackboard()->m_frames
            );
}


}
