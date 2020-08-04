#ifndef AIOLD_DEVICE_ID_HPP_INCLUDED
#   define AIOLD_DEVICE_ID_HPP_INCLUDED

#   include <aiold/object_id.hpp>
#   include <limits>

namespace aiold {


using  device_id = object_id::index_type;


inline constexpr device_id  invalid_device_id() { return std::numeric_limits<device_id>::max(); }


inline object_id  device_to_object_id(device_id id) { return object_id(OBJECT_KIND::DEVICE, id); }


}

#endif
