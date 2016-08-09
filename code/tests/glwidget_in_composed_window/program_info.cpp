#include "./program_info.hpp"

std::string  get_program_name()
{
    return "glwidget_in_composed_window";
}

std::string  get_program_version()
{
    return "1.00";
}

std::string  get_program_description()
{
    return "It open a window with several Qt widgets. Namely, two edit boxes,\n"
           "two window splitters, a tabs widget, and one OpenGL embeded window.\n"
           "In each simulation step the test randomly chooses a clear colour,\n"
           "clears the current frame buffer of the OpenGL window, and swaps frame\n"
           "buffers of that window."
           ;
}
