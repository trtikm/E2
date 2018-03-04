#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_basis_vectors(boost::filesystem::path const&  data_root_dir)
{
    TMPROF_BLOCK();

    buffer  basis_vertex_buffer;
    buffer  basis_colour_buffer;
    create_basis_vectors_vertex_and_colour_buffers(basis_vertex_buffer,basis_colour_buffer);

    batch const  pbatch = batch(
            "generic/batch/basis_vectors",
            qtgl::buffers_binding(
                0U,
                2U,
                {
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, basis_vertex_buffer },
                    { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR, basis_colour_buffer },
                },
                "/generic/buffers_binding/basis_vectors"
                ),
            qtgl::shaders_binding(
                canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
                canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt"),
                "/generic/shaders_binding/basis_vectors"
                ),
            qtgl::textures_binding(),
            qtgl::draw_state::create(),
            modelspace()
            );

    return pbatch;
}


}
