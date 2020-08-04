#include <aiold/cortex.hpp>
#include <aiold/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace aiold {


cortex::cortex(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
{}


void  cortex::initialise()
{
    set_stationary_desire(m_motion_desire_props);
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    set_stationary_desire(m_motion_desire_props);
}


}

namespace aiold {


void  set_stationary_desire(motion_desire_props&  desire_props)
{
    desire_props = motion_desire_props();
}


}
