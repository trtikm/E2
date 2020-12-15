#include <ai/cortex_default.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {



cortex_default::cortex_default(agent const*  myself_)
    : cortex(myself_)
{}


void  cortex_default::next_round(float_32_bit const  time_step_in_seconds)
{
    motion_desire_props_ref() = motion_desire_props();
}


}
