#include <fontcfg/program_info.hpp>
#include <fontcfg/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <com/simulator.hpp>
#include <gfx/image.hpp>
#include <gfx/draw.hpp>
#include <gfx/batch_generators.hpp>
#include <utility/canonical_path.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


struct  simulator : public com::simulator
{
    simulator()
        : com::simulator(get_program_options()->data_root())
        , RECT_HALF_WIDTH(5.0f)
        , m_font_props()
        , m_font_rect()
        , m_chars_matrix()
        , m_example_text()
        , m_show_example_text(true)
        , m_edit_infos()
        , m_info_idx(0U)
    {}

    void  initialise() override
    {
        com::simulator::initialise();

        set_window_title(get_program_name());
        gfx::image_rgba_8888  img;
        gfx::load_png_image(context()->get_icon_root_dir() + "E2_icon.png", img);
        set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        //maximise_window();
        render_config().camera->coordinate_system()->set_origin({ 0.0f, 0.0f, 20.0f });
        render_config().camera->coordinate_system()->set_orientation(quaternion_identity());

        gfx::load_font_mono_props(get_font_pathname(), m_font_props);
        gfx::build_font_batch_template_if_not_built_yet(m_font_props);

        // Make font texture rectangle
        {
            gfx::texture const  texture = gfx::get_font_texture(m_font_props);
            vector3 const  half_size{ RECT_HALF_WIDTH, RECT_HALF_WIDTH * texture.height() / texture.width(), 0.0f };

            m_font_rect.folder = context()->insert_folder(context()->root_folder(), "font_texture");
            m_font_rect.frame = context()->insert_frame(m_font_rect.folder);//, com::invalid_object_guid(), half_size, quaternion_identity());

            static std::vector< std::array<float_32_bit, 3> > const vertices{
                { -half_size(0), -half_size(1), 0.0f }, { half_size(0), -half_size(1), 0.0f }, {  half_size(0), half_size(1), 0.0f },
                { -half_size(0), -half_size(1), 0.0f }, { half_size(0),  half_size(1), 0.0f }, { -half_size(0), half_size(1), 0.0f }
            };
            static std::vector< std::array<float_32_bit, 2> > const texcoords{
                { 0.0f, 0.0f }, { 1.0f, 0.0f }, { 1.0f, 1.0f },
                { 0.0f, 0.0f }, { 1.0f, 1.0f }, { 0.0f, 1.0f }
            };

            m_font_rect.batch = context()->insert_batch(m_font_rect.folder, "FONT.batch",
                    gfx::create_triangle_mesh(vertices, texcoords, texture, true)
                    );
            m_font_rect.half_size = half_size;
        }

        m_edit_infos.push_back({ &m_font_props.min_u, gfx::create_text("min_u",*render_config().font_props), 10000.0f });
        m_edit_infos.push_back({ &m_font_props.min_v, gfx::create_text("min_v",*render_config().font_props), 10000.0f });
        m_edit_infos.push_back({ &m_font_props.char_uv_width, gfx::create_text("char_uv_width",*render_config().font_props), 10000.0f });
        m_edit_infos.push_back({ &m_font_props.char_uv_height, gfx::create_text("char_uv_height",*render_config().font_props), 10000.0f });
        m_edit_infos.push_back({ &m_font_props.char_separ_u, gfx::create_text("char_separ_u",*render_config().font_props), 200000.0f });
        m_edit_infos.push_back({ &m_font_props.char_separ_v, gfx::create_text("char_separ_v",*render_config().font_props), 200000.0f });
        //m_edit_infos.push_back({ &m_font_props.space_size, gfx::create_text("space_size",*render_config().font_props), 10000.0f });
        m_edit_infos.push_back({ &m_font_props.char_width, gfx::create_text("char_width",*render_config().font_props), 200000.0f });
        m_edit_infos.push_back({ &m_font_props.char_height, gfx::create_text("char_height",*render_config().font_props), 200000.0f });
        m_edit_infos.push_back({ &m_font_props.char_separ_dist_x, gfx::create_text("char_separ_dist_x",*render_config().font_props), 200000.0f });
        m_edit_infos.push_back({ &m_font_props.char_separ_dist_y, gfx::create_text("char_separ_dist_y",*render_config().font_props), 200000.0f });
    }

    void  on_begin_round() override
    {
        if (get_window_props().focus_just_lost())
            simulation_config().paused = true;

        if (!get_window_props().has_focus())
            return;

        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_PAUSE()) != 0UL)
            simulation_config().paused = !simulation_config().paused;
        else if (get_keyboard_props().keys_just_pressed().count(osi::KEY_SPACE()) != 0UL)
        {
            simulation_config().paused = false;
            simulation_config().num_rounds_to_pause = 1U;
        }

        bool const  shift = get_keyboard_props().keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
                            get_keyboard_props().keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
        bool const  ctrl = get_keyboard_props().keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
                           get_keyboard_props().keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
        bool const  alt = get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
                          get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL;

        // TODO: Implement tool's functionality here.
        if (ctrl && get_keyboard_props().keys_just_pressed().count(osi::KEY_S()) != 0UL)
            gfx::save_font_mono_props(get_font_pathname(), get_program_options()->data_root(), m_font_props);

        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_T()) != 0UL)
            m_show_example_text = !m_show_example_text;
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_UP()) != 0UL && m_info_idx > 0U)
            --m_info_idx;
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_DOWN()) != 0UL && m_info_idx + 1U < (natural_32_bit)m_edit_infos.size())
            ++m_info_idx;
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_LEFT()) != 0UL)
        {
            edit_info&  info = m_edit_infos.at(m_info_idx);
            *info.value_ptr -= render_config().camera->coordinate_system()->origin()(2) / info.dividend;
            reset();
        }
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_RIGHT()) != 0UL)
        {
            edit_info&  info = m_edit_infos.at(m_info_idx);
            *info.value_ptr += render_config().camera->coordinate_system()->origin()(2) / info.dividend;
            reset();
        }
        m_font_props.space_size = m_font_props.char_width;
    }

    void  on_begin_render() override
    {
        if (get_keyboard_props().keys_pressed().count(osi::KEY_F1()) != 0UL)
        {
            SLOG(
                "\n=== HELP ===\n"
                "UP/DOWN - Select property to modify\n"
                "LEFT/RIGHT - Decrease/Increase the property\n"
                "T - Toggle show the example text.\n"
                "CTRL+S - Overwrite the edited font on the disk.\n"
                "=== END ===\n"
                );
        }

        // TODO: Implement tool's custom render here.
    }

    boost::filesystem::path  get_font_pathname() const
    {
        return canonical_path(get_program_options()->data_root()) / "font" / get_program_options()->font_file_name();
    }

    void  reset()
    {
        m_chars_matrix = gfx::batch();
        m_example_text = gfx::batch();
    }

    void  custom_render() override
    {
        if (seconds_openned() < 0.5f)
            return;

        if (m_chars_matrix.empty())
        {
            std::vector< std::pair<vector3, vector3> >  lines;
            gfx::font_mono_props const&  fnt = m_font_props;
            for (natural_16_bit  i = fnt.min_ascii_code, r = 0U; i <= fnt.max_ascii_code; ++r)
            {
                float_32_bit const  _y0 = fnt.min_v + r * (fnt.char_uv_height + fnt.char_separ_v);
                float_32_bit const  _y1 = _y0 + fnt.char_uv_height;
                float_32_bit const  y0 = (1.0f - _y0) * 2.0f * m_font_rect.half_size(1) - m_font_rect.half_size(1);
                float_32_bit const  y1 = (1.0f - _y1) * 2.0f * m_font_rect.half_size(1) - m_font_rect.half_size(1);
                for (natural_16_bit  c = 0U; c != fnt.max_chars_in_row && i <= fnt.max_ascii_code; ++c, ++i)
                {
                    float_32_bit const  _x0 = fnt.min_u + c * (fnt.char_uv_width + fnt.char_separ_u);
                    float_32_bit const  _x1 = _x0 + fnt.char_uv_width;
                    float_32_bit const  x0 = _x0 * 2.0f * m_font_rect.half_size(0) - m_font_rect.half_size(0);
                    float_32_bit const  x1 = _x1 * 2.0f * m_font_rect.half_size(0) - m_font_rect.half_size(0);
                    lines.push_back({ {x0, y0, 0.01f}, {x1, y0, 0.01f} });
                    lines.push_back({ {x1, y0, 0.01f}, {x1, y1, 0.01f} });
                    lines.push_back({ {x1, y1, 0.01f}, {x0, y1, 0.01f} });
                    lines.push_back({ {x0, y1, 0.01f}, {x0, y0, 0.01f} });
                }
            }
            m_chars_matrix = gfx::create_lines3d(lines, { 1.0f, 0.0f, 0.0f, 1.0f });
        }
        render_batch(m_chars_matrix,{ matrix44_identity() });
        //{
        //    matrix44 M;
        //    compose_from_base_matrix({ -m_font_rect.half_size(0), m_font_rect.half_size(0), 0.01f }, matrix33_identity(), M);
        //    render_batch(m_chars_matrix,{ M });
        //}

        float_32_bit  left_m = 0.001f * -(get_window_props().window_width() / 2.0f) * get_window_props().pixel_width_mm();
        float_32_bit  right_m = 0.001f * (get_window_props().window_width() / 2.0f) * get_window_props().pixel_width_mm();
        float_32_bit  bottom_m = 0.001f * -(get_window_props().window_height() / 2.0f) * get_window_props().pixel_width_mm();
        float_32_bit  top_m = 0.001f * (get_window_props().window_height() / 2.0f) * get_window_props().pixel_width_mm();

        matrix44 ortho_projection;
        gfx::projection_matrix_orthogonal(-1.0f, 1.0f, left_m, right_m, bottom_m, top_m, ortho_projection);

        glViewport(0, 0, get_window_props().window_width(), get_window_props().window_height());
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

        edit_info const&  info = m_edit_infos.at(m_info_idx);
        if (gfx::make_current(info.name, render_config().draw_state))
        {
            gfx::render_batch(
                info.name,
                { left_m, bottom_m + 0.5f * (top_m - bottom_m), 0.0f },
                render_config().text_scale,
                ortho_projection,
                vector3{ 1.0f, 1.0f, 1.0f }
                );
            render_config().draw_state = info.name.get_draw_state();
        }

        if (!m_show_example_text)
            return;

        if (m_example_text.empty())
        {
            gfx::text_info  text_info;
            m_example_text = gfx::create_text(
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ "
                    "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ",
                    //render_config().font_props,
                    m_font_props,
                    right_m - left_m,
                    &text_info
                    );
            text_info=text_info;
        }
        if (gfx::make_current(m_example_text, render_config().draw_state))
        {
            gfx::render_batch(
                m_example_text,
                { left_m, bottom_m + 0.25f * (top_m - bottom_m), 0.0f },
                1.0f,
                ortho_projection,
                vector3{ 1.0f, 1.0f, 1.0f }
                );
            render_config().draw_state = m_example_text.get_draw_state();
        }
    }

private:

    struct  font_rect
    {
        com::object_guid  folder;
        com::object_guid  frame;
        com::object_guid  batch;
        vector3  half_size;
    };

    struct  edit_info
    {
        float_32_bit*  value_ptr;
        gfx::batch  name;
        float_32_bit  dividend;
    };

    float_32_bit  RECT_HALF_WIDTH;

    gfx::font_mono_props  m_font_props;
    font_rect  m_font_rect;
    gfx::batch  m_chars_matrix;
    gfx::batch  m_example_text;
    bool  m_show_example_text;
    std::vector<edit_info>  m_edit_infos;
    natural_32_bit  m_info_idx;
};


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();
    osi::run(std::make_unique<simulator>());
}
