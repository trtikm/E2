#ifndef CELLAB_HOMOGENOUS_SLICE_OF_TISSUE_HPP_INCLUDED
#   define CELLAB_HOMOGENOUS_SLICE_OF_TISSUE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/array_of_bit_units.hpp>
#   include <utility/bits_reference.hpp>


namespace cellab {


/**
 * It represents a 3D array stored inside a 1D array. An elements of the 3D array is defined
 * only by a number of bits to be reserved for it in the array. The underlying 1D array is
 * an instance of the utility 'array_of_bit_units' (see the header 'utility/array_of_bit_units.hpp'
 * of the E2 library 'utility').
 *
 * This structure is used inside 'cellab::dynamic_state_of_neural_tissue' to store slices of
 * the tissue. See the file 'cellab/dynamic_state_of_neural_tissue.hpp' for more details.
 */
struct homogenous_slice_of_tissue
{
    homogenous_slice_of_tissue(natural_16_bit const num_bits_per_unit,
                               natural_32_bit const num_units_along_x_axis,
                               natural_32_bit const num_units_along_y_axis,
                               natural_64_bit const num_units_along_columnar_axis);

    bits_reference find_bits_of_unit(natural_32_bit const shift_along_x_axis,
                                     natural_32_bit const shift_along_y_axis,
                                     natural_64_bit const shift_along_columnar_axis);
    natural_16_bit num_bits_per_unit() const;
    natural_32_bit num_units_along_x_axis() const;
    natural_32_bit num_units_along_y_axis() const;
    natural_64_bit num_units_along_columnar_axis() const;
private:
    natural_64_bit m_num_units_along_x_axis;
    natural_64_bit m_num_units_along_y_axis;
    natural_64_bit m_num_units_along_columnar_axis;
    array_of_bit_units m_array_of_units;    //!< This is the 1D array where individual units are actually stored.
};


/**
 * It computes a total number of bits an intance of 'homogenous_slice_of_tissue' would allocate for
 * its elements for the same arguments passed to the constructor.
 */
natural_64_bit compute_num_bits_of_slice_of_tissue_with_checked_operations(
        natural_16_bit const num_bits_per_unit,
        natural_32_bit const num_units_along_x_axis,
        natural_32_bit const num_units_along_y_axis,
        natural_64_bit const num_units_along_columnar_axis
        );


}

#endif
