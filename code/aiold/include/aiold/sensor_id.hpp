#ifndef AIOLD_SENSOR_ID_HPP_INCLUDED
#   define AIOLD_SENSOR_ID_HPP_INCLUDED

#   include <aiold/object_id.hpp>
#   include <limits>

namespace aiold {


using  sensor_id = object_id::index_type;


inline constexpr sensor_id  invalid_sensor_id() { return std::numeric_limits<sensor_id>::max(); }


inline object_id  sensor_to_object_id(sensor_id id) { return object_id(OBJECT_KIND::SENSOR, id); }


}

#endif
