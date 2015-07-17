#include "./program_info.hpp"

std::string  get_program_name()
{
    return "fill_src_coords_of_synapses";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return  "This program tests cellconnect's algorithm for filling in\n"
            "coordinates of source cell of synapses in territories of\n"
            "cells in the neural tissue. The test builds tissues of several\n"
            "xy-sizes (from 1x1 to 11x11) for 1,6,and 11 kinds of tissue\n"
            "cells and for 1,6, and 11 kinds of sensory cells. For each such\n"
            "tissue a 'filling-in' matrix is constructed. Then the algorithm\n"
            "is run for each tissue with the corresponding matrix with 1,8,\n"
            "and 16 available threads. Once coordinates of source cells\n"
            "are filled in, whole the tissue is scanned in order to infer a\n"
            "matrix which is supposed to be equal to one used in the algorithm.\n"
            "The test compares original and infered matrices. Note that we\n"
            "call the algorithm with the same tissue and matrix for different\n"
            "different counts of threads. Therefore, the tissue (its coordinates)\n"
            "are cleared before we call the algorithm."
            ;
}
