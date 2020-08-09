#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


std::string  make_coord_cross_id_without_prefix(float_32_bit const  size, vector4 const&  colour)
{
    using namespace gfx::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("size") << ':' << as_json(size) << ','
         << as_json("colour") << ':' << as_json(colour)
         << '}';
    return sstr.str();
}


batch  create_coord_cross(float_32_bit const  size, vector4 const&  colour)
{
    TMPROF_BLOCK();

    float_32_bit const  coord = 0.5f * size;
    static std::vector< std::array<float_32_bit, 3> > const  vertices{
        { -coord, 0.0f, 0.0f },{ coord, 0.0f, 0.0f },
        { 0.0f, -coord, 0.0f },{ 0.0f, coord, 0.0f },
        { 0.0f, 0.0f, -coord },{ 0.0f, 0.0f, coord },
    };

    return create_lines3d(vertices, colour, FOG_TYPE::NONE,
                          get_sketch_id_prefix() + sketch_kind_coord_cross() + make_coord_cross_id_without_prefix(size, colour));
}


}
