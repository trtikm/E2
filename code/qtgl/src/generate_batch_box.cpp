#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_wireframe_box(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        boost::filesystem::path const&  data_root_dir,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    buffer  box_vertex_buffer;
    create_wireframe_box_vertex_buffer(lo_corner,hi_corner,box_vertex_buffer,id);

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/batch/wireframe_box/" + id,
        qtgl::buffers_binding(
            0U,
            2U,
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, box_vertex_buffer },
            },
            id.empty() ? id : "/generic/buffers_binding/wireframe_box/" + id
            ),
        qtgl::shaders_binding(
            canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpUmcOpcF.txt"),
            canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt"),
            id.empty() ? id : "/generic/shaders_binding/wireframe_box/" + id
            ),
        qtgl::textures_binding(),
        qtgl::draw_state::create(),
        modelspace()
        );
    return pbatch;
}


}
