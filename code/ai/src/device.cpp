#include <ai/device.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


blackboard_device_ptr  device::create_blackboard(DEVICE_KIND const  device_kind)
{
    TMPROF_BLOCK();

    switch (device_kind)
    {
    case DEVICE_KIND::DEFAULT:
        return std::make_shared<blackboard_device>();
    default:
        UNREACHABLE();
    }
}


void  device::create_modules(blackboard_device_ptr const  bb)
{
    TMPROF_BLOCK();

    switch (bb->m_device_kind)
    {
    case DEVICE_KIND::DEFAULT:
        break;
    default:
        UNREACHABLE();
    }
}


}

namespace ai {


device::device(blackboard_device_ptr const  blackboard_)
    : m_blackboard(blackboard_)
{
    TMPROF_BLOCK();

    // The initialisation order of device's modules is important and mandatory.
}


device::~device()
{
    TMPROF_BLOCK();

    // The release order of device's modules is important and mandatory.
}


void  device::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // The update order of device's modules is important and mandatory.
}


}
