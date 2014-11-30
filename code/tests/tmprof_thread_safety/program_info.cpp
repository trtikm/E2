#include "./program_info.hpp"

std::string  get_program_name()
{
    return "tmprof_thread_safety";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests thread safety of the time profiler tmprof.hpp/cpp.\n"
           "There are considered two (simple) recursive functions for time and\n"
           "hits-count measurement. They are first executed in a single thread\n"
           "several times. Then hit-counts and times are saved. Next the functions\n"
           "are executed again for the same number of times. But now execution are\n"
           "performed simultaneously, each from a separate theread. Then it is\n"
           "checked whether final number of executions on the functions are twice\n"
           "as big as those saved before. Also times have to be bigger.";
}
