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
    static float_32_bit const  static_friction_coefficient[get_num_collision_materials()][get_num_collision_materials()] {
    // NOTE: Elements above the diagonal are NOT used, so initialise them to '.0f'.
    // TODO: The values starting with '+' are not correct. Find the correct values on the Internet and update the table.
        +0.8f,    .0f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // ASPHALT
        +0.8f,  +0.8f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // CONCRETE
        +0.8f,  +0.8f,      +0.8f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // DIRT
        +0.8f,  +0.8f,      +0.8f,  0.95f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GLASS
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GRASS
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GUM
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  0.05f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // ICE
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // LETHER
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,    .0f,    .0f,    .0f,    .0f,    .0f,  // MUD
        +0.8f,  +0.8f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  0.50f,    .0f,    .0f,    .0f,    .0f,  // PLASTIC
        1.00f,  0.95f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  +0.8f,  1.10f,    .0f,    .0f,    .0f,  // RUBBER
        +0.8f,  +0.8f,      +0.8f,  0.60f,  +0.8f,  +0.8f,  0.03f,  0.60f,  +0.8f,  0.35f,  0.95f,  0.78f,    .0f,    .0f,  // STEEL
        +0.8f,  0.62f,      +0.8f,  +0.8f,  +0.8f,  +0.8f,  0.05f,  0.40f,  +0.8f,  +0.8f,  +0.8f,  0.60f,  0.55f,    .0f,  // WOOD
          .0f,    .0f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // NO_FRINCTION_NO_BOUNCING
    //  ASPHALT CONCRETE    DIRT    GLASS   GRASS   GUM     ICE     LEATHER MUD     PLASTIC RUBBER  STEEL   WOOD    NO_FRINCTION_NO_BOUNCING
    };
    return material_0 > material_1 ? static_friction_coefficient[(natural_8_bit)material_0][(natural_8_bit)material_1] :
                                     static_friction_coefficient[(natural_8_bit)material_1][(natural_8_bit)material_0] ;
}


float_32_bit  get_dynamic_friction_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        )
{
    static float_32_bit const  dynamic_friction_coefficient[get_num_collision_materials()][get_num_collision_materials()] {
    // NOTE: Elements above the diagonal are NOT used, so initialise them to '.0f'.
    // TODO: The values starting with '+' are not correct. Find the correct values on the Internet and update the table.
        +0.6f,    .0f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // ASPHALT
        +0.6f,  +0.6f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // CONCRETE
        +0.6f,  +0.6f,      +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // DIRT
        +0.6f,  +0.6f,      +0.6f,  0.40f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GLASS
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GRASS
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // GUM
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // ICE
        0.80f,  0.85f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // LETHER
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,    .0f,    .0f,  // MUD
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  0.35f,    .0f,    .0f,    .0f,    .0f,  // PLASTIC
        0.80f,  0.85f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,    .0f,    .0f,    .0f,  // RUBBER
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  0.02f,  0.25f,  +0.6f,  0.25f,  0.50f,  0.50f,    .0f,    .0f,  // STEEL
        +0.6f,  +0.6f,      +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  +0.6f,  0.40f,  0.35f,    .0f,  // WOOD
          .0f,    .0f,        .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,    .0f,  // NO_FRINCTION_NO_BOUNCING
    //  ASPHALT CONCRETE    DIRT    GLASS   GRASS   GUM     ICE     LEATHER MUD     PLASTIC RUBBER  STEEL   WOOD    NO_FRINCTION_NO_BOUNCING
    };
    return material_0 > material_1 ? dynamic_friction_coefficient[(natural_8_bit)material_0][(natural_8_bit)material_1] :
                                     dynamic_friction_coefficient[(natural_8_bit)material_1][(natural_8_bit)material_0] ;
}


}
