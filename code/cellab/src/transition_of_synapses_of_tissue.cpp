#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <memory>

#include <utility/development.hpp>

namespace cellab {


void apply_transition_of_synapses_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_synapse_to_be_updated,
            static_state_of_synapse const& static_state_of_updated_synapse,
            bits_const_reference bits_of_source_cell,
            static_state_of_cell const& static_state_of_source_cell,
            bits_const_reference bits_of_target_cell_in_whose_territory_the_synapse_currently_appears,
            static_state_of_cell const& static_state_of_target_cell,
            std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated synapse, see 'static_state_of_updated_synapse' data.
                // NOTE: Coordinates (0,0,0) reference the signalling data in the territory of the target cell.
                get_bits_of_signalling_data_in_neighbourhood_of_the_synapse_at_given_coordiates,
            static_state_of_signalling const& static_state_of_signalling_data
            )> single_threaded_transition_function_of_synapse_inside_tissue,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    NOT_IMPLEMENTED_YET();
}


}
