#include <qtgl/batch_generators.hpp>
#include <qtgl/batch.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_triangle_mesh(
        qtgl::buffer  vertex_buffer,
        qtgl::buffer  index_buffer,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit, 4> > const  colours(
            vertex_buffer.num_primitives(),
            std::array<float_32_bit, 4> {
                (float_32_bit)colour(0),
                (float_32_bit)colour(1),
                (float_32_bit)colour(2),
                (float_32_bit)colour(3)
                }
            );

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/triangle_mesh/batch/" + id,
        buffers_binding(
            0U,
            index_buffer,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION, vertex_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/triangle_mesh/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/triangle_mesh/buffers_binding/" + id
            ),
        textures_binding_map_type{},
        {}, // Texcoord binding.
        effects_config{
            nullptr,
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::BUFFER}}, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state(nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        qtgl::buffer  vertex_buffer,
        qtgl::buffer  index_buffer,
        qtgl::buffer  texcoord_buffer,
        texture const&  diffuse,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/triangle_mesh/batch/" + id,
        buffers_binding(
            0U,
            index_buffer,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION, vertex_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0, texcoord_buffer },
            },
            id.empty() ? id : "/generic/triangle_mesh/buffers_binding/" + id
            ),
        textures_binding_map_type({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, diffuse }
            }),
        texcoord_binding({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0 }
            }),
        effects_config{
            nullptr,
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE}}, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state(nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<float_32_bit, 2> > const&  texcoords,
        texture const&  diffuse,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/triangle_mesh/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/triangle_mesh/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0,
                  buffer(texcoords, (id.empty() ? id : "/generic/triangle_mesh/buffer/texcoords/" + id)) },
            },
            id.empty() ? id : "/generic/triangle_mesh/buffers_binding/" + id
            ),
        textures_binding_map_type({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, diffuse }
            }),
        texcoord_binding({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0 }
            }),
        effects_config{
            nullptr,
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE}}, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state(nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}



}
