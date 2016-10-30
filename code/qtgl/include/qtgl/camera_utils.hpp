#ifndef QTGL_CAMERA_UTILS_HPP_INCLUDED
#   define QTGL_CAMERA_UTILS_HPP_INCLUDED

#   include <qtgl/camera.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <angeo/tensor_math.hpp>

namespace qtgl {


void  cursor_line_begin(camera_perspective const&  camera, vector2 const&  mouse_pos, window_props const&  props, vector3&  output);
void  cursor_line_end(camera_perspective const&  camera, vector3 const&  cursor_line_begin, vector3&  output);


}

#endif
