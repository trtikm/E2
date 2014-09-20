#ifndef E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_CELL_HPP_INCLUDED
#   define E2_TEST_EFLOOP_CONSTRUCTION_AND_SIMULATION_MY_CELL_HPP_INCLUDED

#   include <utility/bits_reference.hpp>


struct my_cell
{
    my_cell(bits_const_reference const& bits);
    void operator>>(bits_reference const& bits);
};


#endif
