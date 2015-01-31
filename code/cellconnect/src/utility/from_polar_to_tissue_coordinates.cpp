#include <cellconnect/utility/from_polar_to_tissue_coordinates.hpp>
#include <cmath>

namespace cellconnect {


std::pair<integer_64_bit,integer_64_bit>  from_polar_to_tissue_coordinates(
        float_32_bit const  angle,
        float_32_bit const  radius
        )
{
    return std::pair<integer_64_bit,integer_64_bit>(
                std::round(radius * std::cos(angle)),
                std::round(radius * std::sin(angle))
                );
}


}
