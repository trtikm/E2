#include <ai/agent.hpp>
#include <ai/cortex_mock_human.hpp>
#include <ai/cortex_input_encoder_human.hpp>
#include <ai/cortex_output_decoder_human.hpp>
#include <ai/sensory_controller_human.hpp>
#include <ai/action_controller_human.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agent::agent(
        blackboard_ptr const  blackboard_, input_devices_const_ptr const  input_devices_)
    : m_cortex_primary()
    , m_cortex_secondary()
    , m_cortex_input_encoder()
    , m_cortex_output_decoder()
    , m_sensory_controller()
    , m_action_controller()
    , m_blackboard(blackboard_)
{
    cortex_io_ptr const  io = std::make_shared<cortex_io>();
    io->input.resize(2U);
    io->num_inner_inputs = 1U;
    io->output.resize(2U);

    m_cortex_primary.reset(std::make_unique<cortex_mock_human>(io, input_devices_).release());
    m_cortex_secondary.reset(std::make_unique<cortex>(io).release()); // Not used so far.

    m_cortex_input_encoder.reset(std::make_unique<cortex_input_encoder_human>(io, m_blackboard).release());
    m_cortex_output_decoder.reset(std::make_unique<cortex_output_decoder_human>(io, m_blackboard).release());

    m_sensory_controller.reset(std::make_unique<sensory_controller_human>(m_blackboard).release());
    m_action_controller.reset(std::make_unique<action_controller_human>(m_blackboard).release());
}


void  agent::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    if (!ready())
        return;

    m_sensory_controller->next_round(time_step_in_seconds);
    m_cortex_input_encoder->run();
    m_cortex_primary->next_round(time_step_in_seconds);
    m_cortex_output_decoder->run();
    m_action_controller->next_round(time_step_in_seconds);
}


}
