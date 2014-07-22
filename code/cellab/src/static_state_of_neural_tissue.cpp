#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>


namespace cellab {


static_state_of_neural_tissue::static_state_of_neural_tissue(
        unsigned char const num_kinds_of_cells,
        unsigned char const num_bits_per_cell,
        unsigned char const num_bits_per_synapse,
        unsigned int const num_cells_along_x_axis,
        unsigned int const num_cells_along_y_axis,
        std::vector<unsigned short> const&
            array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
        )
    : m_num_cells_along_x_axis(num_cells_along_x_axis)
    , m_num_cells_along_y_axis(num_cells_along_y_axis)
    , m_num_cells_along_columnar_axis(
        [](std::vector<unsigned short> const& v)
        {
            ASSUMPTION(!v.empty());
            unsigned int sum = 0;
            for (unsigned short i : v)
            {
                ASSUMPTION(i > 0);
                sum += i;
            }
            return sum;
        }(array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column)
        )
    , m_num_kinds_of_cells(num_kinds_of_cells)
    , m_num_bits_per_cell(num_bits_per_cell)
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_num_cells_in_column_per_kind_of_cell(
          array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column
          )
    , m_static_state_of_cell_per_kind_of_cell(
          array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells
          )
    , m_static_state_of_synapse_per_pair_of_kinds_of_cells(
          row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
          )
{
    ASSUMPTION(m_num_cells_along_x_axis > 0);
    ASSUMPTION(m_num_cells_along_y_axis > 0);
    ASSUMPTION(m_num_cells_along_columnar_axis > 0);
    ASSUMPTION(m_num_kinds_of_cells > 0);
    ASSUMPTION(m_num_bits_per_cell > 0);
    ASSUMPTION(m_num_bits_per_synapse > 0);
    ASSUMPTION(m_num_kinds_of_cells == m_num_cells_in_column_per_kind_of_cell.size());
    ASSUMPTION(m_num_kinds_of_cells == m_static_state_of_cell_per_kind_of_cell.size());
    ASSUMPTION(m_num_kinds_of_cells * m_num_kinds_of_cells == m_static_state_of_synapse_per_pair_of_kinds_of_cells.size());

    LOG(debug,FUNCTION_PROTOTYPE());
}

static_state_of_neural_tissue::~static_state_of_neural_tissue()
{
    LOG(debug,FUNCTION_PROTOTYPE());
}

unsigned char static_state_of_neural_tissue::num_kinds_of_cells() const
{
    return m_num_kinds_of_cells;
}

unsigned char static_state_of_neural_tissue::num_bits_per_cell() const
{
    return m_num_bits_per_cell;
}

unsigned char static_state_of_neural_tissue::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

unsigned int static_state_of_neural_tissue::num_cells_along_x_axis() const
{
    return m_num_cells_along_x_axis;
}

unsigned int static_state_of_neural_tissue::num_cells_along_y_axis() const
{
    return m_num_cells_along_y_axis;
}

unsigned int static_state_of_neural_tissue::num_cells_along_columnar_axis() const
{
    return m_num_cells_along_columnar_axis;
}

unsigned short static_state_of_neural_tissue::num_cells_in_column(unsigned char const kind_of_cell) const
{
    ASSUMPTION(kind_of_cell < num_kinds_of_cells());
    return m_num_cells_in_column_per_kind_of_cell[kind_of_cell];
}

unsigned char static_state_of_neural_tissue::compute_kind_of_cell_from_its_position_along_columnar_axis(
                    unsigned int position_of_cell_in_column) const
{
    ASSUMPTION(position_of_cell_in_column < num_kinds_of_cells());
    unsigned char kind = 0;
    while (position_of_cell_in_column >= num_cells_in_column(kind))
    {
        position_of_cell_in_column -= num_cells_in_column(kind);
        ++kind;
    }
    return kind;
}

static_state_of_cell const& static_state_of_neural_tissue::get_static_state_of_cell(
        unsigned char const kind_of_cell) const
{
    ASSUMPTION(kind_of_cell < num_kinds_of_cells());
    return m_static_state_of_cell_per_kind_of_cell.at(kind_of_cell);
}

static_state_of_synapse const& static_state_of_neural_tissue::get_static_state_of_synapse(
        unsigned char const kind_of_source_cell,
        unsigned char const kind_of_target_cell) const
{
    ASSUMPTION(kind_of_source_cell < num_kinds_of_cells());
    ASSUMPTION(kind_of_target_cell < num_kinds_of_cells());
    return m_static_state_of_synapse_per_pair_of_kinds_of_cells.at(
                kind_of_source_cell * num_kinds_of_cells() + kind_of_target_cell
                );
}


}
