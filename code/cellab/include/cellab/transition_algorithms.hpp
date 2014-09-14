#ifndef CELLAB_TRANSITION_ALGORITHMS_HPP_INCLUDED
#   define CELLAB_TRANSITION_ALGORITHMS_HPP_INCLUDED

#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/territorial_state_of_synapse.hpp>
#include <cellab/shift_in_coordinates.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>

namespace cellab {


typedef std::function<
            void(
                bits_reference& bits_of_synapse_to_be_updated,
                kind_of_cell kind_of_source_cell,
                bits_const_reference const& bits_of_source_cell
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle;

typedef std::function<
            territorial_state_of_synapse(
                bits_reference& bits_of_synapse_to_be_updated,
                kind_of_cell kind_of_source_cell,
                bits_const_reference const& bits_of_source_cell,
                kind_of_cell kind_of_territory_cell,
                bits_const_reference const& bits_of_territory_cell,
                territorial_state_of_synapse current_territorial_state_of_synapse,
                shift_in_coordinates const& shift_to_low_corner,
                shift_in_coordinates const& shift_to_high_corner,
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_signalling
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue;


typedef std::function<
            void(
                bits_reference& bits_of_signalling_data_to_be_updated,
                kind_of_cell kind_of_territory_cell,
                shift_in_coordinates const& shift_to_low_corner,
                shift_in_coordinates const& shift_to_high_corner,
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_cell
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling;

typedef std::function<
            void(
                bits_reference& bits_of_cell_to_be_updated,
                kind_of_cell kind_of_cell_to_be_updated,
                natural_32_bit num_of_synapses_connected_to_the_cell,
                std::function<std::tuple<bits_const_reference,kind_of_cell,kind_of_cell>(natural_32_bit)> const&
                    get_connected_synapse_at_index,
                shift_in_coordinates const& shift_to_low_corner,
                shift_in_coordinates const& shift_to_high_corner,
                std::function<std::pair<bits_const_reference,kind_of_cell>(shift_in_coordinates const&)> const&
                    get_signalling
                ) >
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell;


void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle const&
            transition_function_of_packed_synapse_to_muscle,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue const&
            transition_function_of_packed_synapse_inside_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void apply_transition_of_territorial_lists_of_synapses(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void  apply_transition_of_synaptic_migration_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void apply_transition_of_signalling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling const&
            transition_function_of_packed_signalling,
        natural_32_bit const  num_threads_avalilable_for_computation
        );

void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue> const dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell const&
            transition_function_of_packed_cell,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
