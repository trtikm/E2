#ifndef QTGL_CAMERA_HPP_INCLUDED
#   define QTGL_CAMERA_HPP_INCLUDED

#   include <qtgl/coordinate_system.hpp>
#   include <qtgl/window_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/tensor_math.hpp>

namespace qtgl {


struct camera
{
    camera(qtgl::coordinate_system const&  coordinate_system);
    camera(coordinate_system_ptr  coordinate_system);
    virtual ~camera() {}

    coordinate_system_const_ptr  coordinate_system() const { return m_coordinate_system; }
    coordinate_system_ptr  coordinate_system() { return m_coordinate_system; }

    void  set_coordinate_system(qtgl::coordinate_system const&   coord_system);
    void  set_coordinate_system(coordinate_system_ptr const   coord_system);

    virtual void  projection_matrix(matrix44&  output) const = 0;

private:
    coordinate_system_ptr  m_coordinate_system;
};


typedef std::shared_ptr<camera>  camera_ptr;
typedef std::shared_ptr<camera const>  camera_const_ptr;


void  view_projection_matrix(camera const&  camera_ref, matrix44&  output);


struct camera_perspective;
typedef std::shared_ptr<camera_perspective>  camera_perspective_ptr;
typedef std::shared_ptr<camera_perspective const>  camera_perspective_const_ptr;


struct camera_perspective : public camera
{
    static camera_perspective_ptr  create(
            qtgl::coordinate_system const&  coordinate_system,
            float_32_bit const  near,
            float_32_bit const  far,
            window_props const&  window_info
            );
    static camera_perspective_ptr  create(
            coordinate_system_ptr  coordinate_system,
            float_32_bit const  near,
            float_32_bit const  far,
            window_props const&  window_info
            );

    camera_perspective(qtgl::coordinate_system const&  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       float_32_bit const  left,
                       float_32_bit const  right,
                       float_32_bit const  top,
                       float_32_bit const  bottom
                       );
    camera_perspective(coordinate_system_ptr  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       float_32_bit const  left,
                       float_32_bit const  right,
                       float_32_bit const  top,
                       float_32_bit const  bottom
                       );
    camera_perspective(qtgl::coordinate_system const&  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       window_props const&  window_info);
    camera_perspective(coordinate_system_ptr  coordinate_system,
                       float_32_bit const  near,
                       float_32_bit const  far,
                       window_props const&  window_info);

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

private:

    float_32_bit  m_near;
    float_32_bit  m_far;
    float_32_bit  m_left;
    float_32_bit  m_right;
    float_32_bit  m_bottom;
    float_32_bit  m_top;
};


void  adjust(camera_perspective&  camera_ref, window_props const&  window_info);


}

#endif