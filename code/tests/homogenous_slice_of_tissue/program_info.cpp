#include "./program_info.hpp"

std::string  get_program_name()
{
    return "homogenous_slice_of_tissue";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of 'homogenous_slice_of_tissue'\n"
           "which serves as a memory holder for units of dynamic_state_of_neural_tissue\n"
           "in a 3D array. This test checks that a accessess for given 3D coordinates\n"
           "access the right bits of the allocated memory. The testing is performed\n"
           "for different numbers of units along each dimension and also for\n"
           "different number of bits per unit.";
}
