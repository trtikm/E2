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

    void  on_window_resized(qtgl::window_props const&  window_props);

    void  render_camera_frustum(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        ) const;

private:
    float_32_bit  m_far_plane;
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::batch  m_batch_basis;
    qtgl::batch  m_batch_camera_frustum;
};


#endif
