#include <ai/blackboard.hpp>
#include <utility/invariants.hpp>

namespace ai {


blackboard::~blackboard()
{
    INVARIANT(m_action_controller == nullptr); // Check that 'release_modules()' was called from the agent.
}


void  blackboard::release_modules()
{
    m_cortex_primary = nullptr;
    m_cortex_secondary = nullptr;
    m_cortex_input_encoder = nullptr;
    m_cortex_output_decoder = nullptr;
    m_action_controller = nullptr;
    m_sensory_controller = nullptr;
}


}
