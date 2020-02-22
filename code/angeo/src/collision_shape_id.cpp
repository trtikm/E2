#include <angeo/collision_shape_id.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>
#include <functional>

namespace angeo {


static std::unordered_map<natural_8_bit, std::pair<std::string, std::string> > const  from_index_to_name_and_description = {
    { as_number(COLLISION_SHAPE_TYPE::BOX), { "BOX", "Any box, i.e. more general than AABB." } },
    { as_number(COLLISION_SHAPE_TYPE::CAPSULE), { "CAPSULE", "All points up to given distance from the central line." } },
    { as_number(COLLISION_SHAPE_TYPE::LINE), { "LINE", "" } },
    { as_number(COLLISION_SHAPE_TYPE::POINT), { "POINT", "" } },
    { as_number(COLLISION_SHAPE_TYPE::SPHERE), { "SPHERE", "" } },
    { as_number(COLLISION_SHAPE_TYPE::TRIANGLE), { "TRIANGLE", "" } },
};


static std::unordered_map<std::string, COLLISION_SHAPE_TYPE> const  from_name_to_cst = []() {
    std::unordered_map<std::string, COLLISION_SHAPE_TYPE>  result;
    for (auto const&  elem : from_index_to_name_and_description)
        result.insert({ elem.second.first, as_collision_shape_type(elem.first) });
    return result;
}();


std::string const& description(COLLISION_SHAPE_TYPE const  cst)
{
    return from_index_to_name_and_description.at(as_number(cst)).second;
}


std::string const&  as_string(COLLISION_SHAPE_TYPE const  cst)
{
    return from_index_to_name_and_description.at(as_number(cst)).first;
}


COLLISION_SHAPE_TYPE  as_collision_shape_type(std::string const&  name)
{
    return from_name_to_cst.at(name);
}


}
