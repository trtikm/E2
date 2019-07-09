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

    get_blackboard()->m_cortex_cmd_move_intensity = get_io()->output.at(0);
    get_blackboard()->m_cortex_cmd_turn_intensity = -get_io()->output.at(1);
    get_blackboard()->m_cortex_cmd_elevation_intensity = get_io()->output.at(2);
}


}
