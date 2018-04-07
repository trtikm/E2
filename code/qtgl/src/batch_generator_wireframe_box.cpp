#include <qtgl/batch_generators.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_wireframe_box(
        vector3 const&  lo_corner,
        vector3 const&  hi_corner,
        vector4 const&  colour,
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

    return create_lines3d(vertices, colour, id);
}


}
