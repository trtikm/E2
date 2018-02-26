#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch_ptr  create_wireframe_perspective_frustum(
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

    buffer_ptr  frustum_vertex_buffer;
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

    batch_ptr const  pbatch = batch::create(
        msgstream() << "generic/batch/perspective_frustum" << (id.empty() ? "" : "/") << id << msgstream::end(),
        qtgl::buffers_binding::create(
            2U, {},
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION, frustum_vertex_buffer },
            }
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
