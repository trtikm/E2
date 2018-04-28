#include <qtgl/batch_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_wireframe_perspective_frustum(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(1e-4f < near_plane && near_plane <= far_plane);
    ASSUMPTION(left_plane <= right_plane);
    ASSUMPTION(bottom_plane <= top_plane);

    float_32_bit const  z_shift = 0.5f * (near_plane + far_plane);
    float_32_bit const  scale = far_plane / near_plane;

    std::array< std::array<float_32_bit,3>,8 > const  corners {
        std::array<float_32_bit,3>{ left_plane, bottom_plane, z_shift - near_plane },
        std::array<float_32_bit,3>{ right_plane, bottom_plane, z_shift - near_plane },
        std::array<float_32_bit,3>{ right_plane, top_plane, z_shift - near_plane },
        std::array<float_32_bit,3>{ left_plane, top_plane, z_shift - near_plane },

        std::array<float_32_bit,3>{ scale * left_plane, scale * bottom_plane, z_shift - far_plane },
        std::array<float_32_bit,3>{ scale * right_plane, scale * bottom_plane, z_shift - far_plane },
        std::array<float_32_bit,3>{ scale * right_plane, scale * top_plane, z_shift - far_plane },
        std::array<float_32_bit,3>{ scale * left_plane, scale * top_plane, z_shift - far_plane }
    };

    std::vector< std::array<float_32_bit,3> > const  vertices {
        corners.at(0U), corners.at(1U),
        corners.at(1U), corners.at(2U),
        corners.at(2U), corners.at(3U),
        corners.at(3U), corners.at(0U),

        corners.at(4U), corners.at(5U),
        corners.at(5U), corners.at(6U),
        corners.at(6U), corners.at(7U),
        corners.at(7U), corners.at(4U),

        corners.at(0U), corners.at(4U),
        corners.at(1U), corners.at(5U),
        corners.at(2U), corners.at(6U),
        corners.at(3U), corners.at(7U),
    };

    return create_lines3d(vertices, colour, fog_type_, id);
}


}
