#include <qtgl/text_manager.hpp>
#include <qtgl/draw.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>

namespace qtgl {


text_manager::text_manager()
    : m_font()
    , m_text()
    , m_left(-0.1f)
    , m_right(0.1f)
    , m_bottom(-0.075f)
    , m_top(0.075f)
    , m_cursor{ 0.0f, 0.0f }
    , m_colour{ 1.0f, 1.0f, 1.0f, 1.0f }
    , m_size(0.0025f)
    , m_depth_test_enabled(false)
{}


void  text_manager::set_font_directory(std::string const&  font_directory)
{
    TMPROF_BLOCK();

    std::unordered_map<char, batch>  new_font;
    {
        effects_config const  config(
                qtgl::effects_config::light_types{},
                qtgl::effects_config::lighting_data_types{ { LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::INSTANCE } },
                qtgl::effects_config::shader_output_types{ qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT },
                qtgl::FOG_TYPE::NONE
                );
        if (boost::filesystem::is_directory(font_directory))
            for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(font_directory))
                if (boost::filesystem::is_regular_file(entry.path()) && entry.path().extension() == ".txt")
                {
                    int const  character_ordinal = std::atoi(entry.path().filename().string().c_str());
                    if (character_ordinal >= 33 && character_ordinal <= 126)
                        new_font[character_ordinal] = batch(entry.path(), config);
                }
    }
    m_font.swap(new_font);
}


void  text_manager::set_viewport(
        float_32_bit const  left,
        float_32_bit const  right,
        float_32_bit const  bottom,
        float_32_bit const  top
        )
{
    std::lock_guard<std::mutex> lock(m_mutex);

    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;

    m_text.push_back({});
    m_text.back().first = build_from_camera_to_clipspace_matrix();
}


bool  text_manager::insert(char const  character, float_32_bit const  z_coord)
{
    TMPROF_BLOCK();

    std::lock_guard<std::mutex> lock(m_mutex);

    vector2 const  pos_xy = move_cursor(character);

    auto const  font_char_it = m_font.find(character);
    if (font_char_it == m_font.cend() || !font_char_it->second.loaded_successfully())
        return false;

    matrix44  size_and_position_matrix;
    {
        size_and_position_matrix = matrix44_identity();
        size_and_position_matrix(0, 0) = size_and_position_matrix(1, 1) = m_size;
        size_and_position_matrix(0, 3) = pos_xy(0);
        size_and_position_matrix(1, 3) = pos_xy(1);
        size_and_position_matrix(2, 3) = z_coord;
    }

    if (m_text.empty())
    {
        m_text.push_back({});
        m_text.back().first = build_from_camera_to_clipspace_matrix();
    }

    auto it = m_text.back().second.find(character);
    if (it == m_text.back().second.end())
        it = m_text.back().second.insert({character, vertex_shader_instanced_data_provider(font_char_it->second) }).first;

    it->second.insert_from_model_to_camera_matrix(size_and_position_matrix);
    it->second.insert_diffuse_colour(m_colour);

    return true;
}


void  text_manager::flush()
{
    TMPROF_BLOCK();

    auto const  old_depth_test_state = qtgl::glapi().glIsEnabled(GL_DEPTH_TEST) != 0;
    if (old_depth_test_state != m_depth_test_enabled)
    {
        if (m_depth_test_enabled)
            qtgl::glapi().glEnable(GL_DEPTH_TEST);
        else
            qtgl::glapi().glDisable(GL_DEPTH_TEST);
    }

    qtgl::draw_state  dstate;
    for (auto const&  matrix_and_chars : m_text)
        for (auto const& char_and_provider : matrix_and_chars.second)
        {
            TMPROF_BLOCK();

            batch const  char_batch = m_font.at(char_and_provider.first);
            if (qtgl::make_current(char_batch, dstate))
            {
                render_batch(
                        char_batch,
                        char_and_provider.second,
                        qtgl::vertex_shader_uniform_data_provider(
                            char_batch,
                            {},
                            matrix_and_chars.first
                            )
                        );
                dstate = char_batch.get_draw_state();
            }
        }

    if (old_depth_test_state != m_depth_test_enabled)
    {
        if (old_depth_test_state)
            qtgl::glapi().glEnable(GL_DEPTH_TEST);
        else
            qtgl::glapi().glDisable(GL_DEPTH_TEST);
    }

    m_text.clear();
    m_cursor = vector2{ m_left, m_top - m_size  };
}


matrix44 text_manager::build_from_camera_to_clipspace_matrix() const
{
    matrix44  from_camera_to_clipspace_matrix;
    if (m_depth_test_enabled)
        projection_matrix_orthogonal(-1000.0f, 1000.0f, m_left, m_right, m_bottom, m_top, from_camera_to_clipspace_matrix);
    else
        projection_matrix_orthogonal_2d(m_left, m_right, m_bottom, m_top, from_camera_to_clipspace_matrix);
    return from_camera_to_clipspace_matrix;
}


vector2  text_manager::move_cursor(char  character)
{
    TMPROF_BLOCK();

    float_32_bit const  char_separation = 0.1f;
    float_32_bit const  line_separation = 1.25f;
    float_32_bit const  space_size = 0.5f;
    natural_32_bit const  tab_size = 4U;

    switch (character)
    {
    case '\t':
        character = ' ';
        for (auto i = 1U; i != tab_size; ++i)
            move_cursor(character);
        break;
    case '\r':
        m_cursor(0) = m_left;
        return m_cursor;
    case '\n':
        m_cursor(0) = m_left;
        m_cursor(1) -= line_separation * m_size;
        return m_cursor;
    default:
        break;
    }

    float_32_bit  char_width;
    {
        auto const  it = m_font.find(character);
        if (it == m_font.cend() || !it->second.loaded_successfully())
            char_width = space_size;
        else
        {
            auto const&  bbox = m_font.at(character).get_buffers_binding().get_boundary();
            char_width = bbox.hi_corner()(0) - bbox.lo_corner()(0);
        }
    }

    float_32_bit const  x_shift = (char_width + char_separation) * m_size;

    vector2  ret_pos;
    {
        if (m_cursor(0) + x_shift > m_right)
        {
            m_cursor(0) = m_left;
            m_cursor(1) -= line_separation * m_size;
        }
        ret_pos = m_cursor;
    }

    m_cursor(0) += x_shift;

    return ret_pos;
}


}
