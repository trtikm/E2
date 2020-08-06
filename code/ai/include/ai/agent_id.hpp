#ifndef AI_AGENT_ID_HPP_INCLUDED
#   define AI_AGENT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <limits>

namespace ai {


using  agent_id = natural_16_bit;

inline constexpr agent_id  invalid_agent_id() { return std::numeric_limits<agent_id>::max(); }


}

#endif
