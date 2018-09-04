#include <qtgl/draw_state.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/msgstream.hpp>
#include <unordered_map>

namespace {


std::unordered_map<natural_32_bit, std::string> const&  get_map_from_alpha_blending_functions_to_names()
{
    static std::unordered_map<natural_32_bit, std::string> map {
        { GL_ZERO, "GL_ZERO" },
        { GL_ONE, "GL_ONE" },
        { GL_SRC_COLOR, "GL_SRC_COLOR" },
        { GL_ONE_MINUS_SRC_COLOR, "GL_ONE_MINUS_SRC_COLOR" },
        { GL_SRC_ALPHA, "GL_SRC_ALPHA" },
        { GL_ONE_MINUS_SRC_ALPHA, "GL_ONE_MINUS_SRC_ALPHA" },
        { GL_DST_ALPHA, "GL_DST_ALPHA" },
        { GL_ONE_MINUS_DST_ALPHA, "GL_ONE_MINUS_DST_ALPHA" },
        { GL_DST_COLOR, "GL_DST_COLOR" },
        { GL_ONE_MINUS_DST_COLOR, "GL_ONE_MINUS_DST_COLOR" },
        { GL_SRC_ALPHA_SATURATE, "GL_SRC_ALPHA_SATURATE" },
        { GL_ONE_MINUS_CONSTANT_ALPHA, "GL_ONE_MINUS_CONSTANT_ALPHA" },
        };
    return map;
}


std::unordered_map<natural_32_bit, std::string> const&  get_map_from_cull_modes_to_names()
{
    static std::unordered_map<natural_32_bit, std::string> map {
        { GL_BACK, "GL_BACK" },
        { GL_FRONT, "GL_FRONT" },
        { GL_FRONT_AND_BACK, "GL_FRONT_AND_BACK" }
        };
    return map;
}


}

namespace qtgl { namespace detail {


draw_state_data::draw_state_data(async::finalise_load_on_destroy_ptr)
    : m_use_alpha_blending(false)
    , m_alpha_blending_src_function(GL_SRC_ALPHA)
    , m_alpha_blending_dst_function(GL_ONE_MINUS_CONSTANT_ALPHA)
    , m_cull_face_mode(GL_BACK)
{
    TMPROF_BLOCK();

    // TODO!
}


}}

namespace qtgl {


std::string  draw_state::compute_generic_unique_id(
        bool const  use_alpha_blending,
        natural_32_bit const  alpha_blending_src_function,
        natural_32_bit const  alpha_blending_dst_function,
        natural_32_bit const  cull_face_mode
        )
{
    return msgstream()
        << "ALPHA_BLEND=" << use_alpha_blending
        << ", BLEND_SRC_FUNC=" << get_map_from_alpha_blending_functions_to_names().at(alpha_blending_src_function)
        << ", BLEND_DST_FUNC=" << get_map_from_alpha_blending_functions_to_names().at(alpha_blending_dst_function)
        << ", CULL=" << get_map_from_cull_modes_to_names().at(cull_face_mode)
        ;
}


}

namespace qtgl {


bool  make_current(draw_state const&  state)
{
    TMPROF_BLOCK();

    if (!state.loaded_successfully())
        return false;

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

    INVARIANT(glapi().glGetError() == 0U);

    return true;
}

bool  make_current(draw_state const&  state, draw_state const&  previous_state)
{
    TMPROF_BLOCK();

    if (!previous_state.loaded_successfully())
        return make_current(state);

    if (!state.loaded_successfully())
        return false;

    if (state == previous_state)
        return true;

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

    INVARIANT(glapi().glGetError() == 0U);

    return true;
}


}
