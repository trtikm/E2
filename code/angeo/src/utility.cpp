#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace angeo {


void  get_random_vector_of_magnitude(
        float_32_bit const  magnitude,
        random_generator_for_natural_32_bit&   random_generator,
        vector3&  resulting_vector
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(magnitude >= 0.0f);

    float_32_bit const  phi = get_random_float_32_bit_in_range(0.0f,2.0f * PI(),random_generator);
    float_32_bit const  sin_phi = std::sinf(phi);
    float_32_bit const  cos_phi = std::cosf(phi);

    float_32_bit const  psi = get_random_float_32_bit_in_range(-PI(),PI(),random_generator);
    float_32_bit const  sin_psi = std::sinf(psi);
    float_32_bit const  cos_psi = std::cosf(psi);

    resulting_vector = magnitude * vector3(cos_psi * cos_phi, cos_psi * sin_phi, sin_psi);
}


}
