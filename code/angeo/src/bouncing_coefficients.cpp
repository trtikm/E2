#include <angeo/bouncing_coefficients.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {



static float_32_bit  B(
        float_32_bit const  desired_bouncing_coef   // A value in the interval <0.0f, 1.0f>. Imagine you drop a ball
                                                    // from 1m above the ground. Then 'desired_bouncing_coef' is the
                                                    // desired maximal height above the ground after the ball hits the
                                                    // ground.
                                                    // This function then computes a 'bouncing_coefficient' which will
                                                    // provide the desired 'bouncing-height' behaviour for the physics
                                                    // simulator.
        )
{
    return (float_32_bit)std::log(std::pow(desired_bouncing_coef, 0.8f) *(E() - 1.0f) + 1.0f);
}


float_32_bit  get_bouncing_coefficient(
        COLLISION_MATERIAL_TYPE const  material_0,
        COLLISION_MATERIAL_TYPE const  material_1
        )
{
    static float_32_bit const  bouncing_coefficient[get_num_collision_materials()][get_num_collision_materials()] {
    // NOTE: Elements above the diagonal are NOT used, so initialise them to '.0f'.
    // TODO: The values starting with '+' are not correct. Find the correct values on the Internet and update the table.
        { B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // ASPHALT
        { B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // CONCRETE
        { B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // DIRT
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // GLASS
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // GRASS
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // GUM
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // ICE
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // LETHER
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f,       .0f, },  // MUD
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f,       .0f, },  // PLASTIC
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f,       .0f, },  // RUBBER
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f,       .0f, },  // STEEL
        { B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),  B(+0.1f),       .0f, },  // WOOD
        {      .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f,       .0f, },  // NO_FRINCTION_NO_BOUNCING
    //  ASPHALT    CONCRETE   DIRT       GLASS      GRASS      GUM        ICE        LEATHER    MUD        PLASTIC    RUBBER     STEEL      WOOD       NO_FRINCTION_NO_BOUNCING
    };
    return material_0 > material_1 ? bouncing_coefficient[(natural_8_bit)material_0][(natural_8_bit)material_1] :
                                     bouncing_coefficient[(natural_8_bit)material_1][(natural_8_bit)material_0] ;
}


}
