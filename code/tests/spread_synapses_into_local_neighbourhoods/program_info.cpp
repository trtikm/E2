#include "./program_info.hpp"

std::string  get_program_name()
{
    return "spread_synapses_into_local_neighbourhoods";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return  "This program tests cellconnect's algorithm for spreading synapses\n"
            "from territories of cells in single columns to territories of\n"
            "cells in their neighbourhoods. The test builds tissues of several\n"
            "xy-sizes (from 1x1 to 11x11) for 1,6,and 11 kinds of tissue\n"
            "cells and for 1,6, and 11 kinds of sensory cells. For each such\n"
            "tissue a synapses is columns are filled in using the algorithm\n"
            "'fill_src_coords_of_synapses'. Then the algorithm for spreading\n"
            "synapses is called for each pair of kinds of tissue cells with \n"
            "1,8,16 and 64 available threads according to a 'distribution matrix'.\n"
            "passed to the algorithm. Then for each column and for each pair of\n"
            "kinds of cells there is checked whether counts of synapses in all\n"
            "columns in the neighbourhood of the checked column equals to counts\n"
            "iside the distribution matrix."
            ;
}
