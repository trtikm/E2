#include <ai/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    for (float_32_bit&  value_ref : get_io()->output)
        value_ref = 0.0f;
}


}
