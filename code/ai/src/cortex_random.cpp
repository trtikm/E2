#include <ai/cortex_random.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex_random::cortex_random(blackboard_weak_ptr const  blackboard_)
    : cortex(blackboard_)
{}


void  cortex_random::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // TODO!
    set_stationary_desire();
}


}
