#ifndef ANGEO_COLLISION_MATERIAL_HPP_INCLUDED
#   define ANGEO_COLLISION_MATERIAL_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace angeo {


enum struct  COLLISION_MATERIAL_TYPE : natural_8_bit
{
    CONCRETE            = 0,
    GLASS               = 1,
    GUM                 = 2,
    PLASTIC             = 3,
    RUBBER              = 4,
    STEEL               = 5,
    WOOD                = 6,
};


}

#endif
