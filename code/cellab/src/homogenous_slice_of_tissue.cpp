#include <cellab/homogenous_slice_of_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>


static natural_64_bit compute_num_units_in_slice_of_tissue_with_checked_operations(
        natural_64_bit const num_units_along_x_axis,
        natural_64_bit const num_units_along_y_axis,
        natural_64_bit const num_units_along_columnar_axis
        )
{
    return checked_mul_64_bit(num_units_along_y_axis,
                              checked_mul_64_bit(num_units_along_x_axis,
                                                 num_units_along_columnar_axis));
}


namespace cellab {


homogenous_slice_of_tissue::homogenous_slice_of_tissue(natural_16_bit const num_bits_per_unit,
                                                       natural_32_bit const num_units_along_x_axis,
                                                       natural_32_bit const num_units_along_y_axis,
                                                       natural_64_bit const num_units_along_columnar_axis)
    : m_num_units_along_x_axis(num_units_along_x_axis)
    , m_num_units_along_y_axis(num_units_along_y_axis)
    , m_num_units_along_columnar_axis(num_units_along_columnar_axis)
    , m_array_of_units(num_bits_per_unit,
                       compute_num_units_in_slice_of_tissue_with_checked_operations(
                                m_num_units_along_x_axis,
                                m_num_units_along_y_axis,
                                m_num_units_along_columnar_axis))
{
    ASSUMPTION(num_bits_per_unit > 0U);
    ASSUMPTION(m_num_units_along_x_axis > 0U);
    ASSUMPTION(m_num_units_along_y_axis > 0U);
    ASSUMPTION(m_num_units_along_columnar_axis > 0U);
}

bits_reference homogenous_slice_of_tissue::find_bits_of_unit(
    natural_32_bit const seek_along_x_axis,
    natural_32_bit const seek_along_y_axis,
    natural_64_bit const seek_along_columnar_axis
    )
{
    natural_64_bit const x = seek_along_x_axis;
    natural_64_bit const y = seek_along_y_axis;

    ASSUMPTION(x < m_num_units_along_x_axis);
    ASSUMPTION(y < m_num_units_along_y_axis);
    ASSUMPTION(seek_along_columnar_axis < m_num_units_along_columnar_axis);

    natural_64_bit const index_of_unit =
            y * (m_num_units_along_x_axis * m_num_units_along_columnar_axis) +
            x * m_num_units_along_columnar_axis +
            seek_along_columnar_axis
            ;

    return m_array_of_units.find_bits_of_unit(index_of_unit);
}


natural_64_bit compute_num_bits_of_slice_of_tissue_with_checked_operations(
        natural_16_bit const num_bits_per_unit,
        natural_32_bit const num_units_along_x_axis,
        natural_32_bit const num_units_along_y_axis,
        natural_64_bit const num_units_along_columnar_axis
        )
{
    ASSUMPTION(num_bits_per_unit > 0U);
    ASSUMPTION(num_units_along_x_axis > 0U);
    ASSUMPTION(num_units_along_y_axis > 0U);
    ASSUMPTION(num_units_along_columnar_axis > 0U);
    return compute_num_bits_of_all_array_units_with_checked_operations(
                num_bits_per_unit,
                compute_num_units_in_slice_of_tissue_with_checked_operations(
                    num_units_along_y_axis,
                    num_units_along_x_axis,
                    num_units_along_columnar_axis));
}


}
