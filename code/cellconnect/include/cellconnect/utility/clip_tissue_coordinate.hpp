#ifndef CELLCONNECT_UTILITY_CLIP_TISSUE_COORDINATE_HPP_INCLUDED
#   define CELLCONNECT_UTILITY_CLIP_TISSUE_COORDINATE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace cellconnect {


natural_32_bit  clip_tissue_coordinate(
        integer_64_bit  target_coord,
        natural_32_bit const  num_cells_along_the_axis,
        bool const  is_torus_axis
        );


}

#endif
