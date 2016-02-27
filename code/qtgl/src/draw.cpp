#include <qtgl/draw.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail {


static GLuint  s_id = 0U;
static natural_8_bit  s_num_components_per_element = 0U;
static natural_32_bit  s_num_elements = 0U;

void  draw_make_current(GLuint const  id,
                        natural_8_bit const num_components_per_element,
                        natural_32_bit const  num_elements)
{
    TMPROF_BLOCK();

    ASSUMPTION(num_components_per_element == 2U ||
               num_components_per_element == 3U ||
               num_components_per_element == 4U );
    ASSUMPTION(num_elements > 0U);
    s_id = id;
    s_num_components_per_element = num_components_per_element;
    s_num_elements = num_elements;
}


}}

namespace qtgl {


void  draw()
{
    TMPROF_BLOCK();

    using namespace detail;
    ASSUMPTION(s_num_components_per_element == 2U ||
               s_num_components_per_element == 3U ||
               s_num_components_per_element == 4U );
    ASSUMPTION(s_num_elements > 0U);

    if (s_id == 0U)
        switch (s_num_components_per_element)
        {
        case 1U: glapi().glDrawArrays(GL_POINTS,0U,s_num_elements); break;
        case 2U: glapi().glDrawArrays(GL_LINES,0U,s_num_elements); break;
        case 3U: glapi().glDrawArrays(GL_TRIANGLES,0U,s_num_elements); break;
        default: UNREACHABLE();
        }
    else
    {
        glapi().glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,s_id);
        switch (s_num_components_per_element)
        {
        case 1U: glapi().glDrawElements(GL_POINTS,1U * s_num_elements,GL_UNSIGNED_INT,nullptr); break;
        case 2U: glapi().glDrawElements(GL_LINES,2U * s_num_elements,GL_UNSIGNED_INT,nullptr); break;
        case 3U: glapi().glDrawElements(GL_TRIANGLES,3U * s_num_elements,GL_UNSIGNED_INT,nullptr); break;
        default: UNREACHABLE();
        }
    }
}


}
