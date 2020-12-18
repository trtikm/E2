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
        float_32_bit  ideal_value;
    };

    agent_state_variable(
            std::string const&  name_,
            float_32_bit const  value_,
            float_32_bit const  min_value_,
            float_32_bit const  max_value_,
            float_32_bit const  ideal_value_
            );

    std::string const&  get_name() const { return name; }
    float_32_bit  get_value() const { return value; }
    float_32_bit  get_min_value() const { return constants.min_value; }
    float_32_bit  get_max_value() const { return constants.max_value; }
    float_32_bit  get_ideal_value() const { return constants.ideal_value; }

    void  set_value(float_32_bit const  x)
    {
        value = x < constants.min_value ? constants.min_value : (x > constants.max_value ? constants.max_value : x);
    }

    void  add_to_value(float_32_bit const  dx) { set_value(value + dx); }

private:
    std::string  name;
    float_32_bit  value;
    config  constants;
};


using  agent_state_variables = std::unordered_map<std::string, agent_state_variable>;


agent_state_variables  load_agent_state_variables(agent_config const  config);


}

#endif
