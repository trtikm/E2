#ifndef CELLAB_SHIFT_IN_COORDINATES_HPP_INCLUDED
#   define CELLAB_SHIFT_IN_COORDINATES_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace cellab {


/**
 * It is a shift in tissue coordinates defined by the type 'tissue_coordinates' (see the file
 * 'utilities_for_transition_algorithms.hpp'). It is primarily used for description of spatial
 * neighbourhood (see the type 'spatial_neighbourhood' in 'utilities_for_transition_algorithms.hpp').
 */
struct shift_in_coordinates
{
    shift_in_coordinates(
            integer_8_bit const shift_along_x_axis,
            integer_8_bit const shift_along_y_axis,
            integer_8_bit const shift_along_columnar_axis
            );
    integer_8_bit  get_shift_along_x_axis() const;
    integer_8_bit  get_shift_along_y_axis() const;
    integer_8_bit  get_shift_along_columnar_axis() const;
private:
    integer_8_bit  m_shift_along_x_axis;
    integer_8_bit  m_shift_along_y_axis;
    integer_8_bit  m_shift_along_columnar_axis;
};


}

#endif
