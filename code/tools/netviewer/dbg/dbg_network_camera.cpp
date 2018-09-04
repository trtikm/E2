#include <netviewer/dbg/dbg_network_camera.hpp>
#include <netviewer/program_options.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/draw.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


dbg_network_camera::dbg_network_camera(
        float_32_bit const  far_plane
        )
    : m_far_plane(far_plane)
    , m_camera()
    , m_batch_basis()
    , m_batch_camera_frustum()
{}


void  dbg_network_camera::enable(qtgl::camera_perspective_ptr const  camera)
{
    if (is_enabled())
        disable();

    m_camera = std::make_shared<qtgl::camera_perspective>(*camera);
    m_camera->set_far_plane(m_far_plane);
    m_camera->set_coordinate_system(*camera->coordinate_system());

    m_batch_basis = qtgl::create_basis_vectors();

    m_batch_camera_frustum =
        qtgl::create_wireframe_perspective_frustum(
                m_camera->near_plane(),
                m_camera->far_plane(),
                m_camera->left(),
                m_camera->right(),
                m_camera->top(),
                m_camera->bottom(),
                vector4(1.0f, 1.0f, 1.0f, 1.0f)
                );
}


void  dbg_network_camera::disable()
{
    m_camera.reset();
    m_batch_basis.release();
    m_batch_camera_frustum.release();
}


void  dbg_network_camera::set_far_plane(float_32_bit const  value)
{
    m_far_plane = value;
    if (is_enabled())
    {
        m_camera->set_far_plane(m_far_plane);

        m_batch_camera_frustum =
            qtgl::create_wireframe_perspective_frustum(
                    m_camera->near_plane(),
                    m_camera->far_plane(),
                    m_camera->left(),
                    m_camera->right(),
                    m_camera->top(),
                    m_camera->bottom(),
                    vector4(1.0f, 1.0f, 1.0f, 1.0f)
                    );
    }
}


void  dbg_network_camera::on_window_resized(qtgl::window_props const&  window_props)
{
    qtgl::adjust(*m_camera,window_props);

    m_batch_camera_frustum =
        qtgl::create_wireframe_perspective_frustum(
                m_camera->near_plane(),
                m_camera->far_plane(),
                m_camera->left(),
                m_camera->right(),
                m_camera->top(),
                m_camera->bottom(),
                vector4(1.0f, 1.0f, 1.0f, 1.0f)
                );
}


void  dbg_network_camera::render_camera_frustum(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        ) const
{
    TMPROF_BLOCK();

    if (!is_enabled())
        return;

    if (qtgl::make_current(m_batch_basis, draw_state))
    {
        qtgl::render_batch(
                m_batch_basis,
                matrix_from_world_to_camera,
                matrix_from_camera_to_clipspace,
                *m_camera->coordinate_system()
                );
        draw_state = m_batch_basis.get_draw_state();
    }

    if (qtgl::make_current(m_batch_camera_frustum, draw_state))
    {
        float_32_bit const  param = -0.5f * (m_camera->near_plane() + m_camera->far_plane());

        qtgl::render_batch(
            m_batch_camera_frustum,
            matrix_from_world_to_camera,
            matrix_from_camera_to_clipspace,
            angeo::coordinate_system(
                m_camera->coordinate_system()->origin() + param * angeo::axis_z(*m_camera->coordinate_system()),
                m_camera->coordinate_system()->orientation()
                ),
            vector4(1.0f, 1.0f, 1.0f, 1.0f)
            );

        draw_state = m_batch_camera_frustum.get_draw_state();
    }
}
