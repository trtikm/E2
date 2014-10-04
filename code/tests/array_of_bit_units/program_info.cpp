#include "./program_info.hpp"

std::string  get_program_name()
{
    return "array_of_bit_units";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of the utility called 'array_of_bit_units'\n"
           "which serves as a memory holder for dynamic_state_of_neural_tissue. This\n"
           "test checks that a sufficient memory is always allocated and accesses to\n"
           "the memory are also correct. The testing is performed for different numbers\n"
           "of allocated units and also for different sizes of units.";
}
