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
    return  "This program tests cellconnect's algorithm for filling in"
            "coordinates of source cell of synapses in a neural tissue.";
}
