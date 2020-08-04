#include <aiold/agent_kind.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace aiold {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(AGENT_KIND::MOCK), { "MOCK", "An agent controlled by a human operator via keyboard (see file transitions.mock.json)." } },
    { as_number(AGENT_KIND::STATIONARY), { "STATIONARY", "An agent controlled by a cortex choosing most stationary actions." } },
    { as_number(AGENT_KIND::RANDOM), { "RANDOM", "An agent controlled by a cortex choosing actions randomly." } },
    { as_number(AGENT_KIND::ROBOT), { "ROBOT", "An agent controlled by an experimental (prototyped) cortex." } },
};

static std::unordered_map<std::string, AGENT_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, AGENT_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_agent_kind(elem.first) });
    return result;
}();


std::string const& description(AGENT_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


std::string const&  as_string(AGENT_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).first;
}


AGENT_KIND  as_agent_kind(std::string const&  name)
{
    return from_name_to_kind.at(name);
}


}
