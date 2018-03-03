#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch_ptr  create_wireframe_sphere(
        float_32_bit const  radius,
        natural_8_bit const  num_lines_per_quarter_of_circle,
        boost::filesystem::path const&  data_root_dir,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    buffer  sphere_vertex_buffer;
    create_wireframe_sphere_vertex_buffer(radius,num_lines_per_quarter_of_circle,sphere_vertex_buffer,id);

    batch_ptr const  pbatch = batch::create(
        id.empty() ? id : "/generic/batch/wireframe_sphere/" + id,
        qtgl::buffers_binding(
            0U,
            2U,
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, sphere_vertex_buffer },
            },
            id.empty() ? id : "/generic/buffers_binding/wireframe_sphere/" + id
            ),
        qtgl::shaders_binding::create(
            canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpUmcOpcF.txt"),
            canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt")
            ),
        qtgl::textures_binding(true),
        qtgl::draw_state::create(),
        modelspace()
        );
    return pbatch;
}


}
