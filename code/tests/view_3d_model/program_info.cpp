#include "./program_info.hpp"

std::string  get_program_name()
{
    return "view_3d_model";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "It allows to load 3d model together with textures and shaders\n"
           "so that we can see it and its properties. TODO: it should also\n"
           "provide browding and search in our model database."
           ;
}
