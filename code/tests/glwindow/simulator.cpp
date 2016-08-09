#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/buffer_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <utility/tensor_math.hpp>
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


    qtgl::coordinate_system_ptr const  object_space {
            qtgl::coordinate_system::create(vector3_zero(),quaternion_identity())
            };

    qtgl::buffer_ptr const  object_vertex_buffer =
            qtgl::buffer::create({
                    std::array<float_32_bit,3>
                    { -50.0f, -50.0f, 0.0f },
                    {  50.0f, -50.0f, 0.0f },
                    {  50.0f,  50.0f, 0.0f },
                    { -50.0f,  50.0f, 0.0f }
                    },"object/vertices");
    qtgl::buffer_ptr const  object_colour_buffer =
            qtgl::buffer::create({
                    std::array<float_32_bit,3>
                    { 0.0f, 1.0f, 1.0f },
                    { 1.0f, 0.0f, 1.0f },
                    { 1.0f, 1.0f, 0.0f },
                    { 1.0f, 1.0f, 1.0f }
                    },"object/colours");
    qtgl::buffer_ptr const  object_texcoord_buffer =
            qtgl::buffer::create({
                    std::array<float_32_bit,2>
                    {   0.0f,    0.0f },
                    { 100.0f,    0.0f },
                    { 100.0f,  100.0f },
                    {   0.0f,  100.0f }
                    },"object/texcoords");
    qtgl::buffer_ptr const  object_index_buffer =
            qtgl::buffer::create({
                    std::array<natural_32_bit,3>
                    { 0U, 1U, 2U },
                    { 0U, 2U, 3U },
                    },"object/indices");
    qtgl::buffers_binding_ptr const  object_buffers_binding =
            qtgl::buffers_binding::create(
                    object_index_buffer,{},
                    {
                        { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, object_vertex_buffer },
                        { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, object_colour_buffer },
                        { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0, object_texcoord_buffer },
                    }
                    );
    qtgl::shaders_binding_ptr const  object_shaders_binding =
            qtgl::shaders_binding::create("../data/shared/gfx/shaders/vertex/vs_IpctUamOpctFc.a=Ua.txt",
                                          "../data/shared/gfx/shaders/fragment/fs_IctUdFmix(c,d).txt");

    static bool  use_chessboard_texture = true;
    static float_64_bit  texture_swap_timer = 0.0L;
    texture_swap_timer += miliseconds_from_previous_call;
    if (texture_swap_timer >= 5.0L)
    {
        texture_swap_timer -= 5.0L;
        use_chessboard_texture = !use_chessboard_texture;
    }
    qtgl::textures_binding_ptr const  object_textures_binding{
            qtgl::textures_binding::create({
                    {
                        qtgl::fragment_shader_texture_sampler_binding::BINDING_TEXTURE_DIFFUSE,
                        use_chessboard_texture ?
                                qtgl::make_chessboard_texture_properties() :
                                qtgl::texture_properties("../data/shared/gfx/textures/ruler.png")
                    }
                    })
            };
    if (!object_textures_binding.operator bool())
    {
        std::cout << "ERROR: Cannot create texture binding object.\n";
        return;
    }


    /////////////////////////////////////////////////////////////////////////////////////
    // Preparation of grid's render data
    /////////////////////////////////////////////////////////////////////////////////////


    qtgl::coordinate_system_ptr const  grid_space{
            qtgl::coordinate_system::create(vector3_zero(),quaternion_identity())
            };

    qtgl::buffer_ptr  grid_vertex_buffer;
    qtgl::buffer_ptr  grid_colour_buffer;
    qtgl::create_grid_vertex_and_colour_buffers(
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
                true,
                grid_vertex_buffer,
                grid_colour_buffer
                );
    qtgl::buffers_binding_ptr const  grid_buffers_binding {
                qtgl::buffers_binding::create(
                        2U,{},
                        {
                            { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, grid_vertex_buffer },
                            { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, grid_colour_buffer },
                        }
                        )
                };


    std::string  error_message;
    qtgl::vertex_program_ptr const  grid_vertex_program =
        qtgl::vertex_program::create(
            std::stringstream(
                "#version 420\n"
                "layout(location=0) in vec3  in_position;\n"
                "layout(location=1) in vec3  in_colour;\n"
                "layout(location=1) out vec4  out_colour;\n"
                "uniform mat4  UNIFORM_TRANSFORM_MATRIX_TRANSPOSED;\n"
                "void main() {\n"
                "    gl_Position = vec4(in_position,1.0f) * UNIFORM_TRANSFORM_MATRIX_TRANSPOSED;\n"
                "    out_colour = vec4(in_colour,1.0f);\n"
                "}\n"
                ),
            {
                {
                    qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION,
                    qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR,
                },
                {
                    qtgl::vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION,
                    qtgl::vertex_shader_output_buffer_binding_location::BINDING_OUT_COLOUR,
                },
                {
                    qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED,
                },
            },
            error_message
            );
    if (!grid_vertex_program || !error_message.empty())
    {
        std::cout << error_message << "\n";
        return;
    }
    INVARIANT(error_message.empty());

    qtgl::fragment_program_ptr const  grid_fragment_program =
        qtgl::fragment_program::create(
            std::stringstream(
                "#version 420\n"
                "layout(location=1) in vec4  in_colour;\n"
                "layout(location=0) out vec4  out_colour;\n"
                "void main() {\n"
                "    out_colour = in_colour;\n"
                "}\n"
                ),
            {
                {
                    qtgl::fragment_shader_input_buffer_binding_location::BINDING_IN_POSITION,
                    qtgl::fragment_shader_input_buffer_binding_location::BINDING_IN_COLOUR,
                },
                {
                    qtgl::fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR,
                },
                {},
            },
            error_message
            );
    if (!grid_fragment_program || !error_message.empty())
    {
        std::cout << error_message << "\n";
        return;
    }
    INVARIANT(error_message.empty());

    qtgl::shaders_binding_ptr const  grid_shaders_binding =
            qtgl::shaders_binding::create(grid_vertex_program,grid_fragment_program);
    //qtgl::shaders_binding::create(canonical_path("../data/shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
    //                              canonical_path("../data/shared/gfx/shaders/fragment/fs_IcFc.txt"));


    /////////////////////////////////////////////////////////////////////////////////////
    // Rendering
    /////////////////////////////////////////////////////////////////////////////////////

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  view_projection_matrix;
    qtgl::view_projection_matrix(*m_camera,view_projection_matrix);

    {
        matrix44  object_world_transformation;
        qtgl::transformation_matrix(*object_space,object_world_transformation);
        matrix44 const  object_transform_matrix = view_projection_matrix * object_world_transformation;

        if (qtgl::make_current(*object_shaders_binding))
        {
            qtgl::set_uniform_variable(object_shaders_binding->uniform_variable_accessor(),
                                       qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED,
                                       object_transform_matrix);
            qtgl::set_uniform_variable(object_shaders_binding->uniform_variable_accessor(),
                                       qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA,
                                       0.5f);
        }
        qtgl::make_current(*object_buffers_binding);
        qtgl::make_current(*object_textures_binding);
        qtgl::draw();
    }
    {
        matrix44  grid_world_transformation;
        qtgl::transformation_matrix(*grid_space,grid_world_transformation);
        matrix44 const  grid_transform_matrix = view_projection_matrix * grid_world_transformation;

        if (qtgl::make_current(*grid_shaders_binding))
        {
            qtgl::set_uniform_variable(grid_shaders_binding->uniform_variable_accessor(),
                                       qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED,
                                       grid_transform_matrix);
        }
        qtgl::make_current(*grid_buffers_binding);
        qtgl::draw();
    }

    qtgl::swap_buffers();
}
