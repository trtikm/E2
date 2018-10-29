#include <angeo/collision_material.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

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
    default:
        UNREACHABLE();
    }
}


}
