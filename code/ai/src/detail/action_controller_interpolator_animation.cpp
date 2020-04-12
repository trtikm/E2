#include <ai/detail/action_controller_interpolator_animation.hpp>
#include <ai/skeleton_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_animation::action_controller_interpolator_animation(blackboard_agent_weak_ptr const  blackboard_)
    : action_controller_interpolator(blackboard_)
    , m_src_frames()
    , m_current_frames()
    , m_dst_frames()
{
    ASSUMPTION(get_blackboard() != nullptr && !get_blackboard()->m_motion_templates.empty());
    set_target({ get_blackboard()->m_motion_templates.transitions().initial_motion_name(), 0U }, 0.0f);
    m_src_frames = m_current_frames = m_dst_frames;
}


void  action_controller_interpolator_animation::next_round(float_32_bit const  time_step_in_seconds)
{
    if (done()) return;
    float_32_bit const  interpolation_param = action_controller_interpolator::next_round(time_step_in_seconds);

    interpolate_keyframes_spherical(m_src_frames, m_dst_frames, interpolation_param, m_current_frames);
}


void  action_controller_interpolator_animation::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor,
        float_32_bit const  interpolation_time_in_seconds)
{
    action_controller_interpolator::reset_time(interpolation_time_in_seconds);

    m_src_frames = m_current_frames;
    transform_keyframes_to_reference_frame(
            get_blackboard()->m_motion_templates.at(cursor.motion_name).keyframes.get_keyframes().at(cursor.keyframe_index)
                                                                                                 .get_coord_systems(),
            get_blackboard()->m_motion_templates.at(cursor.motion_name).reference_frames.at(cursor.keyframe_index),
            get_blackboard()->m_motion_templates.pose_frames().get_coord_systems(),
            get_blackboard()->m_motion_templates.hierarchy().parents(),
            m_dst_frames
            );
}


void  action_controller_interpolator_animation::commit(vector3 const&  origin_offset) const
{
    TMPROF_BLOCK();

    auto const&  parents = get_blackboard()->m_motion_templates.hierarchy().parents();
    for (natural_32_bit bone = 0; bone != m_current_frames.size(); ++bone)
        get_blackboard()->m_scene->set_frame_of_scene_node(
                get_blackboard()->m_bone_nids.at(bone),
                true,
                parents.at(bone) < 0 ?
                    angeo::coordinate_system{
                        m_current_frames.at(bone).origin() + origin_offset,
                        m_current_frames.at(bone).orientation()
                        }
                    :
                    m_current_frames.at(bone)
                );
}


}}
