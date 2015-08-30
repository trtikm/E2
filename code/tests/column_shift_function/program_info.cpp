#include "./program_info.hpp"

std::string  get_program_name()
{
    return "column_shift_function";
}

std::string  get_program_version()
{
    return "0.01";
}

std::string  get_program_description()
{
    return "This program tests correctness of the module column_shift_function\n"
           "of the library cellconnect. A shift function is constructed for 5\n"
           "tissue xy-dimensions, 4 largest template dimensions, 4 template repetitions,\n"
           "and 2 layouts of templates. The first 'small' layout is a 3x3 matrix\n"
           "where all templates have the same dimensions. The second 'big' layout\n"
           "is 4x4 matrix, where four corner templates have the largest dimensions"
           "and remaining templates are half width, height, or both dimensions. So,\n"
           "for each of 5 tissues there is constructed 4*4*2=32 shift functions.\n"
           "For each shift function constructed for a tissue we check whether it\n"
           "is a BIJECTIVE function on all valid coordinates of the tissue. Note that\n"
           "this program tests only shift functions constructed by the non-default\n"
           "constructor, i.e. functions which are not the identity. It is not necessary\n"
           "to test the idenity functions, because the code is straightforward and so\n"
           "obviously correct.";
}
