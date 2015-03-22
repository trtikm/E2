#ifndef ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED
#   define ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED

#   include <efloop/access_to_sensory_cells.hpp>
#   include <efloop/access_to_synapses_to_muscles.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace envlab {


struct rules_and_logic_of_environment
{
    virtual ~rules_and_logic_of_environment() {}
    virtual void compute_next_state(
            std::vector<efloop::access_to_sensory_cells>&  accesses_to_sensory_cells,
            std::vector<efloop::access_to_synapses_to_muscles>&  accesses_to_synapses_to_muscles,
            natural_32_bit const  num_threads_avalilable_for_computation
            ) = 0;
};


}

#endif
