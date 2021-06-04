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
        "A command-line tool which accepts a gray-scale input PNG image\n"
        "and each its RGB(A) pixel (r, g, b, a) will be replaced\n"
        "by (255, 255, 255, r)."
        ;        
}
