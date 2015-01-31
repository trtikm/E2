#include <cellconnect/utility/clip_tissue_coordinate.hpp>

namespace cellconnect {


natural_32_bit  clip_tissue_coordinate(
        integer_64_bit  target_coord,
        natural_32_bit const  num_cells_along_the_axis,
        bool const  is_torus_axis
        )
{
    if (is_torus_axis)
    {
        while (target_coord < 0LL)
            target_coord += num_cells_along_the_axis;
        return target_coord % num_cells_along_the_axis;
    }

    if (target_coord < 0LL)
        return 0U;
    if (target_coord > num_cells_along_the_axis)
        return num_cells_along_the_axis - 1U;
    return target_coord;
}


}
