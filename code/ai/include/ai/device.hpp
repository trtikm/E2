#ifndef AI_DEVICE_HPP_INCLUDED
#   define AI_DEVICE_HPP_INCLUDED

#   include <ai/blackboard_device.hpp>
#   include <ai/sensor.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <vector>

namespace ai {


struct  device
{
    enum struct  ACTION
    {
        ERASE_SELF = 0,
    };
    static std::string const&  get_name(ACTION const  action);
    static ACTION  action_from_name(std::string const&  name);

    static std::unordered_map<ACTION, property_map> const&  default_action_configs();

    using  event_action_config = std::unordered_map<
            SENSOR_KIND,
            std::unordered_map<
                    scene::node_id,     // Of the sensor sending an event to this device.
                    std::vector<std::pair<ACTION, property_map> >
                    >
            >;

    static blackboard_device_ptr  create_blackboard(DEVICE_KIND const  device_kind);
    static void  create_modules(blackboard_device_ptr const  bb);

    explicit device(blackboard_device_ptr const  blackboard_);
    ~device();

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_sensor_event(sensor const&  s);

    blackboard_device_ptr  get_blackboard() { return m_blackboard; }
    blackboard_device_const_ptr  get_blackboard() const { return m_blackboard; }

    DEVICE_KIND  get_kind() const { return get_blackboard()->m_device_kind; }

private:
    blackboard_device_ptr  m_blackboard;
};


}

#endif
