#ifndef AI_AGENT_KIND_HPP_INCLUDED
#   define AI_AGENT_KIND_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace ai {


enum struct  AGENT_KIND : natural_8_bit
{
    MOCK = 0,
    STATIONARY = 1,
    RANDOM = 2,
    ROBOT = 3,
};


std::string const&  description(AGENT_KIND const  kind);


inline natural_8_bit  as_number(AGENT_KIND const  kind) noexcept
{
    return *reinterpret_cast<natural_8_bit const*>(&kind);
}


inline AGENT_KIND  as_agent_kind(natural_8_bit const  index)
{
    return (AGENT_KIND)index;
}


inline constexpr natural_32_bit  num_agent_kinds() { return (natural_32_bit)AGENT_KIND::ROBOT + 1U; }


std::string const&  as_string(AGENT_KIND const  kind);
AGENT_KIND  as_agent_kind(std::string const&  name);


}

#endif
