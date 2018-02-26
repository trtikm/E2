#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch_ptr  create_basis_vectors(boost::filesystem::path const&  data_root_dir)
{
    TMPROF_BLOCK();

    buffer_ptr  basis_vertex_buffer;
    buffer_ptr  basis_colour_buffer;
    create_basis_vectors_vertex_and_colour_buffers(basis_vertex_buffer,basis_colour_buffer);

    batch_ptr const  pbatch = batch::create(
            "generic/batch/basis",
            qtgl::buffers_binding::create(
                2U, {},
                {
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, basis_vertex_buffer },
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, basis_colour_buffer },
                }
                ),
            qtgl::shaders_binding::create(
                canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
                canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt")
                ),
            qtgl::textures_binding(true),
            qtgl::draw_state::create(),
            modelspace()
            );

    return pbatch;
}


}
