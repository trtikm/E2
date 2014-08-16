#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/bits_reference.hpp>
#include <functional>
#include <memory>
#include <vector>

#include <utility/development.hpp>

namespace cellab {


void apply_transition_of_spaialy_local_intercellular_signalling_in_tissue(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_signalling_data_to_be_updated,
            static_state_of_signalling const& static_state_of_signalling_data,
            std::function<bits_const_reference(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated signalling data, see 'static_state_of_signalling_data' data.
                // NOTE: Coordinates (0,0,0) reference the cell of the updated signalling data.
                get_bits_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates,
            std::function<static_state_of_cell const&(natural_8_bit,natural_8_bit,natural_8_bit)>
                // NOTE: All coordinates range in [-N,N] where N is radius of local neighbourhood of the
                //       updated signalling data, see 'static_state_of_signalling_data' data.
                // NOTE: Coordinates (0,0,0) reference the static state of cell of the updated signalling data.
                get_static_state_of_cell_in_neighbourhood_of_updated_signalling_data_at_given_coordiates,
            )> single_threaded_transition_function_of_local_intercellular_signalling,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    NOT_IMPLEMENTED_YET();
}


}
