#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>


namespace cellab {


static_state_of_neural_tissue::static_state_of_neural_tissue(
        unsigned char const num_kinds_of_cells_in_neural_tissue,
        unsigned char const num_kinds_of_sensory_cells,
        unsigned char const num_bits_per_cell,
        unsigned char const num_bits_per_synapse,
        unsigned int const num_tissue_cells_along_x_axis,
        unsigned int const num_tissue_cells_along_y_axis,
        std::vector<unsigned short> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
        std::vector<cellab::static_state_of_synapse> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
        unsigned int num_sensory_cells,
        unsigned int num_synapses_to_muscles
        )
    : m_num_kinds_of_tissue_cells(num_kinds_of_cells_in_neural_tissue)
    , m_num_kinds_of_sensory_cells(num_kinds_of_sensory_cells)
    , m_num_bits_per_cell(num_bits_per_cell)
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_num_tissue_cells_along_x_axis(num_tissue_cells_along_x_axis)
    , m_num_tissue_cells_along_y_axis(num_tissue_cells_along_y_axis)
    , m_num_tissue_cells_along_columnar_axis(
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
        }(array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column)
        )
    , m_num_tissue_cells_in_column_per_kind_of_tissue_cell(
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column
        )
    , m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell(
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells
        )
    , m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell(
        array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells
        )
    , m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells(
        row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses
        )
    , m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell(
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles
        )
    , m_num_sensory_cells(num_sensory_cells)
    , m_num_synapses_to_muscles(num_synapses_to_muscles)
{
    ASSUMPTION(m_num_kinds_of_tissue_cells > 0);
    ASSUMPTION(m_num_bits_per_cell > 0);
    ASSUMPTION(m_num_bits_per_synapse > 0);
    ASSUMPTION(m_num_tissue_cells_along_x_axis > 0);
    ASSUMPTION(m_num_tissue_cells_along_y_axis > 0);
    ASSUMPTION(m_num_tissue_cells_along_columnar_axis > 0);
    ASSUMPTION(m_num_kinds_of_tissue_cells == m_num_tissue_cells_in_column_per_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells == m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_kinds_of_sensory_cells == m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells * m_num_kinds_of_tissue_cells ==
               m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells ==
               m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_sensory_cells >= m_num_kinds_of_sensory_cells);
    ASSUMPTION(sizeof(unsigned char) == 1);
    ASSUMPTION(sizeof(unsigned short) == 2);
    ASSUMPTION(sizeof(unsigned int) == 4);

    LOG(debug,FUNCTION_PROTOTYPE());
}

static_state_of_neural_tissue::~static_state_of_neural_tissue()
{
    LOG(debug,FUNCTION_PROTOTYPE());
}

unsigned char static_state_of_neural_tissue::num_kinds_of_cells_in_neural_tissue() const
{
    return m_num_kinds_of_tissue_cells;
}

unsigned char static_state_of_neural_tissue::num_kinds_of_sensory_cells() const
{
    return m_num_kinds_of_sensory_cells;
}

unsigned char static_state_of_neural_tissue::num_bits_per_cell() const
{
    return m_num_bits_per_cell;
}

unsigned char static_state_of_neural_tissue::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

unsigned int static_state_of_neural_tissue::num_tissue_cells_along_x_axis() const
{
    return m_num_tissue_cells_along_x_axis;
}

unsigned int static_state_of_neural_tissue::num_tissue_cells_along_y_axis() const
{
    return m_num_tissue_cells_along_y_axis;
}

unsigned int static_state_of_neural_tissue::num_tissue_cells_along_columnar_axis() const
{
    return m_num_tissue_cells_along_columnar_axis;
}

unsigned short static_state_of_neural_tissue::num_tissue_cells_in_column(
        unsigned char const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_num_tissue_cells_in_column_per_kind_of_tissue_cell[kind_of_tissue_cell];
}

unsigned char static_state_of_neural_tissue::compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                    unsigned int position_of_tissue_cell_in_column) const
{
    ASSUMPTION(position_of_tissue_cell_in_column < num_tissue_cells_along_columnar_axis());
    unsigned char kind = 0;
    while (position_of_tissue_cell_in_column >= num_tissue_cells_in_column(kind))
    {
        position_of_tissue_cell_in_column -= num_tissue_cells_in_column(kind);
        ++kind;
    }
    return kind;
}

static_state_of_cell const& static_state_of_neural_tissue::get_static_state_of_tissue_cell(
        unsigned char const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell.at(kind_of_tissue_cell);
}

static_state_of_cell const& static_state_of_neural_tissue::get_static_state_of_sensory_cell(
        unsigned char const kind_of_sensory_cell) const
{
    ASSUMPTION(kind_of_sensory_cell < num_kinds_of_sensory_cells());
    return m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell.at(kind_of_sensory_cell);
}

static_state_of_synapse const& static_state_of_neural_tissue::get_static_state_of_synapse(
        unsigned char const kind_of_source_tissue_cell,
        unsigned char const kind_of_target_tissue_cell) const
{
    ASSUMPTION(kind_of_source_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    ASSUMPTION(kind_of_target_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells.at(
                kind_of_source_tissue_cell * num_kinds_of_cells_in_neural_tissue() + kind_of_target_tissue_cell
                );
}

static_state_of_synapse const& static_state_of_neural_tissue::get_static_state_of_synapse_to_muscle(
        unsigned char const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell.at(kind_of_tissue_cell);
}


unsigned int static_state_of_neural_tissue::num_sensory_cells() const
{
    return m_num_sensory_cells;
}

unsigned int static_state_of_neural_tissue::num_synapses_to_muscles() const
{
    return m_num_synapses_to_muscles;
}


}
