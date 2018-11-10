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


}
