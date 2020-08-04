#include <aiold/sensor_kind.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace aiold {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(SENSOR_KIND::TOUCH_BEGIN), { "TOUCH_BEGIN",
            "Sends an event to the owner when any sensor's collider hits another\n"
            "collider in the scene.",
            } },
    { as_number(SENSOR_KIND::TOUCHING), { "TOUCHING",
            "Sends an event to the owner whenever any sensor's collider has a collision\n"
            "contact with another collider in the scene."
            } },
    { as_number(SENSOR_KIND::TOUCH_END), { "TOUCH_END",
            "Sends an event to the owner when all sensor's colliders loose a collision\n"
            "contact with another collider in the scene.",
            } },
    { as_number(SENSOR_KIND::TIMER), { "TIMER",
            "Sends an event to the owner in the end of each passed fixed time period."
            } }
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


std::string const&  as_string(aiold::SENSOR_KIND const  kind)
{
    return aiold::from_index_to_name_and_description.at(as_number(kind)).first;
}


aiold::SENSOR_KIND  as_sensor_kind(std::string const&  name)
{
    return aiold::from_name_to_kind.at(name);
}