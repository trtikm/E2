#include <qtgl/buffer_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch_ptr  create_lines3d(
        std::vector< std::array<float_32_bit, 3> >  vertices,
        std::vector< std::array<float_32_bit, 3> >  colours,
        boost::filesystem::path const&  data_root_dir,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!vertices.empty() && vertices.size() % 2ULL == 0ULL);
    ASSUMPTION(vertices.size() == colours.size());

    batch_ptr const  pbatch = batch::create(
        id.empty() ? id : "/generic/batch/lines3d_with_colours/" + id,
        qtgl::buffers_binding(
            0U,
            2U,
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/buffer/vertices/lines3d_with_colours/" + id)) },
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR,
                  buffer(colours, false, id.empty() ? id : "/generic/buffer/colours/lines3d_with_colours/" + id) },
            },
            id.empty() ? id : "/generic/buffers_binding/lines3d_with_colours/" + id
            ),
        qtgl::shaders_binding(
            canonical_path(data_root_dir / "shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
            canonical_path(data_root_dir / "shared/gfx/shaders/fragment/fs_IcFc.txt"),
            id.empty() ? id : "/generic/shaders_binding/lines3d_with_colours/" + id
            ),
        qtgl::textures_binding(true),
        qtgl::draw_state::create(),
        modelspace()
        );
    return pbatch;
}

batch_ptr  create_lines3d(
    std::vector< std::pair<vector3, vector3> > const&  lines,
    vector3 const&  common_colour,
    boost::filesystem::path const&  data_root_dir,
    std::string const&  id
    )
{
    TMPROF_BLOCK();

    ASSUMPTION(!lines.empty());
    ASSUMPTION(common_colour(0) >= 0.0f && common_colour(0) <= 1.0f);
    ASSUMPTION(common_colour(1) >= 0.0f && common_colour(1) <= 1.0f);
    ASSUMPTION(common_colour(2) >= 0.0f && common_colour(2) <= 1.0f);

    std::vector< std::array<float_32_bit, 3> >  vertices;
    std::vector< std::array<float_32_bit, 3> >  colours;
    for (auto const&  AB : lines)
    {
        vertices.push_back({ (float_32_bit)AB.first(0), (float_32_bit)AB.first(1), (float_32_bit)AB.first(2) });
        colours.push_back({ (float_32_bit)common_colour(0), (float_32_bit)common_colour(1), (float_32_bit)common_colour(2) });

        vertices.push_back({ (float_32_bit)AB.second(0), (float_32_bit)AB.second(1), (float_32_bit)AB.second(2) });
        colours.push_back({ (float_32_bit)common_colour(0), (float_32_bit)common_colour(1), (float_32_bit)common_colour(2) });
    }

    return create_lines3d(vertices,colours,data_root_dir,id);
}


}
