#include <ai/detail/action_controller_interpolator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace ai { namespace detail {


action_controller_interpolator::action_controller_interpolator(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_total_interpolation_time_in_seconds(0.0f)
    , m_consumed_time_in_seconds(0.0f)
{
    ASSUMPTION(get_blackboard() != nullptr && !get_blackboard()->m_motion_templates.empty());
}


void  action_controller_interpolator::add_time(float_32_bit const  time_step_in_seconds)
{
    m_consumed_time_in_seconds = std::min(m_total_interpolation_time_in_seconds, m_consumed_time_in_seconds + time_step_in_seconds);
}


void  action_controller_interpolator::reset_time(float_32_bit const  total_interpolation_time_in_seconds)
{
    ASSUMPTION(total_interpolation_time_in_seconds >= 0.0f);
    m_total_interpolation_time_in_seconds = total_interpolation_time_in_seconds;
    m_consumed_time_in_seconds = 0.0f;
}


float_32_bit  action_controller_interpolator::get_interpolation_parameter() const
{
    return m_total_interpolation_time_in_seconds < 0.0001f ?
                1.0f :
                std::min(m_consumed_time_in_seconds / m_total_interpolation_time_in_seconds, 1.0f);
}


}}
