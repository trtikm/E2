#ifndef GFX_CAMERA_HPP_INCLUDED
#   define GFX_CAMERA_HPP_INCLUDED

#   include <angeo/coordinate_system.hpp>
#   include <gfx/viewport.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <angeo/tensor_math.hpp>

namespace gfx {


struct camera
{
    camera(angeo::coordinate_system const&  coordinate_system);
    camera(angeo::coordinate_system_ptr  coordinate_system);
    virtual ~camera() {}

    angeo::coordinate_system_const_ptr  coordinate_system() const { return m_coordinate_system; }
    angeo::coordinate_system_ptr  coordinate_system() { return m_coordinate_system; }

    void  set_coordinate_system(angeo::coordinate_system const&   coord_system);
    void  set_coordinate_system(angeo::coordinate_system_ptr const   coord_system);

    void  to_camera_space_matrix(matrix44&  output) const { to_base_matrix(*this->coordinate_system(), output); }
    virtual void  projection_matrix(matrix44&  output) const = 0;

private:
    angeo::coordinate_system_ptr  m_coordinate_system;
};


typedef std::shared_ptr<camera>  camera_ptr;
typedef std::shared_ptr<camera const>  camera_const_ptr;


struct camera_perspective;
typedef std::shared_ptr<camera_perspective>  camera_perspective_ptr;
typedef std::shared_ptr<camera_perspective const>  camera_perspective_const_ptr;


struct camera_perspective : public camera
{
    static camera_perspective_ptr  create(
            angeo::coordinate_system const&  coordinate_system,
            float_32_bit const  near,
            float_32_bit const  far,
            viewport const&  window_info
            );
    static camera_perspective_ptr  create(
            angeo::coordinate_system_ptr  coordinate_system,
            float_32_bit const  near,
            float_32_bit const  far,
            viewport const&  window_info
            );
    static camera_perspective_ptr  create(
            angeo::coordinate_system_ptr  coordinate_system,
            float_32_bit const  near,
            float_32_bit const  far,
            float_32_bit const  left,
            float_32_bit const  right,
            float_32_bit const  bottom,
            float_32_bit const  top
            );

    camera_perspective(angeo::coordinate_system const&  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       viewport const&  window_info
                       );
    camera_perspective(angeo::coordinate_system_ptr  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       viewport const&  window_info
                       );
    camera_perspective(angeo::coordinate_system_ptr  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       float_32_bit const  left,
                       float_32_bit const  right,
                       float_32_bit const  bottom,
                       float_32_bit const  top
                       );

    float_32_bit  near_plane() const { return m_near; }
    float_32_bit  far_plane() const { return m_far; }

    float_32_bit  left() const { return m_left; }
    float_32_bit  right() const { return m_right; }

    float_32_bit  bottom() const { return m_bottom; }
    float_32_bit  top() const { return m_top; }

    void  set_near_plane(float_32_bit const  near);
    void  set_far_plane(float_32_bit const  far);

    void  set_left(float_32_bit const  left);
    void  set_right(float_32_bit const  right);

    void  set_bottom(float_32_bit const  bottom);
    void  set_top(float_32_bit const  top);

    void  projection_matrix(matrix44&  output) const;

    void  projection_matrix_perspective(matrix44&  output) const;
    void  projection_matrix_orthogonal(matrix44&  output) const;
    void  projection_matrix_orthogonal_2d(matrix44&  output) const;

    // If the passed point is within the frustum, then the pixel coords are computed and returns true. Otherwise returns false.
    bool  pixel_coordinates_in_01_of_point_in_camera_space(vector3 const&  point, vector2&  pixel) const;

    void  ray_points_in_camera_space(vector2 const&  pixel_coords_in_01, vector3&  ray_begin, vector3&  ray_end) const;

    // Each pair represents a origin and normal (not mornalised; pointing into the frustum) of a clip plane.
    // Order of planes in the array: front, back, left, right, bottom, top.
    void  clip_planes_in_camera_space(std::array<std::pair<vector3, vector3>, 6U>&  clip_planes) const;

private:

    float_32_bit  m_near;
    float_32_bit  m_far;
    float_32_bit  m_left;
    float_32_bit  m_right;
    float_32_bit  m_bottom;
    float_32_bit  m_top;
};


void  adjust(camera_perspective&  camera_ref, viewport const&  window_info);


void  projection_matrix_perspective(
        float_32_bit const  near_,
        float_32_bit const  far_,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        );
void  projection_matrix_orthogonal(
        float_32_bit const  near_,
        float_32_bit const  far_,
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        );

void  projection_matrix_orthogonal_2d(
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top,
        matrix44&  output
        );


bool  is_point_in_camera_space_inside_camera_frustum(camera_perspective const&  cam, vector3 const&  point_in_camera_space);


}

#endif
