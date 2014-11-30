#ifndef ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED
#   define ENVLAB_RULES_AND_LOGIC_OF_ENVIRONMENT_HPP_INCLUDED

#   include <efloop/access_to_sensory_cells.hpp>
#   include <efloop/access_to_synapses_to_muscles.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace envlab {


struct rules_and_logic_of_environment
{
    virtual ~rules_and_logic_of_environment() {}
    virtual void compute_next_state(
            efloop::access_to_sensory_cells&  access_to_sensory_cells,
            efloop::access_to_synapses_to_muscles const&  access_to_synapses_to_muscles,
            natural_32_bit const  num_threads_avalilable_for_computation
            ) = 0;
};


}

#endif
