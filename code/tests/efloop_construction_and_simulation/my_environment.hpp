#ifndef E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_ENVIRONMENT_HPP_INCLUDED
#   define E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_ENVIRONMENT_HPP_INCLUDED

#   include <envlab/rules_and_logic_of_environment.hpp>
#   include <utility/basic_numeric_types.hpp>


struct my_environment : public envlab::rules_and_logic_of_environment
{
    void compute_next_state(
            std::vector<efloop::access_to_sensory_cells>&  accesses_to_sensory_cells,
            std::vector<efloop::access_to_synapses_to_muscles>&  accesses_to_synapses_to_muscles,
            natural_32_bit const  num_threads_avalilable_for_computation
            );
};


#endif
