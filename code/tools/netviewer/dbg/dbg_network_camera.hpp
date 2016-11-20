#ifndef E2_TOOL_NETVIEWER_DBG_NETWORK_CAMERA_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_DBG_NETWORK_CAMERA_HPP_INCLUDED

#   include <qtgl/camera.hpp>
#   include <qtgl/draw.hpp>


struct  dbg_network_camera
{
    dbg_network_camera(float_32_bit const  far_plane);

    bool  is_enabled() const { return get_camera().operator bool(); }
    void  enable(qtgl::camera_perspective_ptr const  camera);
    void  disable();

    qtgl::camera_perspective_ptr  get_camera() const noexcept { return m_camera; }
    void  set_far_plane(float_32_bit const  value);

    void  render_camera_frustum(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr&  draw_state);

private:
    float_32_bit  m_far_plane;
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::batch_ptr  m_batch_basis;
    qtgl::batch_ptr  m_batch_camera_frustum;
};


#endif
