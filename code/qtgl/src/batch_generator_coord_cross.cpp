#include <qtgl/batch_generators.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


batch  create_coord_cross(
        float_32_bit const  size,
        vector4 const&  colour,
        FOG_TYPE const  fog_type_,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    float_32_bit const  coord = 0.5f * size;
    static std::vector< std::array<float_32_bit, 3> > const  vertices{
        { -coord, 0.0f, 0.0f },{ coord, 0.0f, 0.0f },
        { 0.0f, -coord, 0.0f },{ 0.0f, coord, 0.0f },
        { 0.0f, 0.0f, -coord },{ 0.0f, 0.0f, coord },
    };

    return create_lines3d(vertices, colour, fog_type_, id);
}


}
