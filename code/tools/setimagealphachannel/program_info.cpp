#include <setimagealphachannel/program_info.hpp>

std::string  get_program_name()
{
    return "setimagealphachannel";
}

std::string  get_program_version()
{
    return "1.0";
}

std::string  get_program_description()
{
    return
        "A command-line tool which accepts an input PNG image and each\n"
        "its pixel with RGB(A) colour (255, 0, 255, *) will be replaced\n"
        " by RGBA (0, 0, 0, 0)."
        ;        
}
