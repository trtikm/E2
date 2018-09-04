#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/log.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <string>
#include <algorithm>

#include <iostream>


simulator::simulator(vector3 const&  initial_clear_colour)
    : qtgl::real_time_simulator()
    , m_camera(
            qtgl::camera_perspective::create(
                    angeo::coordinate_system::create(
                            vector3(0.5f,0.5f,2.0f),
                            quaternion_identity()
                            ),
                    0.25f,
                    500.0f,
                    window_props()
                    )
            )
    , m_free_fly_config
            {
                {
                    false,
                    false,
                    2U,
                    2U,
                    -5.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_W()),
                },
                {
                    false,
                    false,
                    2U,
                    2U,
                    5.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_S()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    -5.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_A()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    5.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_D()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    -2.5f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_Q()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    2.5f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_E()),
                },
                {
                    true,
                    true,
                    2U,
                    0U,
                    -(10.0f * PI()) * (window_props().pixel_width_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
                {
                    true,
                    false,
                    0U,
                    1U,
                    -(10.0f * PI()) * (window_props().pixel_height_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
//                {
//                    false,
//                    false,
//                    2U,
//                    2U,
//                    -5.0f,
//                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
//                },
//                {
//                    false,
//                    false,
//                    2U,
//                    2U,
//                    5.0f,
//                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
//                },
//                {
//                    false,
//                    false,
//                    0U,
//                    0U,
//                    50.0f * (window_props().pixel_width_in_milimeters() / 1000.0f),
//                    qtgl::free_fly_controler::AND(
//                            {
//                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
//                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
//                            })
//                },
//                {
//                    false,
//                    false,
//                    1U,
//                    1U,
//                    -50.0f * (window_props().pixel_height_in_milimeters() / 1000.0f),
//                    qtgl::free_fly_controler::AND(
//                            {
//                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
//                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
//                            })
//                },
//                {
//                    true,
//                    true,
//                    2U,
//                    0U,
//                    -(10.0f * PI()) * (window_props().pixel_width_in_milimeters() / 1000.0f),
//                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
//                },
//                {
//                    true,
//                    false,
//                    0U,
//                    1U,
//                    -(10.0f * PI()) * (window_props().pixel_height_in_milimeters() / 1000.0f),
//                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
//                },
            }

    , m_grid_space{ angeo::coordinate_system::create(vector3_zero(),quaternion_identity()) }
    , m_grid_batch(
            qtgl::create_grid(
                50.0f,
                50.0f,
                50.0f,
                1.0f,
                1.0f,
                { 0.4f, 0.4f, 0.4f },
                { 0.4f, 0.4f, 0.4f },
                { 0.5f, 0.5f, 0.5f },
                { 0.5f, 0.5f, 0.5f },
                { 1.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f },
                10U,
                qtgl::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE::TRIANGLE
                )
            )

    , m_batch_space{ angeo::coordinate_system::create(vector3_zero(),quaternion_identity()) }
    , m_batches{
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/pixel_chest/all.txt")),

            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/body.txt")),
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/hair.txt")),
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/rags.txt")),

            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/miss_fortune_road_wrarior/body.txt")),
            }

    , m_effects_config(
            qtgl::effects_config::light_types{},
            qtgl::effects_config::lighting_data_types{
                { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            qtgl::effects_config::shader_output_types{ qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT },
            qtgl::FOG_TYPE::NONE
            )
{
    LOG(debug,"simulator::simulator()");

    set_clear_color(initial_clear_colour);
}

simulator::~simulator()
{
    LOG(debug,"simulator::~simulator()");
}

void simulator::next_round(float_64_bit const  miliseconds_from_previous_call,
                           bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    qtgl::adjust(*m_camera,window_props());
    auto const translated_rotated =
            qtgl::free_fly(*m_camera->coordinate_system(),m_free_fly_config,
                           miliseconds_from_previous_call,mouse_props(),keyboard_props());

    if (translated_rotated.first)
        call_listeners(notifications::camera_position_updated());
    if (translated_rotated.second)
        call_listeners(notifications::camera_orientation_updated());

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  matrix_from_world_to_camera;
    m_camera->to_camera_space_matrix(matrix_from_world_to_camera);
    matrix44  matrix_from_camera_to_clipspace;
    m_camera->projection_matrix(matrix_from_camera_to_clipspace);

    qtgl::draw_state  draw_state;

    {
        matrix44  grid_world_transformation;
        angeo::from_base_matrix(*m_grid_space,grid_world_transformation);

        if (qtgl::make_current(m_grid_batch, draw_state))
        {
            qtgl::render_batch(
                    m_grid_batch,
                    matrix_from_world_to_camera * grid_world_transformation,
                    matrix_from_camera_to_clipspace
                    );
            draw_state = m_grid_batch.get_draw_state();
        }
    }

    {
        matrix44  batch_world_transformation;
        angeo::from_base_matrix(*m_batch_space, batch_world_transformation);
        matrix44 const  matrix_from_model_to_camera = matrix_from_world_to_camera * batch_world_transformation;

        for (qtgl::batch const  batch : m_batches)
            if (qtgl::make_current(batch,draw_state))
            {
                for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform : batch.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                    switch (uniform)
                    {
                        case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                            break;
                        case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA:
                            batch.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_model_to_camera);
                            break;
                        case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                            batch.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                            break;
                    }

                qtgl::draw();

                draw_state = batch.get_draw_state();
            }
    }

    qtgl::swap_buffers();
}

void  simulator::insert_batch(boost::filesystem::path const&  batch_pathname)
{
    if (std::find_if(m_batches.cbegin(),m_batches.cend(),
                     [&batch_pathname](qtgl::batch const  ptr) { return ptr.path_component_of_uid() == batch_pathname; })
            == m_batches.cend())
        m_batches.push_back(qtgl::batch(batch_pathname, get_effects_config()));
}

void  simulator::erase_batch(boost::filesystem::path const&  batch_pathname)
{
    auto const  it = std::remove_if(m_batches.begin(),m_batches.end(),
                                    [&batch_pathname](qtgl::batch const  ptr){ return ptr.path_component_of_uid() == batch_pathname; });
    if (it != m_batches.cend())
        m_batches.erase(it);
}
