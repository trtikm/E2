#ifndef CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_cell.hpp>
#   include <cellab/static_state_of_synapse.hpp>
#   include <cellab/static_state_of_signalling.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/noncopyable.hpp>
#   include <boost/scoped_array.hpp>
#   include <vector>

namespace cellab {


struct static_state_of_neural_tissue : private boost::noncopyable
{
    static_state_of_neural_tissue(
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
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_signalling_data,
        natural_32_bit num_sensory_cells,
        natural_32_bit num_synapses_to_muscles
        );

    ~static_state_of_neural_tissue();

    natural_16_bit num_kinds_of_cells_in_neural_tissue() const;
    natural_16_bit num_kinds_of_sensory_cells() const;

    natural_16_bit num_bits_per_cell() const;
    natural_16_bit num_bits_per_synapse() const;
    natural_16_bit num_bits_per_signalling() const;

    natural_32_bit num_tissue_cells_along_x_axis() const;
    natural_32_bit num_tissue_cells_along_y_axis() const;
    natural_32_bit num_tissue_cells_along_columnar_axis() const;
    natural_32_bit num_tissue_cells_in_column(natural_16_bit const kind_of_tissue_cell) const;

    natural_16_bit compute_kind_of_tissue_cell_from_its_position_along_columnar_axis(
            natural_32_bit position_of_tissue_cell_in_column) const;

    static_state_of_cell const& get_static_state_of_tissue_cell(
            natural_16_bit const kind_of_tissue_cell) const;

    static_state_of_cell const& get_static_state_of_sensory_cell(
            natural_16_bit const kind_of_sensory_cell) const;

    static_state_of_synapse const& get_static_state_of_synapse(
            natural_16_bit const kind_of_source_tissue_cell,
            natural_16_bit const kind_of_target_tissue_cell) const;

    static_state_of_synapse const& get_static_state_of_synapse_to_muscle(
            natural_16_bit const kind_of_tissue_cell) const;

    static_state_of_signalling const& get_static_state_of_signalling(natural_16_bit const kind_of_tissue_cell) const;

    natural_32_bit num_sensory_cells() const;
    natural_32_bit num_synapses_to_muscles() const;

private:
    natural_16_bit m_num_kinds_of_tissue_cells;
    natural_16_bit m_num_kinds_of_sensory_cells;

    natural_16_bit m_num_bits_per_cell;
    natural_16_bit m_num_bits_per_synapse;
    natural_16_bit m_num_bits_per_signalling;

    natural_32_bit m_num_tissue_cells_along_x_axis;
    natural_32_bit m_num_tissue_cells_along_y_axis;
    natural_32_bit m_num_tissue_cells_along_columnar_axis;

    std::vector<natural_32_bit> m_num_tissue_cells_in_column_per_kind_of_tissue_cell;
    std::vector<static_state_of_cell> m_static_state_of_tissue_cell_for_each_kind_of_tissue_cell;
    std::vector<static_state_of_cell> m_static_state_of_sensory_cell_for_each_kind_of_sensory_cell;
    std::vector<static_state_of_synapse> m_static_state_of_synapse_for_each_pair_of_kinds_of_tissue_cells;
    std::vector<static_state_of_synapse> m_static_state_of_synapse_to_muscle_for_each_kind_of_tissue_cell;
    std::vector<static_state_of_signalling> m_static_state_of_signalling_for_each_kind_of_tissue_cell;

    natural_32_bit m_num_sensory_cells;
    natural_32_bit m_num_synapses_to_muscles;
};


}

#endif
