#ifndef CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED
#   define CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <tuple>

namespace cellconnect {


/**
 * The tuple defines a matrix whose elements will be mapped to particular cells in a tissue.
 * TODO!
 */
typedef std::tuple<
                //! A number of rows of the matrix
                natural_16_bit,
                //! A number of columns of the matrix
                natural_16_bit,
                //! The matrix stored here in the row-major order.
                //! We assume the row-axis is aligned with the x-axis of a tissue
                //! TODO!
                std::vector<natural_16_bit>
                >
        shift_template;

natural_16_bit  goto_left_up(natural_16_bit const ID);
natural_16_bit  goto_left(natural_16_bit const ID);
natural_16_bit  goto_left_down(natural_16_bit const ID);
natural_16_bit  goto_down(natural_16_bit const ID);
natural_16_bit  goto_right_down(natural_16_bit const ID);
natural_16_bit  goto_right(natural_16_bit const ID);
natural_16_bit  goto_right_up(natural_16_bit const ID);
natural_16_bit  goto_up(natural_16_bit const ID);

natural_16_bit  gofrom_left_up(natural_16_bit const ID);
natural_16_bit  gofrom_left(natural_16_bit const ID);
natural_16_bit  gofrom_left_down(natural_16_bit const ID);
natural_16_bit  gofrom_down(natural_16_bit const ID);
natural_16_bit  gofrom_right_down(natural_16_bit const ID);
natural_16_bit  gofrom_right(natural_16_bit const ID);
natural_16_bit  gofrom_right_up(natural_16_bit const ID);
natural_16_bit  gofrom_up(natural_16_bit const ID);


bool  compute_dimestions_repetitions_and_scales_for_templates_along_one_axis_of_tissue(
        natural_32_bit const  num_tissue_cells_along_the_axis,
        natural_16_bit const  dimension_of_the_central_template,
        natural_8_bit const  num_repetitions_of_the_central_template,
        natural_32_bit&  scale_of_all_templates,
        natural_16_bit&  dimension_of_the_front_template,
        natural_8_bit&  num_repetitions_of_the_front_template,
        natural_16_bit&  dimension_of_the_tail_template,
        natural_8_bit&  num_repetitions_of_the_tail_template
        );


struct column_shift_function
{
    /**
     * The default constructor initialises the identity shift function.
     */
    column_shift_function();

    column_shift_function(
            natural_32_bit const  num_cells_along_x_axis,
            natural_32_bit const  num_cells_along_y_axis,
            std::vector<shift_template > const&  shift_templates,
            natural_16_bit const  num_rows_in_layouts,
            natural_16_bit const  num_columns_in_layouts,
            std::vector<natural_16_bit> const&  layout_of_templates,
            std::vector<natural_32_bit> const&  layout_of_scales_along_x_axis_of_tissue,
            std::vector<natural_32_bit> const&  layout_of_scales_along_y_axis_of_tissue,
            std::vector< std::pair<natural_16_bit,natural_32_bit> > const&  layout_of_row_repetitions,
            std::vector< std::pair<natural_16_bit,natural_32_bit> > const&  layout_of_column_repetitions
            );

    std::pair<natural_32_bit,natural_32_bit>  operator()(natural_32_bit const  x_coord, natural_32_bit const  y_coord) const;

private:
    bool  m_use_identity_function;
    natural_32_bit const  m_num_cells_along_x_axis;
    natural_32_bit const  m_num_cells_along_y_axis;
    std::vector<shift_template>  m_shift_templates;
    natural_16_bit const  m_num_rows_in_layouts;
    natural_16_bit const  m_num_columns_in_layouts;
    std::vector<natural_16_bit>  m_layout_of_templates;
    std::vector<natural_32_bit>  m_layout_of_scales_along_x_axis_of_tissue;
    std::vector<natural_32_bit>  m_layout_of_scales_along_y_axis_of_tissue;
    std::vector< std::pair<natural_16_bit,natural_32_bit> >  m_layout_of_row_repetitions;
    std::vector< std::pair<natural_16_bit,natural_32_bit> >  m_layout_of_column_repetitions;
};


}

#endif
