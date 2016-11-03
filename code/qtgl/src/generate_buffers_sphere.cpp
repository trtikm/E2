#include <qtgl/buffer_generators.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


void  create_wireframe_sphere_vertex_buffer(
        float_32_bit const  raduis,
        natural_8_bit const  num_horizontal_slices,
        natural_8_bit const  num_vertical_slices,
        buffer_ptr&  output_vertex_buffer,
        std::string const&  id
        )
{
    std::vector< std::array<float_32_bit,3> > const  vertices {

    };
    NOT_IMPLEMENTED_YET();
    output_vertex_buffer = buffer::create(vertices,msgstream() << "generic/spehre/vertices" << id);
}



}
