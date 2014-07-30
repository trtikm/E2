#ifndef ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED
#   define ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED

#   include <efloop/external_feedback_loop.hpp>
#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <memory>

namespace envlab {


struct rules_and_logic_of_environment
{
    virtual ~rules_and_logic_of_environment() {}
    virtual void compute_next_state_of_all_sensory_cells_of_neural_tissue(
        std::shared_ptr<efloop::read_write_access_to_sensory_cells const> read_write_access_to_sensory_cells,
        std::shared_ptr<efloop::read_access_to_synapses_to_muscles const> read_access_to_synapses_to_muscles,
        unsigned int max_number_of_threads_you_can_create_and_run_simultaneously_during_the_computation
        ) = 0;
};


}

#endif
