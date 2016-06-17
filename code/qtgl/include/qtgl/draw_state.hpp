#ifndef QTGL_DRAW_STATE_HPP_INCLUDED
#   define QTGL_DRAW_STATE_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace qtgl {


struct draw_state
{
    static std::shared_ptr<draw_state const>  create(
            natural_32_bit const  cull_face_mode   // GL_BACK, GL_FRONT, or GL_FRONT_AND_BACK
                        = GL_BACK,
            bool const  use_alpha_blending
                        = false,
            natural_32_bit const  alpha_blending_src_function   // GL_SRC_ALPHA, GL_ONE, GL_ZERO, ...
                        = GL_SRC_ALPHA,
            natural_32_bit const  alpha_blending_dst_function   // GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE, GL_ZERO, ...
                        = GL_ONE_MINUS_CONSTANT_ALPHA
            );

    draw_state(
            natural_32_bit const  cull_face_mode,
            bool const  use_alpha_blending,
            natural_32_bit const  alpha_blending_src_function,
            natural_32_bit const  alpha_blending_dst_function
            );

    natural_32_bit  cull_face_mode() const noexcept { return m_cull_face_mode; }
    bool  use_alpha_blending() const noexcept { return m_use_alpha_blending; }
    natural_32_bit  alpha_blending_src_function() const noexcept { return m_alpha_blending_src_function; }
    natural_32_bit  alpha_blending_dst_function() const noexcept { return m_alpha_blending_dst_function; }

private:

    natural_32_bit  m_cull_face_mode;
    bool  m_use_alpha_blending;
    natural_32_bit  m_alpha_blending_src_function;
    natural_32_bit  m_alpha_blending_dst_function;
};


using  draw_state_ptr = std::shared_ptr<draw_state const>;


void  make_current(draw_state const&  state);
void  make_current(draw_state const&  state, draw_state const&  previous_state);


}

#endif
