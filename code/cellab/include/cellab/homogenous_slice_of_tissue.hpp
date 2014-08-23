#ifndef CELLAB_HOMOGENOUS_SLICE_OF_TISSUE_HPP_INCLUDED
#   define CELLAB_HOMOGENOUS_SLICE_OF_TISSUE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/array_of_bit_units.hpp>
#   include <utility/bits_reference.hpp>


namespace cellab {


struct homogenous_slice_of_tissue
{
    homogenous_slice_of_tissue(natural_16_bit const num_bits_per_unit,
                               natural_32_bit const num_units_along_x_axis,
                               natural_32_bit const num_units_along_y_axis,
                               natural_64_bit const num_units_along_columnar_axis);

    bits_reference find_bits_of_unit(natural_32_bit const seek_along_x_axis,
                                     natural_32_bit const seek_along_y_axis,
                                     natural_64_bit const seek_along_columnar_axis);
private:
    natural_64_bit m_num_units_along_x_axis;
    natural_64_bit m_num_units_along_y_axis;
    natural_64_bit m_num_units_along_columnar_axis;
    array_of_bit_units m_array_of_units;
};


natural_64_bit compute_num_bits_of_slice_of_tissue_with_checked_operations(
        natural_16_bit const num_bits_per_unit,
        natural_32_bit const num_units_along_x_axis,
        natural_32_bit const num_units_along_y_axis,
        natural_64_bit const num_units_along_columnar_axis
        );


}

#endif
