#include <ai/device.hpp>
#include <ai/simulator.hpp>
#include <ai/sensor_action_default_processor.hpp>
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


void  device::on_sensor_event(sensor const&  s, sensor::other_object_info const&  other)
{
    auto const  actions_it = get_blackboard()->m_sensor_actions->find(s.get_self_rid());

    // Each sensor must have an action attached to it, otherwise it is completely useless.
    ASSUMPTION(actions_it != get_blackboard()->m_sensor_actions->end());

    for (sensor_action&  action : actions_it->second)
    {
        // Here put processing of device-specific actions

        // Here we process actions the default way (i.e. not specific to a device)
        if (process_sensor_event_using_default_procedure(
                get_blackboard()->m_self_rid,
                action,
                other,
                get_blackboard()->m_simulator_ptr,
                get_blackboard()->m_scene
                ))
            continue;

        UNREACHABLE(); // If we get here, then we forgot to implement processing of the current action. FIX THAT!
    }
}


}
