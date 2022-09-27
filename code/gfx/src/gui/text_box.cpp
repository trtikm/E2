#include <gfx/gui/text_box.hpp>
#include <gfx/camera.hpp>
#include <gfx/draw.hpp>
#include <osi/provider.hpp>
#include <utility/assumptions.hpp>
#include <utility/log.hpp>
#include <utility/config.hpp>

namespace gfx { namespace gui {


text_box::text_id::text_id()
    : text_id("", 1.0f, 1.0f)
{}


text_box::text_id::text_id(std::string const&  text_, float_32_bit const  scale_, float_32_bit const  width_)
    : text(text_)
    , scale(scale_)
    , width(width_)
{}


bool  text_box::text_id::operator==(text_id const&  other) const
{
    return scale == other.scale && width == other.width && text == other.text;
}


bool  text_box::text_id::operator!=(text_id const&  other) const
{
    return !(*this == other);
}


text_box::text_box(
        std::shared_ptr<font_mono_props const> const  font,
        std::shared_ptr<viewport const> const  vp,
        bool const  resize_with_viewport_,
        vector2 const&  lo,
        vector2 const&  hi,
        bool  do_char_escaping
        )
    : window(vp, resize_with_viewport_, lo, hi)

    , m_font(font)

    , m_tid()
    , m_text_info()
    , m_text_batch()
    , m_do_char_escaping(do_char_escaping)

    , m_bottom_line_index(0U)
    , m_scroll_lines_delta(5U)

    , m_cursor_visible(true)
    , m_cursor_countdown_seconds(1.0f)
    , m_cursor_scale(1.0f)
    , m_cursor_batch()
{
    ASSUMPTION(m_font != nullptr);
}


void  text_box::update(float_32_bit const  round_seconds, osi::keyboard_props const&  keyboard, osi::mouse_props const&  mouse)
{
    super::update(round_seconds, keyboard, mouse);

    text_id const  tid(m_text, 1.0f, 0.001f * get_viewport().width_mm());

    if (tid != m_tid)
    {
        m_tid = tid;
        m_text_batch = gfx::create_text(m_text, *m_font, m_tid.width / m_tid.scale, &m_text_info, 255U, true, m_do_char_escaping, "");
        m_cursor_countdown_seconds = 1.0f;
        m_cursor_visible = true;
        m_bottom_line_index = m_text_info.num_rows == 0U ? 0U : m_text_info.num_rows - 1U;
    }
    else
    {
        m_cursor_countdown_seconds -= round_seconds;
        if (m_cursor_countdown_seconds <= 0.0f)
        {
            m_cursor_countdown_seconds = m_cursor_visible ? 0.5f : 1.0f;
            m_cursor_visible = !m_cursor_visible;
        }
    }

    float_32_bit  raw_line_idx = 
            (float_32_bit)m_bottom_line_index - m_scroll_lines_delta *
            (get_viewport().is_point_inside({ mouse.cursor_x(), mouse.cursor_y() }) ? mouse.wheel_delta_y() : 0.0f);
    raw_line_idx = std::max(0.0f, std::min((float_32_bit)(m_text_info.num_rows == 0U ? 0U : m_text_info.num_rows - 1U),
                                           raw_line_idx + 0.5f));
    m_bottom_line_index = (natural_32_bit)raw_line_idx;

    if (m_cursor_batch.empty() || m_cursor_scale != m_tid.scale)
    {
        m_cursor_scale = m_tid.scale;
        m_cursor_batch = gfx::create_lines3d(
                std::vector<std::pair<vector3,vector3> >{
                        { { 0.0f, 0.0f, 0.0f }, { 0.0f, m_font->char_height, 0.0f } }
                        },
                std::vector<vector4>{
                        { { 1.0f, 0.0f, 0.0f, 1.0f } }
                        }
                );
        m_cursor_countdown_seconds = 1.0f;
        m_cursor_visible = true;
    }

}


void  text_box::render(draw_state&  dstate) const
{
    gfx::viewport const&  vp = get_viewport();

    natural_16_bit const  left_pixel = (natural_16_bit)vp.left;
    natural_16_bit const  right_pixel = (natural_16_bit)vp.right;

    float_32_bit  left_m = 0.001f * left_pixel * vp.pixel_width_mm;
    float_32_bit  right_m = 0.001f * right_pixel * vp.pixel_width_mm;
    float_32_bit  bottom_m = 0.001f * -(vp.height() / 2.0f) * vp.pixel_height_mm;
    float_32_bit  top_m = 0.001f * (vp.height() / 2.0f) * vp.pixel_height_mm;

    float_32_bit const  line_dy = m_tid.scale * (m_font->char_height + m_font->char_separ_dist_y);
    bool const  all_fits_in = top_m - bottom_m >= line_dy * m_text_info.num_rows;

    vector3 const  pos{
            left_m,
            all_fits_in ? top_m - line_dy : bottom_m + line_dy * m_bottom_line_index,
            0.0f
            };

    matrix44 ortho_projection;
    gfx::projection_matrix_orthogonal(-1.0f, 1.0f, left_m, right_m, bottom_m, top_m, ortho_projection);

    gfx::make_current(vp);
#if PLATFORM() != PLATFORM_WEBASSEMBLY()
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif

    if (m_cursor_visible && gfx::make_current(m_cursor_batch, dstate))
    {
        gfx::render_batch(
            m_cursor_batch,
            pos + m_tid.scale * expand23(m_text_info.cursor_pos, 0.0f),
            m_tid.scale,
            ortho_projection,
            vector3{ 1.0f, 1.0f, 1.0f }
            );
        dstate = m_cursor_batch.get_draw_state();
    }

    if (gfx::make_current(m_text_batch, dstate))
    {
        gfx::render_batch(
            m_text_batch,
            pos,
            m_tid.scale,
            ortho_projection,
            vector3{ 1.0f, 1.0f, 1.0f }
            );
        dstate = m_text_batch.get_draw_state();
    }
}


}}
