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
        "A command-line tool which accepts 2 PNG images with RGBA channels.\n"
        "However, A-channels are ignored in both images, even if they are\n"
        "present. Both images must have equal with and height in pixels.\n"
        "RGB channels of the resulting PNG image are equal to RGB channels\n"
        "of the first image, and the A channel is equal to mean of RGB\n"
        "channels of the second image. Of course, the width and height\n"
        "of the resulting image are equal to those of the input images."
        ;        
}
