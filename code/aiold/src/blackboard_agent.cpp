#include <aiold/blackboard_agent.hpp>
#include <cassert>

namespace aiold {


blackboard_agent::~blackboard_agent()
{
    assert(
        m_cortex == nullptr &&
        m_sensory_controller == nullptr &&
        m_action_controller == nullptr
        ); 
}


}
