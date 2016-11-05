#include <qtgl/draw.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail { namespace current_draw {


static bool  s_are_buffers_ready = false;
static GLuint  s_id = 0U;
static natural_8_bit  s_num_components_per_primitive = 0U;
static natural_32_bit  s_num_primitives = 0U;

void  set_are_buffers_ready(bool const  are_buffers_ready)
{
    s_are_buffers_ready = are_buffers_ready;
}

void  set_index_buffer_id(GLuint const id)
{
    s_id = id;
}

void  set_num_components_per_primitive(natural_8_bit const  num_components_per_primitive)
{
    ASSUMPTION(num_components_per_primitive == 2U ||
               num_components_per_primitive == 3U ||
               num_components_per_primitive == 4U );
    s_num_components_per_primitive = num_components_per_primitive;
}

void  set_num_primitives(natural_32_bit const  num_primitives)
{
    ASSUMPTION(num_primitives > 0U);
    s_num_primitives = num_primitives;
}


}}}

namespace qtgl {


void  draw()
{
    TMPROF_BLOCK();

    using namespace detail::current_draw;

    if (!s_are_buffers_ready)
        return;

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
