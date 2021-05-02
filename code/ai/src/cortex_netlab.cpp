#include <ai/cortex_netlab.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {



cortex_netlab::cortex_netlab(agent const*  myself_, bool const  use_mock_)
    : cortex_mock_optional(myself_, use_mock_)
{}


void  cortex_netlab::next_round(float_32_bit const  time_step_in_seconds)
{
    motion_desire_props_ref() = motion_desire_props();
}


}
