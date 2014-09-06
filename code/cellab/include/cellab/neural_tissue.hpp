#ifndef CELLAB_NEURAL_TISSUE_HPP_INCLUDED
#   define CELLAB_NEURAL_TISSUE_HPP_INCLUDED

#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/territorial_state_of_synapse.hpp>
#   include <cellab/shift_in_coordinates.hpp>
#   include <cellab/transition_algorithms.hpp>

namespace cellab {


struct neural_tissue
{
    neural_tissue( std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_tissue );

    virtual ~neural_tissue() {}

    void compute_next_state_of_both_neural_tissue_and_environment(
            natural_32_bit max_number_of_threads_to_be_created_and_run_simultaneously
            );

private:

    virtual void transition_function_of_synapse_to_muscle(
            bits_reference& bits_of_synapse_to_be_updated,
            kind_of_cell kind_of_source_cell,
            bits_const_reference const& bits_of_source_cell
            );

    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> m_dynamic_state_of_tissue;

    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
        m_transition_function_of_packed_synapse_to_muscle;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
        m_transition_function_of_packed_synapse_inside_tissue;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
        m_transition_function_of_packed_signalling;
    single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
        m_transition_function_of_packed_cell;
};


}

#endif
