#include "./program_info.hpp"

std::string  get_program_name()
{
    return "unsigned_integer_wrap_operations";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of algorithms 'checked_add_*_bit' and\n"
           "'checked_mul_*_bit' which are supposed to detect wraps of operations of\n"
           "unsigned integers.";
}
