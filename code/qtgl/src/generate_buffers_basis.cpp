#include <qtgl/buffer_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


void  create_basis_vectors_vertex_and_colour_buffers(
        buffer_ptr&  output_vertex_buffer,
        buffer_ptr&  output_colour_buffer
        )
{
    static std::vector< std::array<float_32_bit,3> > const  vertices {
            { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f },
            };
    static std::vector< std::array<float_32_bit,3> > const  colours {
            { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f },
            { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f },
            { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f },
            };

    output_vertex_buffer = buffer::create(vertices,"generic/basis/vertices");
    output_colour_buffer = buffer::create(colours,"generic/basis/colours");
}


}
