#include <qtgl/buffer_generators.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


void  create_wireframe_box_vertex_buffer(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        buffer_ptr&  output_vertex_buffer,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit,3> > const  vertices {
        { lo_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), lo_corner(1), lo_corner(2) },
        { hi_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), hi_corner(1), lo_corner(2) },
        { hi_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), hi_corner(1), lo_corner(2) },
        { lo_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), lo_corner(1), lo_corner(2) },

        { lo_corner(0), lo_corner(1), hi_corner(2) }, { hi_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), lo_corner(1), hi_corner(2) }, { hi_corner(0), hi_corner(1), hi_corner(2) },
        { hi_corner(0), hi_corner(1), hi_corner(2) }, { lo_corner(0), hi_corner(1), hi_corner(2) },
        { lo_corner(0), hi_corner(1), hi_corner(2) }, { lo_corner(0), lo_corner(1), hi_corner(2) },

        { lo_corner(0), lo_corner(1), lo_corner(2) }, { lo_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), lo_corner(1), lo_corner(2) }, { hi_corner(0), lo_corner(1), hi_corner(2) },
        { hi_corner(0), hi_corner(1), lo_corner(2) }, { hi_corner(0), hi_corner(1), hi_corner(2) },
        { lo_corner(0), hi_corner(1), lo_corner(2) }, { lo_corner(0), hi_corner(1), hi_corner(2) },
    };
    output_vertex_buffer = buffer::create(vertices,msgstream() << "generic/box/vertices" << id);
}


}
