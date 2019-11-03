#include <ai/blackboard.hpp>
#include <utility/invariants.hpp>

namespace ai {


blackboard::~blackboard()
{
    // Check that modules were released from the agent.
    INVARIANT(
        m_cortex == nullptr &&
        m_sensory_controller == nullptr &&
        m_action_controller == nullptr
        ); 
}


}
