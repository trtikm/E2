#ifndef CELLAB_SHIFT_IN_COORDINATES_HPP_INCLUDED
#   define CELLAB_SHIFT_IN_COORDINATES_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>

namespace cellab {


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
