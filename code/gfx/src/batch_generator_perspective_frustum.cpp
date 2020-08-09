#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


std::string  make_perspective_frustum_id_without_prefix(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  with_axis
        )
{
    using namespace gfx::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("near_plane") << ':' << as_json(near_plane) << ','
         << as_json("far_plane") << ':' << as_json(far_plane) << ','
         << as_json("left_plane") << ':' << as_json(left_plane) << ','
         << as_json("right_plane") << ':' << as_json(right_plane) << ','
         << as_json("top_plane") << ':' << as_json(top_plane) << ','
         << as_json("bottom_plane") << ':' << as_json(bottom_plane) << ','
         << as_json("colour") << ':' << as_json(colour)
         << as_json("with_axis") << ':' << as_json(with_axis)
         << '}';
    return sstr.str();
}


batch  create_wireframe_perspective_frustum(
        float_32_bit const  near_plane,
        float_32_bit const  far_plane,
        float_32_bit const  left_plane,
        float_32_bit const  right_plane,
        float_32_bit const  top_plane,
        float_32_bit const  bottom_plane,
        vector4 const&  colour,
        bool const  with_axis
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(1e-4f < std::fabs(near_plane) && std::fabs(near_plane) <= std::fabs(far_plane));
    ASSUMPTION(left_plane <= right_plane);
    ASSUMPTION(bottom_plane <= top_plane);

    float_32_bit const  scale = far_plane / near_plane;

    std::array< std::array<float_32_bit,3>,8 > const  corners {
        std::array<float_32_bit,3>{ left_plane, bottom_plane, near_plane },
        std::array<float_32_bit,3>{ right_plane, bottom_plane, near_plane },
        std::array<float_32_bit,3>{ right_plane, top_plane, near_plane },
        std::array<float_32_bit,3>{ left_plane, top_plane, near_plane },

        std::array<float_32_bit,3>{ scale * left_plane, scale * bottom_plane, far_plane },
        std::array<float_32_bit,3>{ scale * right_plane, scale * bottom_plane, far_plane },
        std::array<float_32_bit,3>{ scale * right_plane, scale * top_plane, far_plane },
        std::array<float_32_bit,3>{ scale * left_plane, scale * top_plane, far_plane }
    };

    std::vector< std::array<float_32_bit,3> >  vertices {
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

    if (with_axis)
    {
        vertices.push_back({ 0.0f, 0.0f, near_plane });
        vertices.push_back({ 0.0f, 0.0f, far_plane });
    }

    return create_lines3d(vertices, colour, FOG_TYPE::NONE,
        get_sketch_id_prefix() + sketch_kind_perspective_frustum() + make_perspective_frustum_id_without_prefix(
                near_plane,
                far_plane,
                left_plane,
                right_plane,
                top_plane,
                bottom_plane,
                colour,
                with_axis
                ));
}


}
