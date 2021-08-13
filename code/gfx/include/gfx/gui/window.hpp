#ifndef GFX_GUI_WINDOW_HPP_INCLUDED
#   define GFX_GUI_WINDOW_HPP_INCLUDED

#   include <gfx/viewport.hpp>
#   include <gfx/draw_state.hpp>
#   include <osi/mouse_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <memory>

namespace gfx { namespace gui {


struct  window
{
    window(
        std::shared_ptr<viewport const> const  vp,
        bool const  resize_with_viewport_ = true,
        vector2 const&  lo = { 0.0f, 0.0f },
        vector2 const&  hi = { 1.0f, 1.0f }
        );
    virtual ~window() {}

    viewport const&  get_viewport() const { return *m_viewport; }
    vector2 const&  get_lo() const { return m_lo; }
    vector2 const&  get_hi() const { return m_hi; }
    bool  resize_with_viewport() const { return m_resize_with_viewport; }
    void  resize_to_viewport();

    virtual void  update(float_32_bit const  round_seconds, osi::keyboard_props const&  keyboard, osi::mouse_props const&  mouse);
    virtual void  render(draw_state&  dstate) const {}

private:
    std::shared_ptr<viewport const>  m_viewport;
    vector2  m_lo;
    vector2  m_hi;
    bool  m_resize_with_viewport;
};


}}

#endif
