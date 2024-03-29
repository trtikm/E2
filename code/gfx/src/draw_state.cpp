#include <gfx/draw_state.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <filesystem>
#include <unordered_map>
#include <iostream>
#include <fstream>
#include <iomanip>

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


std::unordered_map<std::string, natural_32_bit> const&  get_map_from_alpha_blending_function_names_to_gl_values()
{
    static std::unordered_map<std::string, natural_32_bit> map {
        { "ZERO", GL_ZERO },
        { "ONE", GL_ONE },
        { "SRC_COLOR", GL_SRC_COLOR },
        { "ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR },
        { "SRC_ALPHA", GL_SRC_ALPHA },
        { "ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA },
        { "DST_ALPHA", GL_DST_ALPHA },
        { "ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA },
        { "DST_COLOR", GL_DST_COLOR },
        { "ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR },
        { "SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE },
        { "ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA },
        };
    return map;
}


std::unordered_map<std::string, natural_32_bit> const&  get_map_from_cull_mode_names_to_gl_values()
{
    static std::unordered_map<std::string, natural_32_bit> map {
        { "BACK", GL_BACK },

        // Remaining culling modes are not supported.

        //{ "FRONT", GL_FRONT },
        //{ "FRONT_AND_BACK", GL_FRONT_AND_BACK }
        };
    return map;
}


natural_32_bit  read_property(
        boost::property_tree::ptree const&  props,
        std::string const&  name,
        std::unordered_map<std::string, natural_32_bit> const&  map
        )
{
    try
    {
        return map.at(props.get<std::string>(name));
    }
    catch (...)
    {
        throw std::runtime_error(msgstream() << "Missing or invalid value for required key '" << name << "'.");
    }
};


}

namespace gfx { namespace detail {


draw_state_data::draw_state_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_use_alpha_blending(false)
    , m_alpha_blending_src_function(GL_SRC_ALPHA)
    , m_alpha_blending_dst_function(GL_ONE_MINUS_CONSTANT_ALPHA)
    , m_cull_face_mode(GL_BACK)
{
    TMPROF_BLOCK();

    std::filesystem::path const  pathname = finaliser->get_key().get_unique_id();

    if (!std::filesystem::exists(pathname))
        throw std::runtime_error(msgstream() << "The passed file '" << pathname << "' does not exist.");

    boost::property_tree::ptree draw_state_ptree;
    boost::property_tree::read_info(pathname.string(), draw_state_ptree);

    m_use_alpha_blending = draw_state_ptree.get("use_alpha_blending", false);
    m_alpha_blending_src_function = 
        read_property(draw_state_ptree, "alpha_blending_src_function", get_map_from_alpha_blending_function_names_to_gl_values());
    m_alpha_blending_dst_function =
        read_property(draw_state_ptree, "alpha_blending_dst_function", get_map_from_alpha_blending_function_names_to_gl_values());
    m_cull_face_mode = read_property(draw_state_ptree, "cull_face_mode", get_map_from_cull_mode_names_to_gl_values());
}


}}

namespace gfx {


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


draw_state  create_draw_state(boost::property_tree::ptree const&  props)
{
    return{
        nullptr,
        props.get("use_alpha_blending", false),
        read_property(props, "alpha_blending_src_function", get_map_from_alpha_blending_function_names_to_gl_values()),
        read_property(props, "alpha_blending_dst_function", get_map_from_alpha_blending_function_names_to_gl_values())
        };
}


}

namespace gfx {


bool  make_current(draw_state const&  state)
{
    TMPROF_BLOCK();

    if (!state.loaded_successfully())
        return false;

    if (state.cull_face_mode() == GL_NONE)
        glDisable(GL_CULL_FACE);
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(state.cull_face_mode());
    }
    if (state.use_alpha_blending())
        glEnable(GL_BLEND);
    else
        glDisable(GL_BLEND);
    glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());

    INVARIANT(glGetError() == 0U);

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
            glDisable(GL_CULL_FACE);
        else
        {
            if (previous_state.cull_face_mode() == GL_NONE)
                glEnable(GL_CULL_FACE);
            glCullFace(state.cull_face_mode());
        }
    }

    if (state.use_alpha_blending() != previous_state.use_alpha_blending())
        if (state.use_alpha_blending())
        {
            glEnable(GL_BLEND);
            glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());
        }
        else
            glDisable(GL_BLEND);
    else if (state.use_alpha_blending() &&
             (state.alpha_blending_src_function() != previous_state.alpha_blending_src_function() ||
              state.alpha_blending_dst_function() != previous_state.alpha_blending_dst_function()) )
        glBlendFunc(state.alpha_blending_src_function(),state.alpha_blending_dst_function());

    INVARIANT(glGetError() == 0U);

    return true;
}


}
