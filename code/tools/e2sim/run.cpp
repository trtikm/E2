#include <osi/simulator.hpp>
#include <osi/opengl.hpp>
#include <gfx/draw.hpp>
#include <gfx/batch_generators.hpp>
#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


struct  simulator : public osi::simulator
{
    gfx::camera_perspective_ptr  m_camera;
    gfx::effects_config  m_effects_config;
    vector4  m_diffuse_colour;
    vector3  m_ambient_colour;
    vector4  m_specular_colour;
    vector3  m_directional_light_direction;
    vector3  m_directional_light_colour;
    vector4  m_fog_colour;
    float  m_fog_near;
    float  m_fog_far;
    gfx::batch  m_batch_grid;

    simulator()
        : osi::simulator()
        , m_camera(
                gfx::camera_perspective::create(
                        angeo::coordinate_system::create(
                                vector3(10.0f, 10.0f, 4.0f),
                                quaternion(0.293152988f, 0.245984003f, 0.593858004f, 0.707732975f)
                                ),
                        0.25f,
                        500.0f,
                        -0.2f,
                         0.2f,
                        -0.1f,
                         0.1f
                        )
                )
        , m_effects_config(
                nullptr,
                gfx::effects_config::light_types{
                    gfx::LIGHT_TYPE::AMBIENT,
                    gfx::LIGHT_TYPE::DIRECTIONAL,
                    },
                gfx::effects_config::lighting_data_types{
                    { gfx::LIGHTING_DATA_TYPE::DIRECTION, gfx::SHADER_DATA_INPUT_TYPE::UNIFORM },
                    { gfx::LIGHTING_DATA_TYPE::NORMAL, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE },
                    { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE },
                    //{ gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::UNIFORM },
                    { gfx::LIGHTING_DATA_TYPE::SPECULAR, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE }
                    },
                gfx::SHADER_PROGRAM_TYPE::VERTEX,
                gfx::effects_config::shader_output_types{
                    gfx::SHADER_DATA_OUTPUT_TYPE::DEFAULT
                    },
                gfx::FOG_TYPE::NONE,
                gfx::SHADER_PROGRAM_TYPE::VERTEX
                )
        , m_diffuse_colour{ 1.0f, 1.0f, 1.0f, 1.0f }
        , m_ambient_colour{ 0.5f, 0.5f, 0.5f }
        , m_specular_colour{ 1.0f, 1.0f, 1.0f, 2.0f }
        , m_directional_light_direction(normalised(-vector3(2.0f, 1.0f, 3.0f)))
        , m_directional_light_colour{ 1.0f, 1.0f, 1.0f }
        , m_fog_colour{ 0.25f, 0.25f, 0.25f, 2.0f }
        , m_fog_near(0.25f)
        , m_fog_far(1000.0f)
        , m_batch_grid{ 
                gfx::create_grid(
                        50.0f,
                        50.0f,
                        50.0f,
                        1.0f,
                        1.0f,
                        { 0.4f, 0.4f, 0.4f, 1.0f },
                        { 0.4f, 0.4f, 0.4f, 1.0f },
                        { 0.5f, 0.5f, 0.5f, 1.0f },
                        { 0.5f, 0.5f, 0.5f, 1.0f },
                        { 1.0f, 0.0f, 0.0f, 1.0f },
                        { 0.0f, 1.0f, 0.0f, 1.0f },
                        { 0.0f, 0.0f, 1.0f, 1.0f },
                        10U,
                        gfx::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE::TRIANGLE,
                        m_effects_config.get_fog_type(),
                        "e2sim"
                        )
                }
    {}

    void  terminate() override
    {
        m_batch_grid.release();
    }

    void  round()
    {
        glViewport(0, 0, window_width(), window_height());
        if (has_focus())
            glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
        else
            glClearColor(0.75f, 0.25f, 0.25f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        matrix44  matrix_from_world_to_camera;
        m_camera->to_camera_space_matrix(matrix_from_world_to_camera);
        matrix44  matrix_from_camera_to_clipspace;
        m_camera->projection_matrix(matrix_from_camera_to_clipspace);

        gfx::draw_state  draw_state;
        if (gfx::make_current(m_batch_grid, draw_state))
        {
            gfx::render_batch(
                m_batch_grid,
                gfx::vertex_shader_uniform_data_provider(
                    m_batch_grid,
                    { matrix_from_world_to_camera },
                    matrix_from_camera_to_clipspace,
                    m_diffuse_colour,
                    m_ambient_colour,
                    m_specular_colour,
                    transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                    m_directional_light_colour,
                    m_fog_colour,
                    m_fog_near,
                    m_fog_far
                    )
                );
            draw_state = m_batch_grid.get_draw_state();
        }
    }
};


void run(int argc, char* argv[])
{
    simulator s;
    osi::run(s);
}
