#ifndef AIOLD_DEVICE_HPP_INCLUDED
#   define AIOLD_DEVICE_HPP_INCLUDED

#   include <aiold/blackboard_device.hpp>
#   include <aiold/sensor.hpp>
#   include <aiold/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <vector>

namespace aiold {


struct  device
{
    static blackboard_device_ptr  create_blackboard(DEVICE_KIND const  device_kind);
    static void  create_modules(blackboard_device_ptr const  bb);

    explicit device(blackboard_device_ptr const  blackboard_);
    ~device();

    void  next_round(float_32_bit const  time_step_in_seconds);

    void  on_sensor_event(sensor const&  s, sensor::other_object_info const&  other);

    blackboard_device_ptr  get_blackboard() { return m_blackboard; }
    blackboard_device_const_ptr  get_blackboard() const { return m_blackboard; }

    DEVICE_KIND  get_kind() const { return get_blackboard()->m_device_kind; }

private:
    blackboard_device_ptr  m_blackboard;
};


}

#endif
