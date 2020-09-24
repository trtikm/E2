#include <ai/cortex_default.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {



cortex_default::cortex_default()
{}


void  cortex_default::next_round(float_32_bit const  time_step_in_seconds)
{
    cortex::next_round(time_step_in_seconds);
}


}
