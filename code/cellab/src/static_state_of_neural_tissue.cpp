#include <cellab/static_state_of_neural_tissue.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>

namespace cellab {


extern void check_for_unsigned_wrap_errors(static_state_of_neural_tissue const& tissue_props);


static_state_of_neural_tissue::static_state_of_neural_tissue(
        natural_16_bit const num_kinds_of_cells_in_neural_tissue,
        natural_16_bit const num_kinds_of_sensory_cells,
        natural_16_bit const num_bits_per_cell,
        natural_16_bit const num_bits_per_synapse,
        natural_16_bit const num_bits_per_signalling,
        natural_32_bit const num_tissue_cells_along_x_axis,
        natural_32_bit const num_tissue_cells_along_y_axis,
        std::vector<natural_32_bit> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
        std::vector<cellab::static_state_of_cell> const&
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
        std::vector<cellab::static_state_of_synapse> const&
            row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
        std::vector<cellab::static_state_of_synapse> const&
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
        std::vector<cellab::static_state_of_signalling> const&
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_signalling_data,
        natural_32_bit num_sensory_cells,
        natural_32_bit num_synapses_to_muscles
        )
    : m_num_kinds_of_tissue_cells(num_kinds_of_cells_in_neural_tissue)
    , m_num_kinds_of_sensory_cells(num_kinds_of_sensory_cells)
    , m_num_bits_per_cell(num_bits_per_cell)
    , m_num_bits_per_synapse(num_bits_per_synapse)
    , m_num_bits_per_signalling(num_bits_per_signalling)
    , m_num_tissue_cells_along_x_axis(num_tissue_cells_along_x_axis)
    , m_num_tissue_cells_along_y_axis(num_tissue_cells_along_y_axis)
    , m_num_tissue_cells_along_columnar_axis(
        [](std::vector<natural_32_bit> const& v)
        {
            ASSUMPTION(!v.empty());
            natural_64_bit sum = 0U;
            for (natural_32_bit i : v)
            {
                ASSUMPTION(i > 0U);
                sum += i;
            }
            ASSUMPTION((sum >> 32U) == 0U);
            return static_cast<natural_32_bit>(sum & 0xFFFFFFFF);
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
    , m_static_state_of_signalling_for_each_kind_of_sensory_cell(
        array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_signalling_data
        )
    , m_num_sensory_cells(num_sensory_cells)
    , m_num_synapses_to_muscles(num_synapses_to_muscles)
{
    ASSUMPTION(m_num_kinds_of_tissue_cells > 0U);
    ASSUMPTION(m_num_bits_per_cell > 0U);
    ASSUMPTION(m_num_bits_per_synapse > 0U);
    ASSUMPTION(m_num_tissue_cells_along_x_axis > 0U);
    ASSUMPTION(m_num_tissue_cells_along_y_axis > 0U);
    ASSUMPTION(m_num_tissue_cells_along_columnar_axis > 0U);
    ASSUMPTION(m_num_kinds_of_tissue_cells == m_num_tissue_cells_in_column_per_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells == m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_kinds_of_sensory_cells == m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells == m_static_state_of_signalling_for_each_kind_of_sensory_cell.size());
    ASSUMPTION(checked_mul_32_bit(static_cast<natural_32_bit>(m_num_kinds_of_tissue_cells),
                                  static_cast<natural_32_bit>(m_num_kinds_of_tissue_cells))
               == m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells.size());
    ASSUMPTION(m_num_kinds_of_tissue_cells ==
               m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell.size());
    ASSUMPTION(m_num_sensory_cells >= m_num_kinds_of_sensory_cells);

    // Now follows a code which early-detects wrap error for unsigned integers which could
    // otherwise occure later when computing addresses of bits of individual cells in
    // an instance of dynamic_state_of_neural_tissue.
    check_for_unsigned_wrap_errors(*this);

    LOG(debug,FUNCTION_PROTOTYPE());
}

static_state_of_neural_tissue::~static_state_of_neural_tissue()
{
    LOG(debug,FUNCTION_PROTOTYPE());
}

natural_16_bit static_state_of_neural_tissue::num_kinds_of_cells_in_neural_tissue() const
{
    return m_num_kinds_of_tissue_cells;
}

natural_16_bit static_state_of_neural_tissue::num_kinds_of_sensory_cells() const
{
    return m_num_kinds_of_sensory_cells;
}

natural_16_bit static_state_of_neural_tissue::num_bits_per_cell() const
{
    return m_num_bits_per_cell;
}

natural_16_bit static_state_of_neural_tissue::num_bits_per_synapse() const
{
    return m_num_bits_per_synapse;
}

natural_32_bit static_state_of_neural_tissue::num_tissue_cells_along_x_axis() const
{
    return m_num_tissue_cells_along_x_axis;
}

natural_32_bit static_state_of_neural_tissue::num_tissue_cells_along_y_axis() const
{
    return m_num_tissue_cells_along_y_axis;
}

natural_32_bit static_state_of_neural_tissue::num_tissue_cells_along_columnar_axis() const
{
    return m_num_tissue_cells_along_columnar_axis;
}

natural_32_bit static_state_of_neural_tissue::num_tissue_cells_in_column(
        natural_16_bit const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_num_tissue_cells_in_column_per_kind_of_tissue_cell[kind_of_tissue_cell];
}

natural_16_bit static_state_of_neural_tissue::compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
                    natural_32_bit position_of_tissue_cell_in_column) const
{
    ASSUMPTION(position_of_tissue_cell_in_column < num_tissue_cells_along_columnar_axis());
    natural_16_bit kind = 0U;
    while (position_of_tissue_cell_in_column >= num_tissue_cells_in_column(kind))
    {
        position_of_tissue_cell_in_column -= num_tissue_cells_in_column(kind);
        ++kind;
    }
    return kind;
}

static_state_of_cell const& static_state_of_neural_tissue::get_static_state_of_tissue_cell(
        natural_16_bit const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell.at(kind_of_tissue_cell);
}

static_state_of_cell const& static_state_of_neural_tissue::get_static_state_of_sensory_cell(
        natural_16_bit const kind_of_sensory_cell) const
{
    ASSUMPTION(kind_of_sensory_cell < num_kinds_of_sensory_cells());
    return m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell.at(kind_of_sensory_cell);
}

static_state_of_synapse const& static_state_of_neural_tissue::get_static_state_of_synapse(
        natural_16_bit const kind_of_source_tissue_cell,
        natural_16_bit const kind_of_target_tissue_cell) const
{
    ASSUMPTION(kind_of_source_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    ASSUMPTION(kind_of_target_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells.at(
                kind_of_source_tissue_cell * num_kinds_of_cells_in_neural_tissue() + kind_of_target_tissue_cell
                );
}

static_state_of_synapse const& static_state_of_neural_tissue::get_static_state_of_synapse_to_muscle(
        natural_16_bit const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell.at(kind_of_tissue_cell);
}

static_state_of_signalling const& static_state_of_neural_tissue::get_static_state_of_signalling(
        natural_16_bit const kind_of_tissue_cell) const
{
    ASSUMPTION(kind_of_tissue_cell < num_kinds_of_cells_in_neural_tissue());
    return m_static_state_of_signalling_for_each_kind_of_sensory_cell.at(kind_of_tissue_cell);
}

natural_32_bit static_state_of_neural_tissue::num_sensory_cells() const
{
    return m_num_sensory_cells;
}

natural_32_bit static_state_of_neural_tissue::num_synapses_to_muscles() const
{
    return m_num_synapses_to_muscles;
}


}
