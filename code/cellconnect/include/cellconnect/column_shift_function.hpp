#ifndef CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED
#   define CELLCONNECT_COLUMN_SHIFT_FUNCTION_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <tuple>
#   include <map>

namespace cellconnect {


enum exit_shift_kind
{
    DIR_LEFT_UP = 0U,
    DIR_LEFT = 1U,
    DIR_LEFT_DOWN = 2U,
    DIR_DOWN = 3U,
    DIR_RIGHT_DOWN = 4U,
    DIR_RIGHT = 5U,
    DIR_RIGHT_UP = 6U,
    DIR_UP = 7U,
};

exit_shift_kind  get_opposite_exit_shift_kind(exit_shift_kind const  kind);


struct shift_to_target
{
    shift_to_target(natural_16_bit const target_row, natural_16_bit  target_column);
    shift_to_target(exit_shift_kind const  shift_kind, natural_16_bit const  shift_ID);

    bool  is_external() const { return m_is_external; }

    /**
     * The following two functions can be called only when is_external() returns false.
     */

    natural_16_bit  get_target_row() const;
    natural_16_bit  get_target_column() const;

    /**
     * The following two functions can be called only when is_external() returns true.
     */

    exit_shift_kind  get_exit_shift_kind() const;
    natural_16_bit  get_exit_shift_ID() const;

private:
    bool  m_is_external;
    natural_16_bit  m_target_row;
    natural_16_bit  m_target_column;
    exit_shift_kind  m_exit_shift_kind;
    natural_16_bit  m_external_shift_ID;
};


/**
 * It is a recipe form construction of mapping from xy-coordinates to xy-coordinates in the tissue.
 * Templates are mapped into certain regions of the neural tissue, where they define shift in tissue
 * xy-coordinates of the resulting 'column_shift_function' (see bellow). A shift template can be seen
 * as a matrix whose row-axis is aligned with the x-axis of a tissue.
 *
 * Details about the structure of and usage of a shift template can be found in the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellconnect/cellconnect.html#column_shift_function
 */
struct shift_template
{
    /**
     * This constructor initialises a shift template of size num_rows*num_columns automatically.
     */
    shift_template(natural_16_bit const num_rows, natural_16_bit const num_columns,
                   std::vector< std::pair<exit_shift_kind,natural_16_bit> > const& exit_counts
                        = std::vector<  std::pair<exit_shift_kind,natural_16_bit>>()
                       //!< Use this vector if you want your template to have exits (and entries) to adjacent shift
                       //!< templates. Each pair std::pair<exit_shift_kind,natural_16_bit> specifies a direction
                       //!< to an adjacent template and a count of exits to that template.
                   );

    /**
     * Use this constructor, if you want to define each element of the shift template by yourself.
     */
    shift_template(natural_16_bit const num_rows, natural_16_bit const num_columns,
                   std::vector<shift_to_target> const& shifts,
                       //!< Vector of num_rows*num_columns elements of the shift matrix in row-major order.
                   std::map< std::pair<exit_shift_kind,natural_16_bit>,std::pair<natural_16_bit,natural_16_bit>  > const&
                       from_entry_specification_to_entry_coords
                       //!< Each element of the matrix which is not a target of any element of the matrix
                       //!< is called entry element. It is supposed to be a target from some adjacent
                       //!< template. An entry is referenced from an adjacent matrix using a pair
                       //!< std::pair<exit_shift_kind,natural_16_bit>, where exit_shift_kind specifies
                       //!< a direction FROM the adjacent template to this one and natural_16_bit is a unique ID
                       //!< (to distinguish entries from that direction). This template is supposed to return
                       //!< coordinates for a specified entry.
                   );

    natural_16_bit  num_rows() const { return m_num_rows; }
    natural_16_bit  num_columns() const { return m_num_columns; }

    shift_to_target  get_shift(natural_16_bit const  row_index, natural_16_bit const  column_index) const;
    void  set_shift(natural_16_bit const  row_index, natural_16_bit const  column_index, shift_to_target const& shift);

    std::pair<natural_16_bit,natural_16_bit> const&  get_entry_shift(exit_shift_kind const direction_from_adjacent_matrix,
                                                                     natural_16_bit const  shift_ID) const;

    std::map< std::pair<exit_shift_kind,natural_16_bit>,std::pair<natural_16_bit,natural_16_bit> > const&
        get_map_from_entry_specification_to_entry_coords() const noexcept { return m_from_entry_specification_to_entry_coords; }

private:
    natural_16_bit  get_index(natural_16_bit const  row_index, natural_16_bit const  column_index) const
    { return row_index * num_columns() + column_index; }
    shift_to_target  get_coords(natural_16_bit const  index) const
    { return { (natural_16_bit)(index / num_columns()), (natural_16_bit)(index % num_columns()) }; }

    void  setup_data(std::vector<shift_to_target> const& shifts);
    bool  check_consistency() const;

    natural_16_bit  m_num_rows;
    natural_16_bit  m_num_columns;
    std::vector<natural_16_bit>  m_data; //!< The matrix is stored here in the row-major order.
    std::map< std::pair<exit_shift_kind,natural_16_bit>,std::pair<natural_16_bit,natural_16_bit> >
        m_from_entry_specification_to_entry_coords;
};


void  invert_map_from_entry_specification_to_entry_coords_of_shift_template(
        std::map< std::pair<exit_shift_kind,natural_16_bit>,std::pair<natural_16_bit,natural_16_bit> > const& src_map,
        std::map< std::pair<natural_16_bit,natural_16_bit>,std::vector<std::pair<exit_shift_kind,natural_16_bit> > >&
            output_map
        );


/**
 * It defines a layout matrix of indices into some std::vector<shift_template> of shift templates.
 * Each index into the vector must appear at least once in the matrix.
 * We assume the row-axis of the matrix is aligned with the x-axis of a tissue.
 */
struct layout_of_shift_templates
{
    layout_of_shift_templates(natural_16_bit const  num_rows, natural_16_bit const  num_columns,
                              std::vector<natural_16_bit> const&  matrix_of_indices_of_shift_templates
                                  //!< The layout matrix is stored in this vector in the row-major order.
                              );

    natural_16_bit num_rows() const { return m_num_rows; }
    natural_16_bit num_columns() const { return m_num_columns; }
    natural_16_bit  num_templates() const { return m_num_templates; }
    natural_16_bit  get_template_index(natural_16_bit const  row_index, natural_16_bit const  column_index) const;

private:
    natural_16_bit  get_index(natural_16_bit const  row_index, natural_16_bit const  column_index) const
    { return row_index * num_columns() + column_index; }

    natural_16_bit  m_num_rows;
    natural_16_bit  m_num_columns;
    std::vector<natural_16_bit>  m_layout; //!< The matrix is stored here in the row-major order.
    natural_16_bit  m_num_templates; //!< The maximal index in m_layout inreased by 1
};


/**
 * It defines number of repetitions of a block of adjacent rows or columns in an instance of struct layout_of_shift_templates.
 * The block is defined as a pair of indices [begin_index,end_index) of adjacent rows or columns.
 */
struct repetition_block
{
    repetition_block(natural_16_bit const  block_begin_index, natural_16_bit const  block_end_index,
                     natural_16_bit const  block_num_repetitions);

    natural_16_bit block_begin_index() const { return m_block_begin_index; }
    natural_16_bit block_end_index() const { return m_block_end_index; }
    natural_16_bit block_num_repetitions() const { return m_block_num_repetitions; }

private:
    natural_16_bit  m_block_begin_index;
    natural_16_bit  m_block_end_index;
    natural_16_bit  m_block_num_repetitions;
};


/**
 * It defines a bijective function, say F, from xy-coordinates of all cells in a tissue to xy-coordinates of
 * all cells in the tissue. If [x,y] are valid coordinates of a cell in a tissue, then F([x,y]) are also valid
 * coordinates of a cell in the tissue. There are implemented two kinds of F: (1) Identity function, and (2)
 * Tempate-based one. The second kind can be seen as tilling a floor (tissue) with a simple mosaic of standardised
 * bricks (templates). A template is a small version of F, i.e. a bijective funtion on xy-coordinates, applied
 * only to a (small) rectangular area inside the tissue. For detailed description see the documentation:
 *
 *      file:///<E2-root-dir>/doc/project_documentation/cellconnect/cellconnect.html
 *
 */
struct column_shift_function
{
    /**
     * The default constructor initialises the identity shift function.
     * It is used for speading synapses into territories in local neghbourhoods.
     */
    column_shift_function();

    /**
     * It creates a permutation of coordinates of cells.
     * It is used for speading synapses into territories in distant neghbourhoods.
     */
    column_shift_function(
            natural_32_bit const  num_cells_along_x_axis,
            natural_32_bit const  num_cells_along_y_axis,
            std::vector<shift_template> const&  shift_templates,
            natural_32_bit const  scale_of_shift_templates_along_row_axis,
            natural_32_bit const  scale_of_shift_templates_along_column_axis,
            layout_of_shift_templates const&  layout_of_shift_templates,
            std::vector<repetition_block> const&  repetition_blocks_of_rows_in_layout_of_shift_templates,
            std::vector<repetition_block> const&  repetition_blocks_of_columns_in_layout_of_shift_templates
            );

    std::pair<natural_32_bit,natural_32_bit>  operator()(natural_32_bit const  x_coord, natural_32_bit const  y_coord) const;

    bool is_identity_function() const { return m_use_identity_function; }

    natural_32_bit num_cells_along_x_axis() const { return m_num_cells_along_x_axis; }
    natural_32_bit num_cells_along_y_axis() const { return m_num_cells_along_y_axis; }

    natural_32_bit scale_along_row_axis() const { return m_scale_of_shift_templates_along_row_axis; }
    natural_32_bit scale_along_column_axis() const { return m_scale_of_shift_templates_along_column_axis; }

    natural_16_bit num_layout_rows() const { return m_layout_of_shift_templates.num_rows(); }
    natural_16_bit num_layout_columns() const { return m_layout_of_shift_templates.num_columns(); }
    shift_template const&  get_shift_template(natural_16_bit const  layout_row_index, natural_16_bit const  layout_column_index) const;
    layout_of_shift_templates const&  get_layout_of_shift_templates() const noexcept { return m_layout_of_shift_templates; }

    natural_16_bit num_templates() const { return m_layout_of_shift_templates.num_templates(); }
    shift_template const&  get_shift_template(natural_16_bit const  template_index) const;

    natural_16_bit  num_row_repetition_blocks() const { return (natural_16_bit)m_repetition_blocks_of_rows_in_layout_of_shift_templates.size(); }
    natural_16_bit  num_column_repetition_blocks() const { return (natural_16_bit)m_repetition_blocks_of_columns_in_layout_of_shift_templates.size(); }
    repetition_block const& get_row_repetition_block(natural_16_bit const  row_block_index) const;
    repetition_block const& get_column_repetition_block(natural_16_bit const  column_block_index) const;

    natural_32_bit  num_repetition_block_cells_along_x_axis(natural_16_bit const  row_block_index) const;
    natural_32_bit  num_repetition_block_cells_along_y_axis(natural_16_bit const  column_block_index) const;
    natural_32_bit  num_shift_template_cells_along_x_axis(natural_16_bit const  layout_row_index) const;
    natural_32_bit  num_shift_template_cells_along_y_axis(natural_16_bit const  layout_column_index) const;

private:
    void  build_repetitions(std::vector<repetition_block> const&  reps, natural_16_bit const  size,
                            std::vector<repetition_block>&  output);

    bool  check_layout_consistency() const;
    bool  check_consistency_between_layout_and_tissue() const;

    bool  m_use_identity_function;
    natural_32_bit  m_num_cells_along_x_axis;
    natural_32_bit  m_num_cells_along_y_axis;
    std::vector<shift_template>  m_shift_templates;
    natural_32_bit  m_scale_of_shift_templates_along_row_axis;
    natural_32_bit  m_scale_of_shift_templates_along_column_axis;
    layout_of_shift_templates  m_layout_of_shift_templates;
    std::vector<repetition_block>  m_repetition_blocks_of_rows_in_layout_of_shift_templates;
    std::vector<repetition_block>  m_repetition_blocks_of_columns_in_layout_of_shift_templates;
};


void  compute_tissue_axis_length_and_template_scale(
        natural_32_bit const  desired_number_of_cells_along_one_axis_of_tissue,
        natural_16_bit const  largest_template_dimension,
        natural_16_bit const  num_repetitions_of_largest_template_dimension,
        natural_32_bit&  num_tissue_cells_along_the_axis,
        natural_32_bit&  scale_of_all_template_dimensions
        );


}

#endif
