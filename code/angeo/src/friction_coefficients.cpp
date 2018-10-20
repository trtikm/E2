#include <angeo/friction_coefficients.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {


float_32_bit  get_static_friction_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        )
{
    // TODO!
    return 0.4f;
}


float_32_bit  get_dynamic_friction_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        )
{
    // TODO!
    return 0.25f;
}


}
