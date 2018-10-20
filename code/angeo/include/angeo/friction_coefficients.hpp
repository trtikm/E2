#ifndef ANGEO_FRICTION_COEFFICIENTS_HPP_INCLUDED
#   define ANGEO_FRICTION_COEFFICIENTS_HPP_INCLUDED

#   include <angeo/collision_material.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace angeo {


float_32_bit  get_static_friction_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        );


float_32_bit  get_dynamic_friction_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        );


}

#endif
