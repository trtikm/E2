#ifndef AI_AGENT_ID_HPP_INCLUDED
#   define AI_AGENT_ID_HPP_INCLUDED

#   include <ai/object_id.hpp>
#   include <limits>

namespace ai {


using  agent_id = object_id::index_type;


inline constexpr agent_id  invalid_agent_id() { return std::numeric_limits<agent_id>::max(); }


inline object_id  agent_to_object_id(agent_id id) { return object_id(OBJECT_KIND::AGENT, id); }


}

#endif
