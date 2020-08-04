#ifndef AIOLD_SENSOR_KIND_HPP_INCLUDED
#   define AIOLD_SENSOR_KIND_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace aiold {


enum struct  SENSOR_KIND : natural_8_bit
{
    TOUCH_BEGIN,
    TOUCHING,
    TOUCH_END,
    TIMER
};


std::string const&  description(SENSOR_KIND const  kind);


inline natural_8_bit  as_number(SENSOR_KIND const  kind) noexcept
{
    return (natural_8_bit)kind;
}


inline SENSOR_KIND  as_sensor_kind(natural_8_bit const  index)
{
    return (SENSOR_KIND)index;
}


inline constexpr natural_8_bit  num_sensor_kinds() { return (natural_8_bit)SENSOR_KIND::TIMER + 1; }


}


std::string const&  as_string(aiold::SENSOR_KIND const  kind);
aiold::SENSOR_KIND  as_sensor_kind(std::string const&  name);


#endif