#include <qtgl/draw.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail {


static GLuint  s_id = 0U;
static natural_8_bit  s_num_components_per_primitive = 0U;
static natural_32_bit  s_num_primitives = 0U;

void  draw_make_current(GLuint const  id,
                        natural_8_bit const num_components_per_primitive,
                        natural_32_bit const  num_primitives)
{
    TMPROF_BLOCK();

    ASSUMPTION(num_components_per_primitive == 2U ||
               num_components_per_primitive == 3U ||
               num_components_per_primitive == 4U );
    ASSUMPTION(num_primitives > 0U);
    s_id = id;
    s_num_components_per_primitive = num_components_per_primitive;
    s_num_primitives = num_primitives;
}


}}

namespace qtgl {


void  draw()
{
    TMPROF_BLOCK();

    using namespace detail;
    ASSUMPTION(s_num_components_per_primitive == 2U ||
               s_num_components_per_primitive == 3U ||
               s_num_components_per_primitive == 4U );
    ASSUMPTION(s_num_primitives > 0U);

    if (s_id == 0U)
        switch (s_num_components_per_primitive)
        {
        case 1U: glapi().glDrawArrays(GL_POINTS,0U,s_num_primitives); break;
        case 2U: glapi().glDrawArrays(GL_LINES,0U,s_num_primitives); break;
        case 3U: glapi().glDrawArrays(GL_TRIANGLES,0U,s_num_primitives); break;
        default: UNREACHABLE();
        }
    else
    {
        glapi().glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_id);
        switch (s_num_components_per_primitive)
        {
        case 1U: glapi().glDrawElements(GL_POINTS,1U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        case 2U: glapi().glDrawElements(GL_LINES,2U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        case 3U: glapi().glDrawElements(GL_TRIANGLES,3U * s_num_primitives,GL_UNSIGNED_INT,nullptr); break;
        default: UNREACHABLE();
        }
    }
}


}
