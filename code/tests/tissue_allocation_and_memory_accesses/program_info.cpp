#include "./program_info.hpp"

std::string  get_program_name()
{
    return "tissue_allocation_and_memory_accesses";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of memory allocation of a neural tissues.\n"
           "Moreover, it also tests correctness of read/write accesses into the allocated\n"
           "memory.";
}
