#ifndef AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED
#   define AI_DETAIL_ACTION_CONTROLLER_INTERPOLATOR_HPP_INCLUDED

#   include <ai/blackboard_agent.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace ai { namespace detail {


struct  action_controller_interpolator
{
    explicit action_controller_interpolator(blackboard_agent_weak_ptr const  blackboard_);
    virtual ~action_controller_interpolator() {}

    blackboard_agent_ptr  get_blackboard() const { return m_blackboard.lock(); }

    bool  done() const { return m_consumed_time_in_seconds >= m_total_interpolation_time_in_seconds; }

    void  reset_time(float_32_bit const  interpolation_time_in_seconds);

    float_32_bit  get_interpolation_parameter() const; // The iterpolation parameter is always in <0,1>.

    float_32_bit  next_round(float_32_bit const  time_step_in_seconds);

private:

    blackboard_agent_weak_ptr  m_blackboard;
    float_32_bit  m_total_interpolation_time_in_seconds;
    float_32_bit  m_consumed_time_in_seconds;
};


}}

#endif
