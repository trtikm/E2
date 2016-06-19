#include <qtgl/draw_state.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>

namespace qtgl {


std::shared_ptr<draw_state const>  draw_state::create(
        natural_32_bit const  cull_face_mode,
        bool const  use_alpha_blending,
        natural_32_bit const  alpha_blending_src_function,
        natural_32_bit const  alpha_blending_dst_function
        )
{
    return std::make_shared<draw_state const>(
                cull_face_mode,
                use_alpha_blending,
                alpha_blending_src_function,
                alpha_blending_dst_function
                );
}

draw_state::draw_state(
        natural_32_bit const  cull_face_mode,
        bool const  use_alpha_blending,
        natural_32_bit const  alpha_blending_src_function,
        natural_32_bit const  alpha_blending_dst_function
        )
    : m_cull_face_mode(cull_face_mode)
    , m_use_alpha_blending(use_alpha_blending)
    , m_alpha_blending_src_function(alpha_blending_src_function)
    , m_alpha_blending_dst_function(alpha_blending_dst_function)
{
    TMPROF_BLOCK();
}


void  make_current(draw_state const&  state)
{
    TMPROF_BLOCK();

    if (state.cull_face_mode() == GL_NONE)
        glapi().glDisable(GL_CULL_FACE);
    else
    {
        glapi().glEnable(GL_CULL_FACE);
        glapi().glCullFace(state.cull_face_mode());
    }
    if (state.use_alpha_blending())
        glapi().glEnable(GL_BLEND);
    else
        glapi().glDisable(GL_BLEND);
    glapi().glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());
}

void  make_current(draw_state const&  state, draw_state const&  previous_state)
{
    TMPROF_BLOCK();

    if (state.cull_face_mode() != previous_state.cull_face_mode())
    {
        if (state.cull_face_mode() == GL_NONE)
            glapi().glDisable(GL_CULL_FACE);
        else
        {
            if (previous_state.cull_face_mode() == GL_NONE)
                glapi().glEnable(GL_CULL_FACE);
            glapi().glCullFace(state.cull_face_mode());
        }
    }

    if (state.use_alpha_blending() != previous_state.use_alpha_blending())
        if (state.use_alpha_blending())
        {
            glapi().glEnable(GL_BLEND);
            glapi().glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());
        }
        else
            glapi().glDisable(GL_BLEND);
    else if (state.use_alpha_blending() &&
             (state.alpha_blending_src_function() != previous_state.alpha_blending_src_function() ||
              state.alpha_blending_dst_function() != previous_state.alpha_blending_dst_function()) )
        glapi().glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());
}


}
