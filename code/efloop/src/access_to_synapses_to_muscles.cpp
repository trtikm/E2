#include <efloop/access_to_synapses_to_muscles.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>

namespace efloop {


access_to_synapses_to_muscles::access_to_synapses_to_muscles(
        std::shared_ptr<cellab::neural_tissue> const  neural_tissue,
        std::mutex* const  mutex_to_synapses_to_muscles
        )
    : m_neural_tissue(neural_tissue)
    , m_mutex_to_synapses_to_muscles(
        mutex_to_synapses_to_muscles != nullptr ? mutex_to_synapses_to_muscles : &void_mutex
        )
{
    ASSUMPTION(m_neural_tissue.operator bool());
}

natural_16_bit  access_to_synapses_to_muscles::num_kinds_of_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_kinds_of_cells();
}

natural_16_bit  access_to_synapses_to_muscles::num_kinds_of_tissue_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_kinds_of_tissue_cells();
}

bool  access_to_synapses_to_muscles::is_it_kind_of_tissue_cell(cellab::kind_of_cell cell_kind) const
{
    return cell_kind < m_neural_tissue->get_static_state_of_neural_tissue()->num_kinds_of_tissue_cells();
}

bool  access_to_synapses_to_muscles::is_it_kind_of_sensory_cell(cellab::kind_of_cell cell_kind) const
{
    return cell_kind >= num_kinds_of_tissue_cells() && cell_kind < num_kinds_of_cells();
}

natural_32_bit access_to_synapses_to_muscles::num_synapses_to_muscles() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_synapses_to_muscles();
}

std::pair<bits_const_reference,cellab::kind_of_cell>
access_to_synapses_to_muscles::get_bits_of_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle
        ) const
{
    ASSUMPTION(index_of_synapse_to_muscle < num_synapses_to_muscles());

    std::lock_guard<std::mutex> const  lock_access_to_synapses_to_muscles(*m_mutex_to_synapses_to_muscles);

    bits_const_reference bits =
            m_neural_tissue->get_dynamic_state_of_neural_tissue()->find_bits_of_synapse_to_muscle(
                    index_of_synapse_to_muscle
                    );

    cellab::tissue_coordinates const source_cell_coords(
                cellab::get_coordinates_of_source_cell_of_synapse_to_muscle(
                        m_neural_tissue->get_dynamic_state_of_neural_tissue(),
                        index_of_synapse_to_muscle
                        )
                );

    cellab::kind_of_cell const cell_kind =
        m_neural_tissue->get_static_state_of_neural_tissue()->compute_kind_of_cell_from_its_position_along_columnar_axis(
                source_cell_coords.get_coord_along_columnar_axis()
                );

    return std::make_pair( bits, cell_kind );
}


}
