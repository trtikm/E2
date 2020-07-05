#include <gfx/batch_generators.hpp>
#include <utility/timeprof.hpp>

namespace gfx {


batch  create_basis_vectors(FOG_TYPE const  fog_type_)
{
    TMPROF_BLOCK();

    static std::vector< std::array<float_32_bit, 3> > const  vertices{
        { 0.0f, 0.0f, 0.0f },{ 1.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },{ 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f },
    };
    static std::vector< std::array<float_32_bit, 4> > const  colours{
        { 1.0f, 0.0f, 0.0f, 1.0f },{ 1.0f, 0.0f, 0.0f, 1.0f },
        { 0.0f, 1.0f, 0.0f, 1.0f },{ 0.0f, 1.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f, 1.0f },
    };

    return create_lines3d(vertices, colours, fog_type_, "basis_vectors<fog:" + name(fog_type_) + ">");
}


}
