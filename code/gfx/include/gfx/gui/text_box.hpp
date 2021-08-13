#ifndef GFX_GUI_TEXT_BOX_HPP_INCLUDED
#   define GFX_GUI_TEXT_BOX_HPP_INCLUDED

#   include <gfx/gui/window.hpp>
#   include <gfx/batch.hpp>
#   include <gfx/batch_generators.hpp>
#   include <string>
#   include <memory>

namespace gfx { namespace gui {


struct  text_box : public  window
{
    using  super = window;

    text_box(
        std::shared_ptr<font_mono_props const> const  font,
        std::shared_ptr<viewport const> const  vp,
        bool const  resize_with_viewport_ = true,
        vector2 const&  lo = { 0.0f, 0.0f },
        vector2 const&  hi = { 1.0f, 1.0f }
        );

    void  set_text(std::string const&  text) { m_text = text; }

    void  update(float_32_bit const  round_seconds, osi::keyboard_props const&  keyboard, osi::mouse_props const&  mouse) override;
    void  render(draw_state&  dstate) const override;

private:

    struct  text_id
    {
        text_id();
        text_id(std::string const&  text_, float_32_bit const  scale_, float_32_bit const  width_);
        bool  operator==(text_id const&  other) const;
        bool  operator!=(text_id const&  other) const;

        std::string  text;
        float_32_bit  scale;
        float_32_bit  width;
    };

    std::shared_ptr<font_mono_props const>  m_font;

    std::string  m_text;
    text_id  m_tid;
    gfx::text_info  m_text_info;
    gfx::batch  m_text_batch;

    natural_32_bit  m_bottom_line_index;
    natural_32_bit  m_scroll_lines_delta;

    bool  m_cursor_visible;
    float_32_bit  m_cursor_countdown_seconds;
    float_32_bit  m_cursor_scale;
    gfx::batch  m_cursor_batch;


};


}}

#endif
