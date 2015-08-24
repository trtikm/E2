#ifndef CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED
#   define CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <tuple>

namespace cellconnect {


enum exit_shift_kind
{
    GOTO_LEFT_UP = 0U,
    GOTO_LEFT = 1U,
    GOTO_LEFT_DOWN = 2U,
    GOTO_DOWN = 3U,
    GOTO_RIGHT_DOWN = 4U,
    GOTO_RIGHT = 5U,
    GOTO_RIGHT_UP = 6U,
    GOTO_UP = 7U,
};


struct shift_to_target
{
    shift_to_target(natural_16_bit const target_row, natural_16_bit  target_column);
    shift_to_target(exit_shift_kind const  shift_kind, natural_16_bit const  shift_ID);

    bool  is_external() const { return m_is_external; }

    natural_16_bit  get_target_row() const;
    natural_16_bit  get_target_column() const;

    exit_shift_kind  get_exit_shift_kind() const;
    natural_16_bit  get_external_shift_ID() const;

private:
    bool  m_is_external;
    natural_16_bit  m_target_row;
    natural_16_bit  m_target_column;
    exit_shift_kind  m_exit_shift_kind;
    natural_16_bit  m_external_shift_ID;
};


/**
 * It defines a matrix whose elements will be mapped to particular cells in a tissue.
 * We assume the row-axis of the matrix is aligned with the x-axis of a tissue.
 * TODO: add more details!
 */
struct shift_template
{
    shift_template(natural_16_bit const num_rows, natural_16_bit const num_columns,
                   std::vector< std::pair<exit_shift_kind,natural_16_bit> > const& exit_counts
                        = std::vector<  std::pair<exit_shift_kind,natural_16_bit>>());
    shift_template(std::vector<shift_to_target> const& shifts,
                   natural_16_bit const num_rows, natural_16_bit const num_columns);

    natural_16_bit  num_rows() const { return m_num_rows; }
    natural_16_bit  num_columns() const { return m_num_columns; }

    shift_to_target  get_shift(natural_16_bit const  row_index, natural_16_bit const  column_index) const;
    void  set_shift(natural_16_bit const  row_index, natural_16_bit const  column_index, shift_to_target const& shift);

private:
    natural_16_bit  get_index(natural_16_bit const  row_index, natural_16_bit const  column_index) const
    { return row_index * num_columns() + column_index; }
    shift_to_target  get_coords(natural_16_bit const  index) const
    { return { (natural_16_bit)(index / num_columns()), (natural_16_bit)(index % num_columns()) }; }

    void  setup_data(std::vector<shift_to_target> const& shifts);
    bool  check_consistency() const;

    natural_16_bit  m_num_rows;
    natural_16_bit  m_num_columns;
    std::vector<natural_16_bit>  m_data; //!< The matrix stored here in the row-major order.
};


struct column_shift_function
{
    /**
     * The default constructor initialises the identity shift function.
     */
    column_shift_function();

    column_shift_function(
            natural_32_bit const  num_cells_along_x_axis,
            natural_32_bit const  num_cells_along_y_axis,
            std::vector<shift_template> const&  shift_templates,
            natural_16_bit const  num_rows_in_layouts,
            natural_16_bit const  num_columns_in_layouts,
            std::vector<natural_16_bit> const&  layout_of_templates,
            std::vector<natural_32_bit> const&  layout_of_scales_along_row_axis,
            std::vector<natural_32_bit> const&  layout_of_scales_along_column_axis,
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
    std::vector<natural_32_bit>  m_layout_of_scales_along_row_axis;
    std::vector<natural_32_bit>  m_layout_of_scales_along_column_axis;
    std::vector< std::pair<natural_16_bit,natural_32_bit> >  m_layout_of_row_repetitions;
    std::vector< std::pair<natural_16_bit,natural_32_bit> >  m_layout_of_column_repetitions;
};


void  compute_tissue_axis_length_and_template_scale(
        natural_32_bit const  desired_number_of_cells_along_one_axis_of_tissue,
        natural_16_bit const  dimension_of_template,
        natural_8_bit const  num_repetitions_of_template,
        natural_32_bit&  num_tissue_cells_along_the_axis,
        natural_32_bit&  scale_of_template
        );


}

#endif
