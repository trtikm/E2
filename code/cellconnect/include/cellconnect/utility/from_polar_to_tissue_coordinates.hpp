#ifndef CELLCONNECT_UTILITY_FROM_POLAR_TO_TISSUE_COORDINATES_HPP_INCLUDED
#   define CELLCONNECT_UTILITY_FROM_POLAR_TO_TISSUE_COORDINATES_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility>

namespace cellconnect {


std::pair<integer_64_bit,integer_64_bit>  from_polar_to_tissue_coordinates(
        float_32_bit const  angle,
        float_32_bit const  radius
        );


}

#endif
