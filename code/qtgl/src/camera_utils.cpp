#include <qtgl/camera_utils.hpp>

namespace qtgl {


void  cursor_line_begin(camera_perspective const&  camera, vector2 const&  mouse_pos, window_props const&  props, vector3&  output)
{
    vector2 const  u(
        camera.left() + (mouse_pos(0) / (float_32_bit)props.width_in_pixels()) * window_width_in_meters(props),
        camera.top() - (mouse_pos(1) / (float_32_bit)props.height_in_pixels()) * window_height_in_meters(props)
        );
    angeo::coordinate_system const&  coord_system = *camera.coordinate_system();
    output = u(0) * axis_x(coord_system) + u(1) * axis_y(coord_system) - camera.near_plane() * axis_z(coord_system);
}


void  cursor_line_end(camera_perspective const&  camera, vector3 const&  cursor_line_begin, vector3& output)
{
    vector3 const  far_plane_normal = axis_z(*camera.coordinate_system());
    vector3 const  far_plane_origin = camera.coordinate_system()->origin() + camera.far_plane() * far_plane_normal;
}


}
