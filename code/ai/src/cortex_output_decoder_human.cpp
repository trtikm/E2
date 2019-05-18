#include <ai/cortex_output_decoder_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_output_decoder_human::run()
{    
    TMPROF_BLOCK();

    get_blackboard()->m_cortex_cmd_move_intensity = get_io()->output.front();
    get_blackboard()->m_cortex_cmd_turn_intensity = -get_io()->output.back();
}


}
