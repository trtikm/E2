#include <ai/blackboard_agent.hpp>
#include <cassert>

namespace ai {


blackboard_agent::~blackboard_agent()
{
    assert(are_all_modules_released()); 
}


bool  blackboard_agent::are_all_modules_released() const
{
    return m_cortex == nullptr && m_sensory_controller == nullptr && m_action_controller == nullptr;
}


}
