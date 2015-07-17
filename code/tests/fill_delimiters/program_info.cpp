#include "./program_info.hpp"

std::string  get_program_name()
{
    return "fill_delimiters";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return  "This program tests cellconnect's algorithm for filling in\n"
            "delimiters of lists of synapses in territories of\n"
            "cells in the neural tissue. The test builds tissues of several\n"
            "xy-sizes (from 1x1 to 11x11) for 1,6,and 11 kinds of tissue\n"
            "cells and for 1,6, and 11 kinds of sensory cells. For each such\n"
            "tissue the algorithm is run with 1,8, and 16 available threads.\n"
            "Once delimiters are filled in, all delimites in whole the tissue\n"
            "are scanned and checked for their valid values. Note that we call\n"
            "the algorithm with the same tissue for different counts of threads.\n"
            "Therefore, the tissue (its delimiters) are cleared before we\n"
            "call the algorithm."
            ;
}
