#include <aiold/device_kind.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace aiold {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(DEVICE_KIND::DEFAULT), { "DEFAULT",
            "A default device."
            } },
};

static std::unordered_map<std::string, DEVICE_KIND> const  from_name_to_kind = []() {
    std::unordered_map<std::string, DEVICE_KIND>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_device_kind(elem.first) });
    return result;
}();


std::string const& description(DEVICE_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).second;
}


std::string const&  as_string(DEVICE_KIND const  kind)
{
    return from_index_to_name_and_description.at(as_number(kind)).first;
}


DEVICE_KIND  as_device_kind(std::string const&  name)
{
    return from_name_to_kind.at(name);
}


}
