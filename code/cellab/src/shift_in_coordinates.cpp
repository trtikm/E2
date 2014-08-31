#include <cellab/shift_in_coordinates.hpp>

namespace cellab {


shift_in_coordinates::shift_in_coordinates(
        integer_8_bit const shift_along_x_axis,
        integer_8_bit const shift_along_y_axis,
        integer_8_bit const shift_along_columnar_axis
        )
    : m_shift_along_x_axis(shift_along_x_axis)
    , m_shift_along_y_axis(shift_along_y_axis)
    , m_shift_along_columnar_axis(shift_along_columnar_axis)
{}

integer_8_bit  shift_in_coordinates::get_shift_along_x_axis() const
{
    return m_shift_along_x_axis;
}

integer_8_bit  shift_in_coordinates::get_shift_along_y_axis() const
{
    return m_shift_along_y_axis;
}

integer_8_bit  shift_in_coordinates::get_shift_along_columnar_axis() const
{
    return m_shift_along_columnar_axis;
}


}
