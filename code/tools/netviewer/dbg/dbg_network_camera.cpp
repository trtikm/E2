#include <netviewer/dbg/dbg_network_camera.hpp>
#include <netviewer/program_options.hpp>
#include <netviewer/draw_utils.hpp>
#include <qtgl/batch_generators.hpp>
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

    m_batch_basis = qtgl::create_basis_vectors(get_program_options()->dataRoot());

    m_batch_camera_frustum =
        qtgl::create_wireframe_perspective_frustum(
                m_camera->near_plane(),
                m_camera->far_plane(),
                m_camera->left(),
                m_camera->right(),
                m_camera->top(),
                m_camera->bottom(),
                get_program_options()->dataRoot()
                );
}


void  dbg_network_camera::disable()
{
    m_camera.reset();
    m_batch_basis.reset();
    m_batch_camera_frustum.reset();
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
                    get_program_options()->dataRoot()
                    );
    }
}


void  dbg_network_camera::render_camera_frustum(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr&  draw_state)
{
    TMPROF_BLOCK();

    if (!is_enabled())
        return;

    if (qtgl::make_current(*m_batch_basis, *draw_state))
    {
        INVARIANT(m_batch_basis->shaders_binding().operator bool());
        render_batch(*m_batch_basis,view_projection_matrix,*m_camera->coordinate_system());
        draw_state = m_batch_basis->draw_state();
    }

    if (qtgl::make_current(*m_batch_camera_frustum, *draw_state))
    {
        INVARIANT(m_batch_camera_frustum->shaders_binding().operator bool());

        float_32_bit const  param = -0.5f * (m_camera->near_plane() + m_camera->far_plane());

        render_batch(
            *m_batch_camera_frustum,
            view_projection_matrix,
            angeo::coordinate_system(
                m_camera->coordinate_system()->origin() + param * angeo::axis_z(*m_camera->coordinate_system()),
                m_camera->coordinate_system()->orientation()
                ),
            vector4(1.0f, 1.0f, 1.0f, 1.0f)
            );

        draw_state = m_batch_camera_frustum->draw_state();
    }
}
