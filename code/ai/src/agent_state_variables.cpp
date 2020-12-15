#include <ai/agent_state_variables.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agent_state_variable::agent_state_variable(
        std::string const&  name_,
        float_32_bit const  value_,
        float_32_bit const  min_value_,
        float_32_bit const  max_value_,
        float_32_bit const  ideal_value_
        )
    : name(name_)
    , value(value_)
    , constants{ min_value_, max_value_, ideal_value_ }
{}


void  load_agent_state_variables(agent_state_variables&  variables, agent_config const  config)
{
    for (auto const&  name_and_ptree : config.state_variables())
        variables.insert({
                name_and_ptree.first,
                {
                    name_and_ptree.first,
                    name_and_ptree.second->get<float_32_bit>("initial_value"),
                    name_and_ptree.second->get<float_32_bit>("min_value"),
                    name_and_ptree.second->get<float_32_bit>("max_value"),
                    name_and_ptree.second->get<float_32_bit>("ideal_value")
                    }
                });
}


}
