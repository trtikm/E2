#include <ai/cortex_input_encoder_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_input_encoder_human::run()
{
    TMPROF_BLOCK();

    for (float_32_bit&  value_ref : get_io()->input)
        value_ref = 0.0f;    
}


}
