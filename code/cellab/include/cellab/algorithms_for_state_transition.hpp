#ifndef CELLAB_ALGORITHMS_FOR_STATE_TRANSITION_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_NEURAL_TISSUE_HPP_INCLUDED

#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>
#include <vector>

namespace cellab {


void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_synapse_to_be_updated
            // TODO!
            )> transition_function_of_synapse_to_muscle,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );

void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_synapse_to_be_updated,
            bits_const_reference bits_of_source_cell,
            bits_const_reference bits_of_the_cell_in_whose_territory_the_synapse_currently_appears,
            // TODO!
            )> transition_function_of_synapse_inside_tissue,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );

void apply_transition_of_synaptic_migration_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            // TODO!
            )> transition_function_of_,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );

void apply_transition_of_spaialy_local_intercellular_signaling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_signaling_data_to_be_updated,
            std::vector<bits_const_reference> const& bits_of_all_cells_in_neighbourhood,
            )> transition_function_of_local_intercellular_signaling,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );

void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_cell_to_be_updated,
            std::vector<bits_const_reference> bits_of_all_connected_synapses,
            bits_const_reference bits_of_signaling_data_of_the_updated_cell,
            std::vector<bits_const_reference> const& bits_of_signaling_data_of_all_cells_in_neighbourhood,
            )> transition_function_of_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        );


}

#endif
