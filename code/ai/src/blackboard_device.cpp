#include <ai/blackboard_device.hpp>
#include <cassert>

namespace ai {


blackboard_device::~blackboard_device()
{
    assert(are_all_modules_released()); 
}


bool  blackboard_device::are_all_modules_released() const
{
    return true;
}


}
