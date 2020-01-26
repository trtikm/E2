#ifndef AI_DEVICE_KIND_HPP_INCLUDED
#   define AI_DEVICE_KIND_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


enum struct  DEVICE_KIND : natural_8_bit
{
    DEFAULT = 0,
};


std::string const&  description(DEVICE_KIND const  kind);


inline natural_8_bit  as_number(DEVICE_KIND const  kind) noexcept
{
    return *reinterpret_cast<natural_8_bit const*>(&kind);
}


inline DEVICE_KIND  as_device_kind(natural_8_bit const  index)
{
    return (DEVICE_KIND)index;
}


inline constexpr natural_8_bit  num_device_kinds() { return (natural_8_bit)DEVICE_KIND::DEFAULT + 1; }


std::string const&  as_string(DEVICE_KIND const  kind);
DEVICE_KIND  as_device_kind(std::string const&  name);


}

#endif
