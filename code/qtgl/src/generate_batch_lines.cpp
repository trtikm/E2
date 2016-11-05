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
    std::string const&  id
    )
{
    TMPROF_BLOCK();

    ASSUMPTION(!vertices.empty() && vertices.size() % 2ULL == 0ULL);
    ASSUMPTION(vertices.size() == colours.size());

    batch_ptr const  pbatch = batch::create(
        msgstream() << "generic/batch/lines3d" << (id.empty() ? "" : "/") << id << msgstream::end(),
        qtgl::buffers_binding::create(
            2U, {},
            {
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION,
                  buffer::create(vertices, msgstream() << "generic/lines3d/vertices" << id << msgstream::end()) },
                { qtgl::vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR,
                  buffer::create(colours, msgstream() << "generic/lines3d/colours" << id << msgstream::end()) },
            }
            ),
        qtgl::shaders_binding::create(
            canonical_path("../data/shared/gfx/shaders/vertex/vs_IpcUmOpcFc.a=1.txt"),
            canonical_path("../data/shared/gfx/shaders/fragment/fs_IcFc.txt")
            ),
        qtgl::textures_binding::create(textures_binding::texture_files_map{}),
        qtgl::draw_state::create()
        );
    return pbatch;
}

batch_ptr  create_lines3d(
    std::vector< std::pair<vector3, vector3> > const&  lines,
    vector3 const&  common_colour,
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

    return create_lines3d(vertices,colours,id);
}


}
