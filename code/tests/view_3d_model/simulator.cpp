#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/buffer_generators.hpp>
#include <utility/tensor_math.hpp>
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
                    qtgl::coordinate_system::create(
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

    , m_grid_space{ qtgl::coordinate_system::create(vector3_zero(),quaternion_identity()) }
    , m_grid_vertex_buffer()
    , m_grid_colour_buffer()
    , m_grid_buffers_binding()
    , m_grid_shaders_binding()
    , m_grid_draw_state(qtgl::draw_state::create())

    , m_batch_space{ qtgl::coordinate_system::create(vector3_zero(),quaternion_identity()) }
    , m_batches{
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/pixel_chest/all.txt")),

            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/body.txt")),
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/hair.txt")),
            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/barbarian_female/rags.txt")),

            //qtgl::batch::create(canonical_path("../data/shared/gfx/models/miss_fortune_road_wrarior/body.txt")),
            }
{
    LOG(debug,"simulator::simulator()");

    set_clear_color(initial_clear_colour);

    qtgl::create_grid_vertex_and_colour_buffers(
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
                true,
                m_grid_vertex_buffer,
                m_grid_colour_buffer
                );
    m_grid_buffers_binding =
                qtgl::buffers_binding::create(
                        2U,{},
                        {
                            { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, m_grid_vertex_buffer },
                            { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, m_grid_colour_buffer },
                        }
                        );

    m_grid_shaders_binding = qtgl::shaders_binding::create(
                canonical_path("../data/shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
                canonical_path("../data/shared/gfx/shaders/fragment/fs_IcFc.txt")
                );
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

    matrix44  view_projection_matrix;
    qtgl::view_projection_matrix(*m_camera,view_projection_matrix);

    qtgl::draw_state_ptr  draw_state = m_grid_draw_state;
    qtgl::make_current(*draw_state);

    {
        matrix44  grid_world_transformation;
        qtgl::transformation_matrix(*m_grid_space,grid_world_transformation);
        matrix44 const  grid_transform_matrix = view_projection_matrix * grid_world_transformation;

        if (qtgl::make_current(*m_grid_shaders_binding) && qtgl::make_current(*m_grid_buffers_binding))
        {
            qtgl::make_current(*m_grid_draw_state, *draw_state);

            qtgl::set_uniform_variable(m_grid_shaders_binding->uniform_variable_accessor(),
                                       qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED,
                                       grid_transform_matrix);
            qtgl::draw();

            draw_state = m_grid_draw_state;
        }
    }

    {
        matrix44  batch_world_transformation;
        qtgl::transformation_matrix(*m_batch_space, batch_world_transformation);
        matrix44 const  batch_transform_matrix = view_projection_matrix * batch_world_transformation;

        for (qtgl::batch_ptr const  batch : m_batches)
            if (qtgl::make_current(*batch,*draw_state))
            {
                for (qtgl::vertex_shader_uniform_symbolic_name const  uniform : batch->symbolic_names_of_used_uniforms())
                    switch (uniform)
                    {
                        case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
                            break;
                        case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
                            INVARIANT(batch->shaders_binding().operator bool());
                            qtgl::set_uniform_variable(batch->shaders_binding()->uniform_variable_accessor(),
                                                       uniform,batch_transform_matrix);
                            break;
                    }

                qtgl::draw();

                draw_state = batch->draw_state();
            }
    }

    qtgl::swap_buffers();
}

void  simulator::insert_batch(boost::filesystem::path const&  batch_pathname)
{
    qtgl::batch_ptr const  ptr = qtgl::batch::create(batch_pathname);
    if (std::find(m_batches.cbegin(),m_batches.cend(),ptr) == m_batches.cend())
        m_batches.push_back(ptr);
}

void  simulator::erase_batch(boost::filesystem::path const&  batch_pathname)
{
    auto const  it = std::remove_if(m_batches.begin(),m_batches.end(),
                                    [&batch_pathname](qtgl::batch_ptr const  ptr){ return ptr->path() == batch_pathname; });
    if (it != m_batches.cend())
        m_batches.erase(it);
}
