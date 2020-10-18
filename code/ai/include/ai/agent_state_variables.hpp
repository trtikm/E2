#ifndef AI_AGENT_STATE_VARIABLES_HPP_INCLUDED
#   define AI_AGENT_STATE_VARIABLES_HPP_INCLUDED

#   include <ai/agent_config.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <unordered_map>
#   include <memory>

namespace ai {


struct  agent_state_variable
{
    struct  config
    {
        float_32_bit  min_value;
        float_32_bit  max_value;
    };

    agent_state_variable(
            std::string const&  name_,
            float_32_bit const  value_,
            float_32_bit const  min_value_,
            float_32_bit const  max_value_
            );

    std::string  name;
    float_32_bit  value;
    config  constants;
};


using  agent_state_variables = std::unordered_map<std::string, agent_state_variable>;

using  agent_state_variables_ptr = std::shared_ptr<agent_state_variables>;
using  agent_state_variables_const_ptr = std::shared_ptr<agent_state_variables const>;


agent_state_variables_ptr  load_agent_state_variables(agent_config const  config);


}

#endif
