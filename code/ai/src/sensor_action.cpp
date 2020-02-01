#include <ai/sensor_action.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace ai {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(SENSOR_ACTION_KIND::END_OF_LIFE), { "END_OF_LIFE", "Erase self from the scene." } },
};

static std::unordered_map<std::string, SENSOR_ACTION_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, SENSOR_ACTION_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_sensor_action_kind(elem.first) });
    return result;
}();


std::string const&  description(SENSOR_ACTION_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


std::string const&  as_string(SENSOR_ACTION_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).first;
}


SENSOR_ACTION_KIND  as_sensor_action_kind(std::string const&  name)
{
    return from_name_to_kind.at(name);
}


std::unordered_map<SENSOR_ACTION_KIND, property_map> const&  default_sensor_action_props()
{
    static std::unordered_map<SENSOR_ACTION_KIND, property_map>  props {
        { SENSOR_ACTION_KIND::END_OF_LIFE, property_map{} },
    };
    return props;
}


}
