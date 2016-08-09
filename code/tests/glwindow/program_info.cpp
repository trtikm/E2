#include "./program_info.hpp"

std::string  get_program_name()
{
    return "glwindow";
}

std::string  get_program_version()
{
    return "1.00";
}

std::string  get_program_description()
{
    return "It opens an OpenGL window and its simulator renders an object\n"
           "and a grid. The object consists of a rectangle (two triangles)\n"
           "filled in by interpolated vertex colours and a texture colours.\n"
           "Both types of colours are mixed together. The object also uses\n"
           "an alpha chanel common to all vertices (via an unifom variable\n"
           "in the vertex shader). The object uses one of two textures which\n"
           "are swapped every 5 seconds. The grid is just a set of few line with\n"
           "colour. In each simulation step everything is created (shaders,\n"
           "buffer, textures), then everything is rendered, and finally\n"
           "everything is destroyed. The simulator only hold camera-related\n"
           "data between individual steps in order to provide free-fly camera\n"
           "movement. Now follow controls of the camera: LMOUSE - move forward,\n"
           "RMOUSE - move backward, LMOUSE + RMOUSE - move in the projection plane,\n"
           "MMOUSE - rotate."
           ;
}
