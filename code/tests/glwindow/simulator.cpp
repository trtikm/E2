#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/random.hpp>
#include <utility/canonical_path.hpp>
#include <sstream>
#include <string>

#   include <iostream>


simulator::simulator()
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
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
                },
                {
                    false,
                    false,
                    2U,
                    2U,
                    5.0f,
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
                },
                {
                    false,
                    false,
                    0U,
                    0U,
                    50.0f * (window_props().pixel_width_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::AND(
                            {
                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
                            })
                },
                {
                    false,
                    false,
                    1U,
                    1U,
                    -50.0f * (window_props().pixel_height_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::AND(
                            {
                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::LEFT_MOUSE_BUTTON()),
                                qtgl::free_fly_controler::mouse_button_pressed(qtgl::RIGHT_MOUSE_BUTTON()),
                            })
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
            }
{
    qtgl::glapi().glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
    qtgl::glapi().glEnable(GL_BLEND);
}

simulator::~simulator()
{
}

void simulator::next_round(float_64_bit const  miliseconds_from_previous_call,
                           bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    /////////////////////////////////////////////////////////////////////////////////////
    // Next round computation (only update of the m_camera, for now...)
    /////////////////////////////////////////////////////////////////////////////////////


    qtgl::adjust(*m_camera,window_props());
    qtgl::free_fly(*m_camera->coordinate_system(),m_free_fly_config,
                   miliseconds_from_previous_call,mouse_props(),keyboard_props());


    /////////////////////////////////////////////////////////////////////////////////////
    // Preparation of object's render data
    /////////////////////////////////////////////////////////////////////////////////////


    angeo::coordinate_system_ptr const  object_space {
            angeo::coordinate_system::create(vector3_zero(),quaternion_identity())
            };

    qtgl::buffer const  object_vertex_buffer =
            qtgl::buffer(std::vector< std::array<float_32_bit, 3> >{
                    { -50.0f, -50.0f, 0.0f },
                    {  50.0f, -50.0f, 0.0f },
                    {  50.0f,  50.0f, 0.0f },
                    { -50.0f,  50.0f, 0.0f }
                    },true);
    qtgl::buffer const  object_colour_buffer =
            qtgl::buffer(std::vector< std::array<float_32_bit, 3> >{
                    { 0.0f, 1.0f, 1.0f },
                    { 1.0f, 0.0f, 1.0f },
                    { 1.0f, 1.0f, 0.0f },
                    { 1.0f, 1.0f, 1.0f }
                    });
    qtgl::buffer const  object_texcoord_buffer =
            qtgl::buffer(std::vector< std::array<float_32_bit, 2> >{
                    {   0.0f,    0.0f },
                    { 100.0f,    0.0f },
                    { 100.0f,  100.0f },
                    {   0.0f,  100.0f }
                    });
    qtgl::buffer const  object_index_buffer =
            qtgl::buffer(std::vector< std::array<natural_32_bit, 3> >{
                    { 0U, 1U, 2U },
                    { 0U, 2U, 3U },
                    });
    qtgl::buffers_binding const  object_buffers_binding =
            qtgl::buffers_binding(
                    0U,
                    object_index_buffer,
                    {
                        { qtgl::VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION, object_vertex_buffer },
                        { qtgl::VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE, object_colour_buffer },
                        { qtgl::VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0, object_texcoord_buffer },
                    }
                    );
    qtgl::shaders_binding const  object_shaders_binding =
            qtgl::shaders_binding("../data/shared/gfx/shaders/vertex/vs_IpctUamOpctFc.a=Ua.txt",
                                  "../data/shared/gfx/shaders/fragment/fs_IctUdFmix(c,d).txt");

    static bool  use_chessboard_texture = true;
    static float_64_bit  texture_swap_timer = 0.0L;
    texture_swap_timer += miliseconds_from_previous_call;
    if (texture_swap_timer >= 5.0L)
    {
        texture_swap_timer -= 5.0L;
        use_chessboard_texture = !use_chessboard_texture;
    }
    qtgl::textures_binding const  object_textures_binding({
                    {
                        qtgl::FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE,
                        use_chessboard_texture ?
                                qtgl::make_chessboard_texture() :
                                qtgl::texture("../data/shared/gfx/textures/ruler.txt")
                    }
            });


    /////////////////////////////////////////////////////////////////////////////////////
    // Preparation of grid's render data
    /////////////////////////////////////////////////////////////////////////////////////


    angeo::coordinate_system_ptr const  grid_space{
            angeo::coordinate_system::create(vector3_zero(),quaternion_identity())
            };

    qtgl::batch  grid_batch = qtgl::create_grid(
                50.0f,
                50.0f,
                50.0f,
                100.0f,
                100.0f,
                { 0.4f, 0.4f, 0.4f },
                { 0.4f, 0.4f, 0.4f },
                { 0.5f, 0.5f, 0.5f },
                { 0.5f, 0.5f, 0.5f },
                { 1.0f, 0.0f, 0.0f },
                { 0.0f, 1.0f, 0.0f },
                { 0.0f, 0.0f, 1.0f },
                10U,
                qtgl::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE::TRIANGLE
                );

    /////////////////////////////////////////////////////////////////////////////////////
    // Rendering
    /////////////////////////////////////////////////////////////////////////////////////

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  view_projection_matrix;
    qtgl::view_projection_matrix(*m_camera,view_projection_matrix);

    {
        matrix44  object_world_transformation;
        angeo::from_base_matrix(*object_space,object_world_transformation);
        matrix44 const  object_transform_matrix = view_projection_matrix * object_world_transformation;

        if (qtgl::make_current(object_shaders_binding))
        {
            object_shaders_binding.get_vertex_shader().set_uniform_variable(
                                       qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::TRANSFORM_MATRIX_TRANSPOSED,
                                       object_transform_matrix);
            object_shaders_binding.get_vertex_shader().set_uniform_variable(
                                       qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::COLOUR_ALPHA,
                                       0.5f);
        }
        qtgl::make_current(object_buffers_binding);
        qtgl::make_current(object_textures_binding);
        qtgl::draw();
    }
    {
        matrix44  grid_world_transformation;
        angeo::from_base_matrix(*grid_space,grid_world_transformation);
        matrix44 const  grid_transform_matrix = view_projection_matrix * grid_world_transformation;

        if (qtgl::make_current(grid_batch))
            qtgl::render_batch(grid_batch, grid_transform_matrix);
    }

    qtgl::swap_buffers();
}
