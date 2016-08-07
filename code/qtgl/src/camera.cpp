#include <qtgl/camera.hpp>
#include <utility/assumptions.hpp>

namespace qtgl {


camera::camera(qtgl::coordinate_system const&  coordinate_system)
    : m_coordinate_system(new qtgl::coordinate_system(coordinate_system))
{
    ASSUMPTION(m_coordinate_system.operator bool());
}


camera::camera(coordinate_system_ptr  coordinate_system)
    : m_coordinate_system(coordinate_system)
{
    ASSUMPTION(m_coordinate_system.operator bool());
}


void  camera::set_coordinate_system(qtgl::coordinate_system const&   coord_system)
{
    m_coordinate_system = coordinate_system_ptr{ new qtgl::coordinate_system(coord_system) };
}


void  camera::set_coordinate_system(coordinate_system_ptr const   coord_system)
{
    ASSUMPTION(coord_system.operator bool());
    m_coordinate_system = coord_system;
}


void  view_projection_matrix(camera const&  camera_ref, matrix44&  output)
{
    matrix44  view_matrix;
    inverted_transformation_matrix(*camera_ref.coordinate_system(),view_matrix);
    matrix44  projection_matrix;
    camera_ref.projection_matrix(projection_matrix);
    output = projection_matrix * view_matrix;
}


camera_perspective_ptr  camera_perspective::create(
        qtgl::coordinate_system const&  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        window_props const&  window_info
        )
{
    return create(coordinate_system_ptr{new qtgl::coordinate_system{coordinate_system}},near,far,window_info);
}


camera_perspective_ptr  camera_perspective::create(
        coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        window_props const&  window_info
        )
{
    return camera_perspective_ptr{ new camera_perspective{coordinate_system,near,far,window_info} };
}


camera_perspective::camera_perspective(
        coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
    : camera(coordinate_system)
    , m_near(near)
    , m_far(far)
    , m_left(left)
    , m_right(right)
    , m_bottom(bottom)
    , m_top(top)
{
    ASSUMPTION(0.0f < m_near && m_near <= m_far);
    ASSUMPTION(m_left <= m_right);
    ASSUMPTION(m_bottom <= m_top);
}


camera_perspective::camera_perspective(
        qtgl::coordinate_system const&  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
    : camera_perspective(
          coordinate_system_ptr{new qtgl::coordinate_system{coordinate_system}},
          near,far,left,right,bottom,top
          )
{}


camera_perspective::camera_perspective(coordinate_system_ptr  coordinate_system,
                                       float_32_bit const  near,
                                       float_32_bit const  far,
                                       window_props const&  window_info)
    : camera(coordinate_system)
    , m_near(near)
    , m_far(far)
    , m_left(-window_width_in_meters(window_info) / 2.0f)
    , m_right(window_width_in_meters(window_info) / 2.0f)
    , m_bottom(-window_height_in_meters(window_info) / 2.0f)
    , m_top(window_height_in_meters(window_info) / 2.0f)
{
    ASSUMPTION(0.0f < m_near && m_near <= m_far);
    ASSUMPTION(m_left <= m_right);
    ASSUMPTION(m_bottom <= m_top);
}


camera_perspective::camera_perspective(qtgl::coordinate_system const&  coordinate_system,
                                       float_32_bit const  near,
                                       float_32_bit const  far,
                                       window_props const&  window_info)
    : camera_perspective(
          coordinate_system_ptr{new qtgl::coordinate_system{coordinate_system}},
          near,far,window_info
          )
{}


void  camera_perspective::set_near_plane(float_32_bit const  near)
{
    ASSUMPTION(0.0f < near && near <= m_far);
    m_near = near;
}

void  camera_perspective::set_far_plane(float_32_bit const  far)
{
    ASSUMPTION(0.0f < m_near && m_near <= far);
    m_far = far;
}

void  camera_perspective::set_left(float_32_bit const  left)
{
    ASSUMPTION(left <= m_right);
    m_left = left;
}

void  camera_perspective::set_right(float_32_bit const  right)
{
    ASSUMPTION(m_left <= right);
    m_right = right;
}

void  camera_perspective::set_bottom(float_32_bit const  bottom)
{
    ASSUMPTION(bottom <= m_top);
    m_bottom = bottom;
}

void  camera_perspective::set_top(float_32_bit const  top)
{
    ASSUMPTION(m_bottom <= top);
    m_top = top;
}


void  camera_perspective::projection_matrix(matrix44&  output) const
{
    output <<
        (2.0f * m_near) / (m_right - m_left), 0.0f, (m_right + m_left) / (m_right - m_left), 0.0f,
        0.0f, 2.0f * m_near / (m_top - m_bottom), (m_top + m_bottom) / (m_top - m_bottom), 0.0f,
        0.0f, 0.0f, -(m_far - m_near) / (m_far - m_near), -(2.0f * m_far * m_near) / (m_far - m_near),
        0.0f, 0.0f, -1.0f, 0.0f
        ;
}

vector3  camera_perspective::cursor3d(vector2 const&  mouse_pos, window_props const&  props) const
{
    vector2 const  u(
        m_left + (mouse_pos(0) / (float_32_bit)props.width_in_pixels()) * window_width_in_meters(props),
        m_top - (mouse_pos(1) / (float_32_bit)props.height_in_pixels()) * window_height_in_meters(props)
        );
    return u(0) * axis_x(*coordinate_system()) + u(1) * axis_y(*coordinate_system()) - m_near * axis_z(*coordinate_system());
}


camera_perspective_ptr  create_camera_perspective(
        coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        window_props const&  window_info
        )
{
    return camera_perspective_ptr( new qtgl::camera_perspective(coordinate_system,near,far,window_info) );
}

void  adjust(camera_perspective&  camera_ref, window_props const&  window_info)
{
    camera_ref.set_left(-window_width_in_meters(window_info) / 2.0f);
    camera_ref.set_right(window_width_in_meters(window_info) / 2.0f);
    camera_ref.set_bottom(-window_height_in_meters(window_info) / 2.0f);
    camera_ref.set_top(window_height_in_meters(window_info) / 2.0f);
}


}
