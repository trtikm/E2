#include <ai/blackboard_agent.hpp>
#include <cassert>

namespace ai {


blackboard_agent::~blackboard_agent()
{
    assert(
        m_cortex == nullptr &&
        m_sensory_controller == nullptr &&
        m_action_controller == nullptr
        ); 
}


}
