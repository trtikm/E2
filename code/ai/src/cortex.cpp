#include <ai/cortex.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex(blackboard_agent_weak_ptr const  blackboard_)
    : m_blackboard(blackboard_)
    , m_motion_desire_props()
{}


void  cortex::initialise()
{
    set_stationary_desire(m_motion_desire_props, get_blackboard());
}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    set_stationary_desire(m_motion_desire_props, get_blackboard());
}


}

namespace ai {


void  set_stationary_desire(motion_desire_props&  desire_props, blackboard_agent_ptr const  bb)
{
    desire_props = motion_desire_props();
}


}
