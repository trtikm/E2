#include <qtgl/batch_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<float_32_bit, 4> > const&  colours,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!vertices.empty() && vertices.size() % 2ULL == 0ULL);
    ASSUMPTION(vertices.size() == colours.size());

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/lines3d_with_colours/batch/" + id,
        buffers_binding(
            0U,
            2U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/lines3d_with_colours/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/lines3d_with_colours/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/lines3d_with_colours/buffers_binding/" + id
            ),
        textures_binding(),
        {}, // Texcoord binding.
        effects_config{
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::BUFFER}},
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_
            },
        draw_state(nullptr),
        modelspace(),
        skeleton_alignment()
        );
    return pbatch;
}


batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::array<float_32_bit, 4> const&  common_colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(common_colour.at(0) >= 0.0f && common_colour.at(0) <= 1.0f);
    ASSUMPTION(common_colour.at(1) >= 0.0f && common_colour.at(1) <= 1.0f);
    ASSUMPTION(common_colour.at(2) >= 0.0f && common_colour.at(2) <= 1.0f);
    ASSUMPTION(common_colour.at(3) >= 0.0f && common_colour.at(3) <= 1.0f);

    std::vector< std::array<float_32_bit, 4> > const  colours(vertices.size(), common_colour);

    return create_lines3d(vertices, colours, fog_type_, id);
}


batch  create_lines3d(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        vector4 const&  common_colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::array<float_32_bit, 4> const  colour{
        (float_32_bit)common_colour(0),
        (float_32_bit)common_colour(1),
        (float_32_bit)common_colour(2),
        (float_32_bit)common_colour(3)
    };

    return create_lines3d(vertices,colour,fog_type_,id);
}


batch  create_lines3d(
        std::vector< std::pair<vector3, vector3> > const&  lines,
        std::vector< vector4 > const&  colours_of_lines,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(lines.size() == colours_of_lines.size());

    std::vector< std::array<float_32_bit, 3> >  vertices;
    for (auto const& AB : lines)
    {
        vertices.push_back({ (float_32_bit)AB.first(0), (float_32_bit)AB.first(1), (float_32_bit)AB.first(2) });
        vertices.push_back({ (float_32_bit)AB.second(0), (float_32_bit)AB.second(1), (float_32_bit)AB.second(2) });
    }

    std::vector< std::array<float_32_bit, 4> >  colours;
    for (auto const& colour : colours_of_lines)
    {
        colours.push_back({
            (float_32_bit)colour(0),
            (float_32_bit)colour(1),
            (float_32_bit)colour(2),
            (float_32_bit)colour(3)
            });
        colours.push_back(colours.back());
    }

    INVARIANT(vertices.size() == colours.size());

    return create_lines3d(vertices, colours, fog_type_, id);
}


batch  create_lines3d(
        std::vector< std::pair<vector3, vector3> > const&  lines,
        vector4 const&  common_colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit, 3> >  vertices;
    for (auto const& AB : lines)
    {
        vertices.push_back({ (float_32_bit)AB.first(0), (float_32_bit)AB.first(1), (float_32_bit)AB.first(2) });
        vertices.push_back({ (float_32_bit)AB.second(0), (float_32_bit)AB.second(1), (float_32_bit)AB.second(2) });
    }

    return create_lines3d(vertices,common_colour,fog_type_,id);
}


}
