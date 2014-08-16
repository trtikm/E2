#ifndef CELLAB_TRANSITION_FUNCTIONS_OF_PACKED_TISSUE_ELEMENTS_HPP_INCLUDED
#   define CELLAB_TRANSITION_FUNCTIONS_OF_PACKED_TISSUE_ELEMENTS_HPP_INCLUDED

#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/convertors_from_packed_to_unpacked_dynamic_states.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>

namespace cellab {


template<typename type_of_unpacked_dynamic_state_of_synapse_in_tissue,
         typename type_of_unpacked_dynamic_state_of_cell_in_tissue>
void single_threaded_transition_function_of_packed_synapse_to_muscle(
        std::function<
            void(
            type_of_unpacked_dynamic_state_of_synapse_in_tissue& synapse_to_be_updated,
            static_state_of_synapse const& static_state_of_updated_synapse,
            type_of_unpacked_dynamic_state_of_cell_in_tissue const& dynamic_state_of_source_cell,
            static_state_of_cell const& static_state_of_source_cell,
            )> single_threaded_transition_function_of_unpacked_synapse_to_muscle,
        bits_reference bits_of_synapse_to_be_updated,
        static_state_of_synapse const& static_state_of_updated_synapse,
        bits_const_reference bits_of_source_cell,
        static_state_of_cell const& static_state_of_source_cell,
        )
{
    type_of_unpacked_dynamic_state_of_synapse_in_tissue
            synapse_to_be_updated(bits_of_synapse_to_be_updated);
    type_of_unpacked_dynamic_state_of_cell_in_tissue
            dynamic_state_of_source_cell(bits_of_source_cell);
    single_threaded_transition_function_of_unpacked_synapse_to_muscle(
                synapse_to_be_updated,
                static_state_of_updated_synapse,
                dynamic_state_of_source_cell,
                static_state_of_source_cell
                );
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;
}


template<typename type_of_unpacked_dynamic_state_of_synapse_in_tissue,
         typename type_of_unpacked_dynamic_state_of_cell_in_tissue,
         typename type_of_unpacked_dynamic_state_of_signaling_data>
void single_threaded_transition_function_of_packed_synapse_inside_tissue(
        std::function<
            void(
            type_of_unpacked_dynamic_state_of_synapse_in_tissue& synapse_to_be_updated,
            static_state_of_synapse const& static_state_of_updated_synapse,
            type_of_unpacked_dynamic_state_of_cell_in_tissue const& dynamic_state_of_source_cell,
            static_state_of_cell const& static_state_of_source_cell,
            type_of_unpacked_dynamic_state_of_cell_in_tissue const& dynamic_state_of_target_cell,
            static_state_of_cell const& static_state_of_target_cell,
            std::function<type_of_unpacked_dynamic_state_of_signaling_data(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated synapse, see 'static_state_of_updated_synapse' data.
                // NOTE: Coordinates (0,0,0) reference the signalling data in the territory of the target cell.
                get_signalling_data_in_neighbourhood_of_the_synapse_at_given_coordiates,
            static_state_of_signalling const& static_state_of_signalling_data
            )> single_threaded_transition_function_of_unpacked_synapse_inside_tissue,
        bits_reference bits_of_synapse_to_be_updated,
        static_state_of_synapse const& static_state_of_updated_synapse,
        bits_const_reference bits_of_source_cell,
        static_state_of_cell const& static_state_of_source_cell,
        bits_const_reference bits_of_target_cell_in_whose_territory_the_synapse_currently_appears,
        static_state_of_cell const& static_state_of_target_cell,
        std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
            get_bits_of_signalling_data_in_neighbourhood_of_the_synapse_at_given_coordiates,
        static_state_of_signalling const& static_state_of_signalling_data
        )
{
    type_of_unpacked_dynamic_state_of_synapse_in_tissue
            synapse_to_be_updated(bits_of_synapse_to_be_updated);
    type_of_unpacked_dynamic_state_of_cell_in_tissue const
            dynamic_state_of_source_cell(bits_of_source_cell);
    type_of_unpacked_dynamic_state_of_cell_in_tissue const
            dynamic_state_of_target_cell(bits_of_target_cell_in_whose_territory_the_synapse_currently_appears);
    single_threaded_transition_function_of_unpacked_synapse_inside_tissue(
                synapse_to_be_updated,
                static_state_of_updated_synapse,
                dynamic_state_of_source_cell,
                static_state_of_source_cell,
                dynamic_state_of_target_cell,
                static_state_of_target_cell,
                std::bind(&private_internal_implementation_details::get_instance_at_coordinates<
                                type_of_unpacked_dynamic_state_of_signaling_data>,
                          get_bits_of_signalling_data_in_neighbourhood_of_the_synapse_at_given_coordiates,
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                static_state_of_signalling_data
                );
    synapse_to_be_updated >> bits_of_synapse_to_be_updated;
}

template<typename type_of_unpacked_dynamic_state_of_signaling_data,
         typename type_of_unpacked_dynamic_state_of_cell_in_tissue>
void single_threaded_transition_function_of_packed_signalling(
        std::function<
            void(
            type_of_unpacked_dynamic_state_of_signaling_data& signalling_data_to_be_updated,
            static_state_of_signalling const& static_state_of_signalling_data,
            std::function<type_of_unpacked_dynamic_state_of_cell_in_tissue(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated signalling data, see 'static_state_of_signalling_data' data.
                // NOTE: Coordinates (0,0,0) reference the cell of the updated signalling data.
                get_dynamic_state_of_cell_at_given_coordiates,
            std::function<static_state_of_cell const&(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated signalling data, see 'static_state_of_signalling_data' data.
                // NOTE: Coordinates (0,0,0) reference the static state of cell of the updated signalling data.
                get_static_state_of_cell_at_given_coordiates,
            )> single_threaded_transition_function_of_unpacked_signalling,
        bits_reference bits_of_signalling_data_to_be_updated,
        static_state_of_signalling const& static_state_of_signalling_data,
        std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
            get_bits_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates,
        std::function<static_state_of_cell const&(natural_8_bit,natural_8_bit,natural_8_bit)>
            get_static_state_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates,
        )
{
    type_of_unpacked_dynamic_state_of_signaling_data
            signalling_data_to_be_updated(bits_of_signalling_data_to_be_updated);
    single_threaded_transition_function_of_unpacked_signalling(
                signalling_data_to_be_updated,
                static_state_of_signalling_data,
                std::bind(&private_internal_implementation_details::get_instance_at_index<
                                type_of_unpacked_dynamic_state_of_cell_in_tissue>,
                          get_bits_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates,
                          std::placeholders::_1),
                get_static_state_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates
                );
    signalling_data_to_be_updated >> bits_of_signalling_data_to_be_updated;
}

template<typename type_of_unpacked_dynamic_state_of_cell_in_tissue,
         typename type_of_unpacked_dynamic_state_of_synapse_in_tissue,
         typename type_of_unpacked_dynamic_state_of_signaling_data>
void single_threaded_transition_function_of_packed_dynamic_state_of_cell(
        std::function<
            void(
            type_of_unpacked_dynamic_state_of_cell_in_tissue& dynamic_state_of_cell_to_be_updated,
            static_state_of_cell const& static_state_of_the_updated_cell,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<type_of_unpacked_dynamic_state_of_synapse_in_tissue(natural_32_bit)>
                // NOTE: Valid indices are [0,num_of_synapses_connected_to_the_cell - 1].
                get_synapse_connected_to_the_cell_at_given_index,
            static_state_of_synapse const& static_state_of_synapses_connected_to_the_cell,
            std::function<type_of_unpacked_dynamic_state_of_signaling_data(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated cell, see 'static_state_of_the_updated_cell' data.
                // NOTE: Coordinates (0,0,0) reference the signalling data of the updated cell.
                get_signalling_data_in_neighbourhood_of_the_updated_cell_at_given_coordiates,
            static_state_of_signalling const& static_state_of_signalling_data
            )> single_threaded_transition_function_of_unpacked_dynamic_state_cell,
        bits_reference bits_of_cell_to_be_updated,
        static_state_of_cell const& static_state_of_the_updated_cell,
        natural_32_bit num_of_synapses_connected_to_the_cell,
        std::function<bits_const_reference(natural_32_bit)>
            get_bits_of_synapse_connected_to_the_cell_at_given_index,
        static_state_of_synapse const& static_state_of_synapses_connected_to_the_cell,
        std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
            get_bits_of_signalling_data_in_neighbourhood_of_the_updated_cell_at_given_coordiates,
        static_state_of_signalling const& static_state_of_signalling_data
        )
{
    type_of_unpacked_dynamic_state_of_cell_in_tissue
            dynamic_state_of_cell_to_be_updated(bits_of_cell_to_be_updated);
    single_threaded_transition_function_of_unpacked_dynamic_state_cell(
                dynamic_state_of_cell_to_be_updated,
                static_state_of_the_updated_cell,
                num_of_synapses_connected_to_the_cell,
                std::bind(&private_internal_implementation_details::get_instance_at_index<
                                type_of_unpacked_dynamic_state_of_synapse_in_tissue>,
                          get_bits_of_synapse_connected_to_the_cell_at_given_index,
                          std::placeholders::_1),
                static_state_of_synapses_connected_to_the_cell,
                std::bind(&private_internal_implementation_details::get_instance_at_coordinates<
                                type_of_unpacked_dynamic_state_of_signaling_data>,
                          get_bits_of_signalling_data_in_neighbourhood_of_the_updated_cell_at_given_coordiates,
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                static_state_of_signalling_data
                );
    dynamic_state_of_cell_to_be_updated >> bits_of_cell_to_be_updated;
}


}

#endif
