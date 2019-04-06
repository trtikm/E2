#ifndef AI_AGENT_ID_HPP_INCLUDED
#   define AI_AGENT_ID_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace ai {


using  agent_id = natural_32_bit;


inline agent_id  invalid_agent_id() { return 0xFFFFFFFFU; }


}

#endif
