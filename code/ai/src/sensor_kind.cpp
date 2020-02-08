#include <ai/sensor_kind.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace ai {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(SENSOR_KIND::TIMER), { "TIMER", "A default sensor." } },
};

static std::unordered_map<std::string, SENSOR_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, SENSOR_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_sensor_kind(elem.first) });
    return result;
}();


std::string const& description(SENSOR_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


}


std::string const&  as_string(ai::SENSOR_KIND const  kind)
{
    return ai::from_index_to_name_and_description.at(as_number(kind)).first;
}


ai::SENSOR_KIND  as_sensor_kind(std::string const&  name)
{
    return ai::from_name_to_kind.at(name);
}
