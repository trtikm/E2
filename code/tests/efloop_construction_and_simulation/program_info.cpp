#include "./program_info.hpp"

std::string  get_program_name()
{
    return "efloop_construction_and_simulation";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of creation of an external feedback loop\n"
           "between a neural tissues and environment. Simulation of the loop is also\n"
           "tested.";
}
