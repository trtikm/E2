#include "./program_info.hpp"

std::string  get_program_name()
{
    return "instance_wrapper";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of the utility called instance_wrapper\n"
           "which provides repetitive explicit construction and destruction of\n"
           "a wrapped instance. The test checks for correctness of constructions\n"
           "and destructions and it also checks for direct accessed to the wrapped\n"
           "instance via overloaded operators '->'.";
}
