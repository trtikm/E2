#ifndef AI_AGENT_SYSTEM_VARIABLES_HPP_INCLUDED
#   define AI_AGENT_SYSTEM_VARIABLES_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <unordered_map>

namespace ai { struct  agent_system_state; }

namespace ai {


using  agent_system_variables = std::unordered_map<std::string, float_32_bit>;


agent_system_variables  load_agent_system_variables();
void  update_system_variables(agent_system_variables&  variables, agent_system_state const&  state);


}

#endif
