#ifndef E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_SYNAPSE_HPP_INCLUDED
#   define E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_SYNAPSE_HPP_INCLUDED

#   include <utility/bits_reference.hpp>


struct my_synapse
{
    my_synapse(bits_const_reference const& bits);
    void operator>>(bits_reference const& bits);
};


#endif
