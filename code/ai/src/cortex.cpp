#include <ai/cortex.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


cortex::cortex()
    : m_motion_desire_props()
{}


void  cortex::next_round(float_32_bit const  time_step_in_seconds)
{
    m_motion_desire_props = motion_desire_props();
}


}
