#include <ai/sensory_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  sensory_controller::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (get_sight() != nullptr)
        get_sight()->next_round(time_step_in_seconds);
}


}
