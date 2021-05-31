#ifndef ANGEO_COLLISION_MATERIAL_HPP_INCLUDED
#   define ANGEO_COLLISION_MATERIAL_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <string>

namespace angeo {


enum struct  COLLISION_MATERIAL_TYPE : natural_8_bit
{
    ASPHALT             = 0,
    CONCRETE            = 1,
    DIRT                = 2,
    GLASS               = 3,
    GRASS               = 4,
    GUM                 = 5,
    ICE                 = 6,
    LEATHER             = 7,
    MUD                 = 8,
    PLASTIC             = 9,
    RUBBER              = 10,
    STEEL               = 11,
    WOOD                = 12,

    WATER               = 13,   // Use it only in a force field.
    AIR                 = 14,   // Use it only in a force field.

    NO_FRINCTION_NO_BOUNCING,
};

constexpr natural_8_bit  get_num_collision_materials() { return 1U + (natural_8_bit)COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING;}
inline natural_8_bit  as_number(COLLISION_MATERIAL_TYPE const  material) { return (natural_8_bit)material; }
COLLISION_MATERIAL_TYPE  as_material(natural_8_bit  const  material);
char const*  to_string(COLLISION_MATERIAL_TYPE const  material);
COLLISION_MATERIAL_TYPE  read_collison_material_from_string(std::string const&  name);


}

#endif
