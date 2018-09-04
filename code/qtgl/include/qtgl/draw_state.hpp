#ifndef QTGL_DRAW_STATE_HPP_INCLUDED
#   define QTGL_DRAW_STATE_HPP_INCLUDED

#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <utility/async_resource_load.hpp>
#   include <memory>
#   include <string>

namespace qtgl { namespace detail {


struct draw_state_data
{
    draw_state_data(async::finalise_load_on_destroy_ptr);

    draw_state_data(
            async::finalise_load_on_destroy_ptr,
            bool const  use_alpha_blending,
            natural_32_bit const  alpha_blending_src_function,
            natural_32_bit const  alpha_blending_dst_function,
            natural_32_bit const  cull_face_mode
            )
        : m_use_alpha_blending(use_alpha_blending)
        , m_alpha_blending_src_function(alpha_blending_src_function)
        , m_alpha_blending_dst_function(alpha_blending_dst_function)
        , m_cull_face_mode(cull_face_mode)
    {}

    bool  use_alpha_blending() const { return m_use_alpha_blending; }
    natural_32_bit  alpha_blending_src_function() const { return m_alpha_blending_src_function; }
    natural_32_bit  alpha_blending_dst_function() const { return m_alpha_blending_dst_function; }
    natural_32_bit  cull_face_mode() const { return m_cull_face_mode; }

private:

    bool  m_use_alpha_blending;
    natural_32_bit  m_alpha_blending_src_function;
    natural_32_bit  m_alpha_blending_dst_function;
    natural_32_bit  m_cull_face_mode;
};


}}

namespace qtgl {


struct draw_state : public async::resource_accessor<detail::draw_state_data>
{
    using  super = async::resource_accessor<detail::draw_state_data>;

    draw_state()
        : super()
    {}

    draw_state(
            boost::filesystem::path const&  path,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : super(async::key_type("qtgl::draw_state", path.string()), 0U, parent_finaliser)
    {}

    draw_state(
            async::finalise_load_on_destroy_ptr const  parent_finaliser,
            bool const  use_alpha_blending = false,
            natural_32_bit const  alpha_blending_src_function   // GL_SRC_ALPHA, GL_ONE, GL_ZERO, ...
                        = GL_SRC_ALPHA,
            natural_32_bit const  alpha_blending_dst_function   // GL_ONE_MINUS_SRC_ALPHA, GL_DST_ALPHA, GL_ONE, GL_ZERO, ...
                        = GL_ONE_MINUS_CONSTANT_ALPHA,
            natural_32_bit const  cull_face_mode   // GL_BACK, GL_FRONT, or GL_FRONT_AND_BACK
                        = GL_BACK
            )
        : super(
            async::key_type(
                    "qtgl::draw_state",
                    compute_generic_unique_id(
                            use_alpha_blending,
                            alpha_blending_src_function,
                            alpha_blending_dst_function,
                            cull_face_mode
                            )
                    ),
            parent_finaliser,
            use_alpha_blending,
            alpha_blending_src_function,
            alpha_blending_dst_function,
            cull_face_mode
            )
    {}

    bool  use_alpha_blending() const { return resource().use_alpha_blending(); }
    natural_32_bit  alpha_blending_src_function() const { return resource().alpha_blending_src_function(); }
    natural_32_bit  alpha_blending_dst_function() const { return resource().alpha_blending_dst_function(); }
    natural_32_bit  cull_face_mode() const { return resource().cull_face_mode(); }

private:

    static std::string  compute_generic_unique_id(
            bool const  use_alpha_blending,
            natural_32_bit const  alpha_blending_src_function,
            natural_32_bit const  alpha_blending_dst_function,
            natural_32_bit const  cull_face_mode
            );
};


bool  make_current(draw_state const&  state);
bool  make_current(draw_state const&  state, draw_state const&  previous_state);


}

#endif
