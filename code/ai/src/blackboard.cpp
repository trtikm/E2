#include <ai/blackboard.hpp>
#include <cassert>

namespace ai {


blackboard::~blackboard()
{
    assert(are_all_modules_released()); 
}


bool  blackboard::are_all_modules_released() const
{
    return m_cortex == nullptr && m_sensory_controller == nullptr && m_action_controller == nullptr;
}


}
