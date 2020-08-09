#include <gfx/batch_generators.hpp>
#include <gfx/detail/as_json.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


std::string  make_arrow_id_without_prefix(float_32_bit const  size, vector4 const&  colour)
{
    using namespace gfx::detail;
    std::stringstream  sstr;
    sstr << '{'
         << as_json("size") << ':' << as_json(size) << ','
         << as_json("colour") << ':' << as_json(colour)
         << '}';
    return sstr.str();
}


batch  create_arrow(float_32_bit const  size, vector4 const&  colour)
{
    TMPROF_BLOCK();

    float_32_bit const  tail = 0.1f * 0.5f * size;
    float_32_bit const  top = 0.1f * size;
    float_32_bit const  height = size;
    static std::vector< std::array<float_32_bit, 3> > const  vertices{
        { -tail, 0.0f, 0.0f },{ tail, 0.0f, 0.0f },
        { 0.0f, -tail, 0.0f },{ 0.0f, tail, 0.0f },
        { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, height },
        { 0.0f, 0.0f, height },{  tail,  0.0f, height - top },
        { 0.0f, 0.0f, height },{ -tail,  0.0f, height - top },
        { 0.0f, 0.0f, height },{  0.0f,  tail, height - top },
        { 0.0f, 0.0f, height },{  0.0f, -tail, height - top },
    };

    return create_lines3d(vertices, colour, FOG_TYPE::NONE,
                          get_sketch_id_prefix() + sketch_kind_arrow() + make_arrow_id_without_prefix(size, colour));
}


}
