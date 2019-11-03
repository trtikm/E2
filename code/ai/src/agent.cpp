#include <ai/agent.hpp>
#include <ai/cortex_mock_human.hpp>
#include <ai/cortex_input_encoder_human.hpp>
#include <ai/cortex_output_decoder_human.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/action_controller_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agent::agent(
        blackboard_ptr const  blackboard_)
    : m_blackboard(blackboard_)
{}


agent::~agent()
{
    get_blackboard()->release_modules();
}


void  agent::set_use_cortex_mock(bool const  state)
{
    if (state != uses_cortex_mock())
        get_blackboard()->m_cortex_primary.swap(get_blackboard()->m_cortex_secondary);
}


bool  agent::uses_cortex_mock() const
{
    return dynamic_cast<cortex_mock*>(get_blackboard()->m_cortex_primary.get()) != nullptr;
}


void  agent::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    get_blackboard()->m_sensory_controller->next_round(time_step_in_seconds);
    get_blackboard()->m_cortex_input_encoder->run();
    get_blackboard()->m_cortex_primary->next_round(time_step_in_seconds);
    get_blackboard()->m_cortex_output_decoder->run();
    get_blackboard()->m_action_controller->next_round(time_step_in_seconds);
}


}
