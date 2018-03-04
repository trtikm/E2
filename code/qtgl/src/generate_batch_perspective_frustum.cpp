#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_wireframe_perspective_frustum(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        boost::filesystem::path const&  data_root_dir,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    buffer  frustum_vertex_buffer;
    create_wireframe_perspective_frustum_vertex_buffer(
            near_plane,
            far_plane,
            left_plane,
            right_plane,
            top_plane,
            bottom_plane,
            frustum_vertex_buffer,
            id
            );

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/batch/perspective_frustum/" + id,
        qtgl::buffers_binding(
            0U,
            2U,
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, frustum_vertex_buffer },
            },
            id.empty() ? id : "/generic/buffers_binding/perspective_frustum/" + id
            ),
        qtgl::shaders_binding(
            canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpUmcOpcF.txt"),
            canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt"),
            id.empty() ? id : "/generic/shaders_binding/perspective_frustum/" + id
            ),
        qtgl::textures_binding(),
        qtgl::draw_state::create(),
        modelspace()
        );
    return pbatch;
}


}
