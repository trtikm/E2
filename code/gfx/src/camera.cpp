#include <gfx/camera.hpp>
#include <utility/assumptions.hpp>

namespace gfx { namespace detail {


float_32_bit  window_width_in_meters(osi::window_props const&  window_info)
{
    return 0.001f * window_info.pixel_width_mm() * window_info.window_width();
}


float_32_bit  window_height_in_meters(osi::window_props const&  window_info)
{
    return 0.001f * window_info.pixel_height_mm() * window_info.window_height();
}


}}

namespace gfx {


camera::camera(angeo::coordinate_system const&  coordinate_system)
    : m_coordinate_system(new angeo::coordinate_system(coordinate_system))
{
    ASSUMPTION(m_coordinate_system.operator bool());
}


camera::camera(angeo::coordinate_system_ptr  coordinate_system)
    : m_coordinate_system(coordinate_system)
{
    ASSUMPTION(m_coordinate_system.operator bool());
}


void  camera::set_coordinate_system(angeo::coordinate_system const&   coord_system)
{
    m_coordinate_system = std::make_shared<angeo::coordinate_system>(coord_system);
}


void  camera::set_coordinate_system(angeo::coordinate_system_ptr const   coord_system)
{
    ASSUMPTION(coord_system.operator bool());
    m_coordinate_system = coord_system;
}


camera_perspective_ptr  camera_perspective::create(
        angeo::coordinate_system const&  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        osi::window_props const&  window_info
        )
{
    return create(std::make_shared<angeo::coordinate_system>(coordinate_system),near,far,window_info);
}


camera_perspective_ptr  camera_perspective::create(
        angeo::coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        osi::window_props const&  window_info
        )
{
    return std::make_shared<camera_perspective>(coordinate_system,near,far,window_info);
}


camera_perspective_ptr  camera_perspective::create(
        angeo::coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
{
    return std::make_shared<camera_perspective>(coordinate_system, near, far, left, right, bottom, top);
}


camera_perspective::camera_perspective(
        angeo::coordinate_system_ptr  coordinate_system,
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
        angeo::coordinate_system const&  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
    : camera_perspective(
          std::make_shared<angeo::coordinate_system>(coordinate_system),
          near,far,left,right,bottom,top
          )
{}


camera_perspective::camera_perspective(angeo::coordinate_system_ptr  coordinate_system,
                                       float_32_bit const  near,
                                       float_32_bit const  far,
                                       osi::window_props const&  window_info)
    : camera(coordinate_system)
    , m_near(near)
    , m_far(far)
    , m_left(-detail::window_width_in_meters(window_info) / 2.0f)
    , m_right(detail::window_width_in_meters(window_info) / 2.0f)
    , m_bottom(-detail::window_height_in_meters(window_info) / 2.0f)
    , m_top(detail::window_height_in_meters(window_info) / 2.0f)
{
    ASSUMPTION(0.0f < m_near && m_near <= m_far);
    ASSUMPTION(m_left <= m_right);
    ASSUMPTION(m_bottom <= m_top);
}


camera_perspective::camera_perspective(angeo::coordinate_system const&  coordinate_system,
                                       float_32_bit const  near,
                                       float_32_bit const  far,
                                       osi::window_props const&  window_info)
    : camera_perspective(
          std::make_shared<angeo::coordinate_system>(coordinate_system),
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
    projection_matrix_perspective(output);
}


void  camera_perspective::projection_matrix_perspective(matrix44&  output) const
{
    gfx::projection_matrix_perspective(m_near, m_far, m_left, m_right, m_bottom, m_top, output);
}


void  camera_perspective::projection_matrix_orthogonal(matrix44&  output) const
{
    gfx::projection_matrix_orthogonal(m_near, m_far, m_left, m_right, m_bottom, m_top, output);
}


void  camera_perspective::projection_matrix_orthogonal_2d(matrix44&  output) const
{
    gfx::projection_matrix_orthogonal_2d(m_left, m_right, m_bottom, m_top, output);
}


bool  camera_perspective::pixel_coordinates_in_01_of_point_in_camera_space(vector3 const&  point, vector2&  pixel) const
{
    if (point(2) > -m_near || point(2) < -m_far)
        return false;
    pixel = { ((point(0) / -point(2)) * m_near - m_left) / (m_right - m_left),
              ((point(1) / -point(2)) * m_near - m_bottom) / (m_top - m_bottom) };
    return pixel(0) >= 0.0f && pixel(0) <= 1.0f && pixel(1) >= 0.0f && pixel(1) <= 1.0f;
}


void  camera_perspective::ray_points_in_camera_space(vector2 const&  pixel_coords_in_01, vector3&  ray_begin, vector3&  ray_end) const
{
    ASSUMPTION(
        pixel_coords_in_01(0) >= 0.0f && pixel_coords_in_01(0) <= 1.0f &&
        pixel_coords_in_01(1) >= 0.0f && pixel_coords_in_01(1) <= 1.0f &&
        m_near > 0.001f
        );
    ray_begin = { m_left + pixel_coords_in_01(0) * (m_right - m_left), m_bottom + pixel_coords_in_01(1) * (m_top - m_bottom), -m_near };
    ray_end = (m_far / m_near) * ray_begin;
}


void  camera_perspective::clip_planes_in_camera_space(std::array<std::pair<vector3, vector3>, 6U>&  clip_planes) const
{
    // Front and back
    clip_planes.at(0) = { { 0.0f, 0.0f, -m_near }, -vector3_unit_z() };
    clip_planes.at(1) = { { 0.0f, 0.0f,  -m_far }, vector3_unit_z() };

    // Left and right
    clip_planes.at(2) = { { -m_left, 0.0f, -m_near }, {  m_near, 0.0f, -m_left } };
    clip_planes.at(3) = { { m_right, 0.0f, -m_near }, { -m_near, 0.0f, -m_right } };

    // Bottom and top
    clip_planes.at(4) = { { 0.0f, -m_bottom, -m_near }, { 0.0f, m_near, -m_bottom } };
    clip_planes.at(5) = { { 0.0f,     m_top, -m_near }, { 0.0f, -m_near, -m_top } };
}


camera_perspective_ptr  create_camera_perspective(
        angeo::coordinate_system_ptr  coordinate_system,
        float_32_bit const  near,
        float_32_bit const  far,
        osi::window_props const&  window_info
        )
{
    return std::make_shared<camera_perspective>(coordinate_system,near,far,window_info);
}

void  adjust(camera_perspective&  camera_ref, osi::window_props const&  window_info)
{
    camera_ref.set_left(-detail::window_width_in_meters(window_info) / 2.0f);
    camera_ref.set_right(detail::window_width_in_meters(window_info) / 2.0f);
    camera_ref.set_bottom(-detail::window_height_in_meters(window_info) / 2.0f);
    camera_ref.set_top(detail::window_height_in_meters(window_info) / 2.0f);
}


void  projection_matrix_perspective(
        float_32_bit const  near_,
        float_32_bit const  far_,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        )
{
    output(0, 0) = (2.0f * near_) / (right - left);
    output(0, 1) = 0.0f;
    output(0, 2) = (right + left) / (right - left);
    output(0, 3) = 0.0f;

    output(1, 0) = 0.0f;
    output(1, 1) = 2.0f * near_ / (top - bottom);
    output(1, 2) = (top + bottom) / (top - bottom);
    output(1, 3) = 0.0f;

    output(2, 0) = 0.0f;
    output(2, 1) = 0.0f;
    output(2, 2) = -(far_ + near_) / (far_ - near_);
    output(2, 3) = -(2.0f * far_ * near_) / (far_ - near_);

    output(3, 0) = 0.0f;
    output(3, 1) = 0.0f;
    output(3, 2) = -1.0f;
    output(3, 3) = 0.0f;
}


void  projection_matrix_orthogonal(
        float_32_bit const  near_,
        float_32_bit const  far_,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        )
{
    output(0, 0) = 2.0f / (right - left);
    output(0, 1) = 0.0f;
    output(0, 2) = 0.0f;
    output(0, 3) = -(right + left) / (right - left);

    output(1, 0) = 0.0f;
    output(1, 1) = 2.0f / (top - bottom);
    output(1, 2) = 0.0f;
    output(1, 3) = -(top + bottom) / (top - bottom);

    output(2, 0) = 0.0f;
    output(2, 1) = 0.0f;
    output(2, 2) = -2.0f / (far_ - near_);
    output(2, 3) = -(far_ + near_) / (far_ - near_);

    output(3, 0) = 0.0f;
    output(3, 1) = 0.0f;
    output(3, 2) = 0.0f;
    output(3, 3) = 1.0f;
}


void  projection_matrix_orthogonal_2d(
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        )
{
    output(0, 0) = 2.0f / (right - left);
    output(0, 1) = 0.0f;
    output(0, 2) = 0.0f;
    output(0, 3) = -(right + left) / (right - left);

    output(1, 0) = 0.0f;
    output(1, 1) = 2.0f / (top - bottom);
    output(1, 2) = 0.0f;
    output(1, 3) = -(top + bottom) / (top - bottom);

    output(2, 0) = 0.0f;
    output(2, 1) = 0.0f;
    output(2, 2) = 0.0f;
    output(2, 3) = 0.0f;

    output(3, 0) = 0.0f;
    output(3, 1) = 0.0f;
    output(3, 2) = 0.0f;
    output(3, 3) = 1.0f;
}


}
