#include <angeo/collision_material.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <unordered_map>

namespace angeo {


COLLISION_MATERIAL_TYPE  as_material(natural_8_bit  const  material_index)
{
    ASSUMPTION(material_index < get_num_collision_materials());
    return (COLLISION_MATERIAL_TYPE)material_index;
}


char const*  to_string(COLLISION_MATERIAL_TYPE const  material)
{
    switch (material)
    {
    case COLLISION_MATERIAL_TYPE::ASPHALT: return "ASPHALT";
    case COLLISION_MATERIAL_TYPE::CONCRETE: return "CONCRETE";
    case COLLISION_MATERIAL_TYPE::DIRT: return "DIRT";
    case COLLISION_MATERIAL_TYPE::GLASS: return "GLASS";
    case COLLISION_MATERIAL_TYPE::GRASS: return "GRASS";
    case COLLISION_MATERIAL_TYPE::GUM: return "GUM";
    case COLLISION_MATERIAL_TYPE::ICE: return "ICE";
    case COLLISION_MATERIAL_TYPE::LEATHER: return "LEATHER";
    case COLLISION_MATERIAL_TYPE::MUD: return "MUD";
    case COLLISION_MATERIAL_TYPE::PLASTIC: return "PLASTIC";
    case COLLISION_MATERIAL_TYPE::RUBBER: return "RUBBER";
    case COLLISION_MATERIAL_TYPE::STEEL: return "STEEL";
    case COLLISION_MATERIAL_TYPE::WOOD: return "WOOD";
    case COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING: return "NO_FRINCTION_NO_BOUNCING";
    default:
        UNREACHABLE();
    }
}


COLLISION_MATERIAL_TYPE  read_collison_material_from_string(std::string const&  name)
{
    static std::unordered_map<std::string, COLLISION_MATERIAL_TYPE> const  map {
        {"ASPHALT", COLLISION_MATERIAL_TYPE::ASPHALT},
        {"CONCRETE", COLLISION_MATERIAL_TYPE::CONCRETE},
        {"DIRT", COLLISION_MATERIAL_TYPE::DIRT},
        {"GLASS", COLLISION_MATERIAL_TYPE::GLASS},
        {"GRASS", COLLISION_MATERIAL_TYPE::GRASS},
        {"GUM", COLLISION_MATERIAL_TYPE::GUM},
        {"ICE", COLLISION_MATERIAL_TYPE::ICE},
        {"LEATHER", COLLISION_MATERIAL_TYPE::LEATHER},
        {"MUD", COLLISION_MATERIAL_TYPE::MUD},
        {"PLASTIC", COLLISION_MATERIAL_TYPE::PLASTIC},
        {"RUBBER", COLLISION_MATERIAL_TYPE::RUBBER},
        {"STEEL", COLLISION_MATERIAL_TYPE::STEEL},
        {"WOOD", COLLISION_MATERIAL_TYPE::WOOD},
        {"NO_FRINCTION_NO_BOUNCING", COLLISION_MATERIAL_TYPE::WOOD},
    };
    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;
}


}
