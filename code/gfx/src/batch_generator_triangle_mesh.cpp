#include <gfx/batch_generators.hpp>
#include <gfx/batch.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
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
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            index_buffer,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION, vertex_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/sketch/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
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
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state((async::finalise_load_on_destroy_ptr)nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
        gfx::buffer  normal_buffer,
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
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            index_buffer,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION, vertex_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_NORMAL, normal_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/sketch/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
            ),
        textures_binding_map_type{},
        {}, // Texcoord binding.
        effects_config{
            nullptr,
            { LIGHT_TYPE::AMBIENT, LIGHT_TYPE::DIRECTIONAL }, // Light types.
            {
                // Lighting data types.
                {LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::BUFFER},
                {LIGHTING_DATA_TYPE::NORMAL, SHADER_DATA_INPUT_TYPE::BUFFER},
                {LIGHTING_DATA_TYPE::DIRECTION, SHADER_DATA_INPUT_TYPE::UNIFORM}
            },
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state((async::finalise_load_on_destroy_ptr)nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        gfx::buffer  vertex_buffer,
        gfx::buffer  index_buffer,
        gfx::buffer  texcoord_buffer,
        texture const&  diffuse,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            index_buffer,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION, vertex_buffer },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0, texcoord_buffer },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
            ),
        textures_binding_map_type({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, diffuse }
            }),
        texcoord_binding({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0 }
            }),
        effects_config{
            nullptr,
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE}}, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state((async::finalise_load_on_destroy_ptr)nullptr),
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
        bool const  use_alpha_blending,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/sketch/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0,
                  buffer(texcoords, (id.empty() ? id : "/generic/sketch/buffer/texcoords/" + id)) },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
            ),
        textures_binding_map_type({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, diffuse }
            }),
        texcoord_binding({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0 }
            }),
        effects_config{
            nullptr,
            {}, // Light types.
            {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE}}, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state(nullptr, use_alpha_blending),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit, 4> > const  colours(
            vertices.size(),
            std::array<float_32_bit, 4> {
                (float_32_bit)colour(0),
                (float_32_bit)colour(1),
                (float_32_bit)colour(2),
                (float_32_bit)colour(3)
                }
            );

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/sketch/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/sketch/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
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
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
        },
        draw_state((async::finalise_load_on_destroy_ptr)nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
    );

    return pbatch;
}


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const& vertices,
        std::vector< std::array<float_32_bit, 3> > const& normals,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(vertices.size() == normals.size());

    std::vector< std::array<float_32_bit, 4> > const  colours(
            vertices.size(),
            std::array<float_32_bit, 4> {
                (float_32_bit)colour(0),
                (float_32_bit)colour(1),
                (float_32_bit)colour(2),
                (float_32_bit)colour(3)
                }
            );

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/sketch/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                  buffer(vertices, true, (id.empty() ? id : "/generic/sketch/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_NORMAL,
                  buffer(normals, false, (id.empty() ? id : "/generic/sketch/buffer/normals/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE,
                  buffer(colours, id.empty() ? id : "/generic/sketch/buffer/diffuse/" + id) },
            },
            id.empty() ? id : "/generic/sketch/buffers_binding/" + id
            ),
        textures_binding_map_type{},
        {}, // Texcoord binding.
        effects_config{
            nullptr,
            { LIGHT_TYPE::AMBIENT, LIGHT_TYPE::DIRECTIONAL }, // Light types.
            {
                // Lighting data types.
                {LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::BUFFER},
                {LIGHTING_DATA_TYPE::NORMAL, SHADER_DATA_INPUT_TYPE::BUFFER},
                {LIGHTING_DATA_TYPE::DIRECTION, SHADER_DATA_INPUT_TYPE::UNIFORM}
            },
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            fog_type_,
            fog_type_ == FOG_TYPE::DETAILED ? SHADER_PROGRAM_TYPE::FRAGMENT : SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        draw_state((async::finalise_load_on_destroy_ptr)nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props()
        );
    return pbatch;
}


batch  create_triangle_mesh(
        std::vector< std::array<float_32_bit, 3> > const&  vertices,
        std::vector< std::array<natural_32_bit, 3> > const&  indices,
        std::vector< std::array<float_32_bit, 3> > const&  normals,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    return create_triangle_mesh(
                buffer(vertices, true, (id.empty() ? id : "/generic/sketch/buffer/vertices/" + id)),
                buffer(indices, (id.empty() ? id : "/generic/sketch/buffer/indices/" + id)),
                buffer(normals, false, (id.empty() ? id : "/generic/sketch/buffer/normals/" + id)),
                colour,
                fog_type_,
                id
                );
}


}
