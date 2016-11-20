#include <qtgl/camera_utils.hpp>
#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace qtgl {


void  cursor_line_begin(camera_perspective const&  camera, vector2 const&  mouse_pos, window_props const&  props, vector3&  output)
{
    vector2 const  u(
        camera.left() + (mouse_pos(0) / (float_32_bit)props.width_in_pixels()) * window_width_in_meters(props),
        camera.top() - (mouse_pos(1) / (float_32_bit)props.height_in_pixels()) * window_height_in_meters(props)
        );
    angeo::coordinate_system const&  coord_system = *camera.coordinate_system();
    output = coord_system.origin() +
             u(0) * axis_x(coord_system) +
             u(1) * axis_y(coord_system) -
             camera.near_plane() * axis_z(coord_system);
}


void  cursor_line_end(camera_perspective const&  camera, vector3 const&  cursor_line_begin, vector3&  output)
{
    vector3 const  far_plane_normal = axis_z(*camera.coordinate_system());
    vector3 const  far_plane_origin = camera.coordinate_system()->origin() + camera.far_plane() * far_plane_normal;
    bool const  result = angeo::collision_ray_and_plane(
            cursor_line_begin,
            normalised(cursor_line_begin - camera.coordinate_system()->origin()),
            far_plane_origin,
            far_plane_normal,
            nullptr,
            nullptr,
            nullptr,
            &output
            );
    INVARIANT(result == true);
}


void  compute_clip_planes(camera_perspective const&  camera, std::vector< std::pair<vector3,vector3> >&  output_planes)
{
    vector3 const& S = camera.coordinate_system()->origin();
    vector3 const  X = angeo::axis_x(*camera.coordinate_system());
    vector3 const  Y = angeo::axis_y(*camera.coordinate_system());
    vector3 const  Z = angeo::axis_z(*camera.coordinate_system());

    output_planes.push_back({ S - camera.near_plane() * Z, -Z });
    output_planes.push_back({ S - camera.far_plane() * Z, Z });
    output_planes.push_back({ S, normalised(camera.near_plane() * X + camera.left() * Z) });
    output_planes.push_back({ S, normalised(-camera.near_plane() * X - camera.right() * Z) });
    output_planes.push_back({ S, normalised(camera.near_plane() * Y + camera.bottom() * Z) });
    output_planes.push_back({ S, normalised(-camera.near_plane() * Y - camera.top() * Z) });
}


}
