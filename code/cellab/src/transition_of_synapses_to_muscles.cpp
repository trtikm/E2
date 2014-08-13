#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <memory>

#include <utility/development.hpp>

namespace cellab {


void apply_transition_of_synapses_to_muscles(
        std::shared_ptr<dynamic_state_of_neural_tissue const> const dynamic_state_of_tissue,
        std::function<
            void(
            bits_reference bits_of_synapse_to_be_updated,
            static_state_of_synapse const& static_state_of_updated_synapse,
            bits_const_reference bits_of_source_cell,
            static_state_of_cell const& static_state_of_source_cell,
            )> single_threaded_transition_function_of_synapse_to_muscle,
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    NOT_IMPLEMENTED_YET();
}



}
