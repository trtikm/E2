#include "./program_info.hpp"

std::string  get_program_name()
{
    return "random";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests required properties of sequences of random numbers.\n"
           "There is a test which checks whether a sequence of random numbers split\n"
           "into n classes modulo 'index of a number in the sequence' produces n\n"
           "smaller sequnces s.t. each gives exactly the same probabilities of\n"
           "appearence of individual numbers as in the original big sequence.\n"
           ;
}
