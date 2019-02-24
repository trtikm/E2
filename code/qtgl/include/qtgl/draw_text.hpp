#ifndef QTGL_DRAW_TEXT_HPP_INCLUDED
#   define QTGL_DRAW_TEXT_HPP_INCLUDED

#   include <qtgl/camera.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>

namespace qtgl {


void  set_font_directory(std::string const&  font_directory);


void  print_character(
        char const  character,
        matrix44 const&  size_and_position_matrix,
        matrix44 const&  from_camera_to_clipspace_matrix,
        vector4 const&  diffuse_colour
        );


// In the beginning of each simulation round is the viewport automatically reset to fit whole area of the GL window.
void  set_text_viewport(
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        );


inline void  set_text_viewport_from_camera(camera_perspective const&  cam)
{
    set_text_viewport(cam.left(), cam.right(), cam.bottom(), cam.top());
}


// Initiallised to white colour; change persists over subsequent simulation rounds (till changed again).
void  set_text_colour(vector4 const&  colour);


// Initiallised to default size (0.007f); the size is in meters; change persists over subsequent simulation rounds (till changed again).
void  set_text_size(float_32_bit const  size_in_meters);


// Both coordinates are in meters.
void  set_text_cursor(vector2 const&  position);


// Initiallised to false; change persists over subsequent simulation rounds (till changed again).
void  enable_depth_testing_for_text(bool const  state);


void  print_character(char const character, float_32_bit const  z_coord = 0.0f);
void  print_text(std::string const&  text, float_32_bit const  z_coord = 0.0f);


}

#endif
