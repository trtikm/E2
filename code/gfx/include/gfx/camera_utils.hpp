#ifndef GFX_CAMERA_UTILS_HPP_INCLUDED
#   define GFX_CAMERA_UTILS_HPP_INCLUDED

#   include <gfx/camera.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <tuple>

namespace gfx {


//void  cursor_line_begin(camera_perspective const&  camera, vector2 const&  mouse_pos, window_props const&  props, vector3&  output);
void  cursor_line_end(camera_perspective const&  camera, vector3 const&  cursor_line_begin, vector3&  output);


void  compute_clip_planes(camera_perspective const&  camera, std::vector< std::pair<vector3,vector3> >&  output_planes);

void  look_at(angeo::coordinate_system&  coordinate_system, vector3 const&  target, float_32_bit const  distance);


}

#endif
