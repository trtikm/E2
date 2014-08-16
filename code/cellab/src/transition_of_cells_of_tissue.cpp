#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>
#include <vector>

#include <utility/development.hpp>

namespace cellab {


void apply_transition_of_cells_of_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_cell_to_be_updated,
            static_state_of_cell const& static_state_of_the_updated_cell,
            natural_32_bit num_of_synapses_connected_to_the_cell,
            std::function<bits_const_reference(natural_32_bit)>
                // NOTE: Valid indices are [0,num_of_synapses_connected_to_the_cell - 1].
                get_bits_of_synapse_connected_to_the_cell_at_given_index,
            static_state_of_synapse const& static_state_of_synapses_connected_to_the_cell,
            std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated cell, see 'static_state_of_the_updated_cell' data.
                // NOTE: Coordinates (0,0,0) reference the signalling data of the updated cell.
                get_bits_of_signalling_data_in_neighbourhood_of_the_updated_cell_at_given_coordiates,
            static_state_of_signalling const& static_state_of_signalling_data
            )> single_threaded_transition_function_of_packed_dynamic_state_of_cell,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    NOT_IMPLEMENTED_YET();
}


}
