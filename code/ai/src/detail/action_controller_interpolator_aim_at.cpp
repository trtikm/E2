#include <ai/detail/action_controller_interpolator_aim_at.hpp>
//#include <ai/sensory_controller.hpp>
#include <ai/skeleton_utils.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator_aim_at::action_controller_interpolator_aim_at(
        action_controller_interpolator const* const  interpolator_,
        skeletal_motion_templates::motion_template_cursor const&  initial_template_cursor
        )
    : action_controller_interpolator_shared(interpolator_)

    , m_src_bones()
    , m_current_bones()
    , m_dst_bones()
{
    set_target(initial_template_cursor);
    m_src_bones = m_current_bones = m_dst_bones;
}


void  action_controller_interpolator_aim_at::interpolate(
        float_32_bit const  time_step_in_seconds,
        float_32_bit const  interpolation_param,
        vector3 const&  aim_at_target_in_agent_space,
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

    // TODO!
}


void  action_controller_interpolator_aim_at::set_target(
        skeletal_motion_templates::motion_template_cursor const&  cursor
        )
{
    m_src_bones = m_current_bones;
    m_dst_bones = get_blackboard()->m_motion_templates.motions_map().at(cursor.motion_name).aim_at.at(cursor.keyframe_index);
}


}}
