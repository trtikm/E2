#include <qtgl/batch_generators.hpp>
#include <qtgl/buffer_generators.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch_ptr  create_grid(
        float_32_bit const  max_x_coordinate,
        float_32_bit const  max_y_coordinate,
        float_32_bit const  max_z_coordinate,
        float_32_bit const  step_along_x_axis,
        float_32_bit const  step_along_y_axis,
        std::array<float_32_bit, 3> const&  colour_for_x_lines,
        std::array<float_32_bit, 3> const&  colour_for_y_lines,
        std::array<float_32_bit, 3> const&  colour_for_highlighted_x_lines,
        std::array<float_32_bit, 3> const&  colour_for_highlighted_y_lines,
        std::array<float_32_bit, 3> const&  colour_for_central_x_line,
        std::array<float_32_bit, 3> const&  colour_for_central_y_line,
        std::array<float_32_bit, 3> const&  colour_for_central_z_line,
        natural_32_bit const  highlight_every,
        bool const  generate_triangle_at_origin,
        boost::filesystem::path const&  data_root_dir,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    buffer_ptr  grid_vertex_buffer;
    buffer_ptr  grid_colour_buffer;
    create_grid_vertex_and_colour_buffers(
            max_x_coordinate,
            max_y_coordinate,
            max_z_coordinate,
            step_along_x_axis,
            step_along_y_axis,
            colour_for_x_lines,
            colour_for_y_lines,
            colour_for_highlighted_x_lines,
            colour_for_highlighted_y_lines,
            colour_for_central_x_line,
            colour_for_central_y_line,
            colour_for_central_z_line,
            highlight_every,
            generate_triangle_at_origin,
            grid_vertex_buffer,
            grid_colour_buffer
            );
    batch_ptr const  pbatch = batch::create(
            msgstream() << "generic/batch/grid" << (id.empty() ? "" : "/") << id << msgstream::end(),
            qtgl::buffers_binding::create(
                2U, {},
                {
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, grid_vertex_buffer },
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, grid_colour_buffer },
                }
                ),
            qtgl::shaders_binding::create(
                canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
                canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt")
                ),
            qtgl::textures_binding::create(textures_binding::texture_files_map{}),
            qtgl::draw_state::create(),
            modelspace()
            );

    return pbatch;
}


}
