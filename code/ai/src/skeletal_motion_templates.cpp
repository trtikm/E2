#include <ai/skeletal_motion_templates.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


skeletal_motion_templates::skeletal_motion_templates()
    : motions_map()
    , is_loaded(false)
{}


bool  skeletal_motion_templates::is_ready() const
{
    if (is_loaded == false)
    {
        for (auto const&  elem : motions_map)
            if (!elem.second.loaded_successfully())
                return false;
        is_loaded = true;
    }
    return true;
}


bool  skeletal_motion_templates::wait_till_loaded_is_finished() const
{
    if (is_loaded == false)
    {
        for (auto const& elem : motions_map)
        {
            while (!elem.second.is_load_finished()) {}
            if (!elem.second.loaded_successfully())
                return false;
        }
        is_loaded = true;
    }
    return true;
}


}

namespace ai {


void  update_motion_using_templates(
        float_32_bit  time_step_in_seconds,
        skeletal_motion_templates const&  smt,
        skeletal_motion_templates::template_motion_info&  motion_info,
        skeletal_motion_templates::template_cursor&  cursor
        )
{
    TMPROF_BLOCK();

    if (motion_info.time_to_reach_dst_frames_in_seconds >= time_step_in_seconds)
    {
        interpolate_keyframes(
                motion_info.current_frames_in_world_space,
                motion_info.dst_frames_in_world_space,
                time_step_in_seconds / motion_info.time_to_reach_dst_frames_in_seconds,
                motion_info.current_frames_in_world_space
                );
        motion_info.time_to_reach_dst_frames_in_seconds -= time_step_in_seconds;
        return;
    }

    skeletal_motion_templates::keyframes const&  animation = smt.motions_map.at(cursor.name);

    if (cursor.keyframe_index + 1U == animation.get_keyframes().size() && cursor.repeat == false)
    {
        motion_info.time_to_reach_dst_frames_in_seconds = 0.0f;
        return;
    }

    time_step_in_seconds -= motion_info.time_to_reach_dst_frames_in_seconds;

    motion_info.time_to_reach_dst_frames_in_seconds = 0.0f;
    do
    {
        if (cursor.keyframe_index + 1U == animation.get_keyframes().size())
            if (cursor.repeat)
            {
                cursor.keyframe_index = 0U;

                infer_parent_frame_from_local_and_world_frames(
                        animation.keyframe_at(0U).get_coord_systems().front(),
                        motion_info.dst_frames_in_world_space.front(),
                        cursor.frame_in_world_space
                        );
            }
            else
            {
                time_step_in_seconds = motion_info.time_to_reach_dst_frames_in_seconds;
                break;
            }
        ++cursor.keyframe_index;
        motion_info.time_to_reach_dst_frames_in_seconds +=
                animation.keyframe_at(cursor.keyframe_index).get_time_point() -
                animation.keyframe_at(cursor.keyframe_index - 1U).get_time_point();
    }
    while (motion_info.time_to_reach_dst_frames_in_seconds < time_step_in_seconds);

    motion_info.current_frames_in_world_space.swap(motion_info.dst_frames_in_world_space);

    transform_skeleton_coord_systems_from_common_reference_frame_to_world(
            animation.keyframe_at(cursor.keyframe_index).get_coord_systems(),
            cursor.frame_in_world_space,
            motion_info.dst_frames_in_world_space
            );
}


}
