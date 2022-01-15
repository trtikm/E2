#include <gfx/batch_generators.hpp>
#include <gfx/batch.hpp>
#include <gfx/texture.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <filesystem>
#include <stdexcept>
#include <thread>
#include <sstream>

namespace gfx {


void  load_font_mono_props(std::filesystem::path const&  pathname, font_mono_props&  output)
{
    if (!std::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access the passed font file '" << pathname << "'.");

    std::filesystem::path  data_root_dir = pathname.parent_path();
    while (true)
    {
        if (data_root_dir.empty())
            throw std::runtime_error(msgstream() << "Cannot find 'font' parent directory in path: " << pathname);
        std::filesystem::path const  current_dir = data_root_dir.filename();
        data_root_dir = data_root_dir.parent_path();
        if (current_dir == "font")
            break;
    }

    boost::property_tree::ptree  ptree;
    boost::property_tree::read_info(pathname.string(), ptree);

    output.font_texture = (data_root_dir / ptree.get<std::string>("font_texture")).string();
    output.min_ascii_code = ptree.get<natural_8_bit>("min_ascii_code");
    output.max_ascii_code = ptree.get<natural_8_bit>("max_ascii_code");
    output.max_chars_in_row = ptree.get<natural_8_bit>("max_chars_in_row");
    output.min_u = ptree.get<float_32_bit>("min_u");
    output.min_v = ptree.get<float_32_bit>("min_v");
    output.char_uv_width = ptree.get<float_32_bit>("char_uv_width");
    output.char_uv_height = ptree.get<float_32_bit>("char_uv_height");
    output.char_separ_u = ptree.get<float_32_bit>("char_separ_u");
    output.char_separ_v = ptree.get<float_32_bit>("char_separ_v");
    output.space_size = ptree.get<float_32_bit>("space_size");
    output.tab_size = ptree.get<natural_8_bit>("tab_size");
    output.char_width = ptree.get<float_32_bit>("char_width");
    output.char_height = ptree.get<float_32_bit>("char_height");
    output.char_separ_dist_x = ptree.get<float_32_bit>("char_separ_dist_x");
    output.char_separ_dist_y = ptree.get<float_32_bit>("char_separ_dist_y");
    output.__batch_template__ = batch();
}


void  save_font_mono_props(
        std::filesystem::path const&  pathname,
        std::filesystem::path const&  data_root_dir,
        font_mono_props const&  props
        )
{
    boost::property_tree::ptree  ptree;
    ptree.add("font_texture", std::filesystem::relative(props.font_texture, data_root_dir).string());
    ptree.add("min_ascii_code", props.min_ascii_code);
    ptree.add("max_ascii_code", props.max_ascii_code);
    ptree.add("max_chars_in_row", props.max_chars_in_row);
    ptree.add("min_u", props.min_u);
    ptree.add("min_v", props.min_v);
    ptree.add("char_uv_width", props.char_uv_width);
    ptree.add("char_uv_height", props.char_uv_height);
    ptree.add("char_separ_u", props.char_separ_u);
    ptree.add("char_separ_v", props.char_separ_v);
    ptree.add("space_size", props.space_size);
    ptree.add("tab_size", props.tab_size);
    ptree.add("char_width", props.char_width);
    ptree.add("char_height", props.char_height);
    ptree.add("char_separ_dist_x", props.char_separ_dist_x);
    ptree.add("char_separ_dist_y", props.char_separ_dist_y);
    boost::property_tree::write_info(pathname.string(), ptree);
}


void  build_font_batch_template_if_not_built_yet(font_mono_props const&  props, std::string const&  id)
{
    if (!props.__batch_template__.empty())
        return;

    std::vector< std::array<float_32_bit, 3> > const  xyz { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
    std::vector< std::array<float_32_bit, 2> > const  uv { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

    buffers_binding const buffers_binding_(
        0U,
        3U,
        {
            { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                buffer(xyz, true, (id.empty() ? id : "/generic/text/buffer/vertices/" + id)) },
            { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0,
                buffer(uv, (id.empty() ? id : "/generic/text/buffer/texcoord/" + id)) },
        },
        id.empty() ? id : "/generic/text/buffers_binding/" + id
        );

    textures_binding_paths_map_type const textures_binding_paths{
        { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, std::filesystem::path(props.font_texture) }
    };

    texcoord_binding const  texcoord_binding_({
        { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0 }
        });

    draw_state const  dstate(nullptr, true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    props.__batch_template__ = batch(
        id.empty() ?
            async::generate_unique_custom_id(
                    "/generic/text/batch/__batch_template__<" +
                    std::filesystem::path(props.font_texture).filename().string() +
                    ">")
            :
            "/generic/text/batch/" + id,
        buffers_binding_,
        textures_binding_paths,
        texcoord_binding_,
        effects_config{
            nullptr,
            { LIGHT_TYPE::AMBIENT }, // Light types.
            { { LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE} }, // Lighting data types.
            SHADER_PROGRAM_TYPE::VERTEX, // lighting algo locaciton
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            FOG_TYPE::NONE,
            SHADER_PROGRAM_TYPE::VERTEX // fog algo location
            },
        dstate,
        modelspace(),
        skeleton_alignment(),
        batch_available_resources::alpha_testing_props(0.01f)
        );
}


texture  get_font_texture(font_mono_props const&  props, bool const  wait_for_resource)
{
    while (wait_for_resource && !props.__batch_template__.get_textures_binding().is_load_finished())
        std::this_thread::yield();
    return props.__batch_template__.get_textures_binding().bindings_map().at(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE);
}


batch  create_text(
        std::string const&  text,
        font_mono_props const&  props,
        float_32_bit const  max_text_width,
        text_info* const  out_info_ptr,
        natural_8_bit const  cursor_char,
        bool const  wrap_whole_words,
        bool const  do_char_escaping,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit, 3> >  xyz;
    std::vector< std::array<float_32_bit, 2> >  uv;
    {
        xyz.reserve(text.size() * 6UL);
        uv.reserve(text.size() * 6UL);

        natural_32_bit const  max_chars_per_line = max_text_width > 0.0f ?
                (natural_32_bit)((max_text_width + props.char_separ_dist_x) / (props.char_width + props.char_separ_dist_x)) :
                std::numeric_limits<natural_32_bit>::max();

        struct  text_scan_info
        {
            natural_32_bit  row = 0U;
            natural_32_bit  column = 0U;
            vector2  cursor{ 0.0f, 0.0f};
            natural_32_bit  index = 0U;
            bool escape = false;
        };

        text_scan_info  info, last_white_char_info;
        for (natural_32_bit  text_size = (natural_32_bit)text.size(); info.index < text_size; ++info.index)
        {
            natural_8_bit  character = text.at(info.index);

            if (character == cursor_char)
            {
                if (out_info_ptr != nullptr)
                {
                    out_info_ptr->cursor_pos = info.cursor;
                    out_info_ptr->cursor_row = info.row;
                    out_info_ptr->cursor_column = info.column;
                }
                continue;
            }

            if (do_char_escaping)
            {
                if (info.escape)
                {
                    switch (character)
                    {
                    case 't':
                    case 'T':
                        character = '\t';
                        break;
                    case 'r':
                    case 'R':
                        character = '\r';
                        break;
                    case 'n':
                    case 'N':
                        character = '\n';
                        break;
                    default:
                        break;
                    }
                    info.escape = false;
                }
                else if (character == '\\')
                {
                    info.escape = true;
                    continue;
                }
            }

            switch (character)
            {
            case ' ':
                info.cursor(0) += props.space_size + props.char_separ_dist_x;
                ++info.column;
                last_white_char_info = info;
                continue;
            case '\t':
                for (natural_32_bit  i = 0U, n = props.tab_size - (info.column % props.tab_size); i != n; ++i)
                {
                    if (max_text_width > 0.0f && info.cursor(0) + props.char_width > max_text_width)
                    {
                        info.cursor(0) = 0.0f;
                        info.cursor(1) -= props.char_height + props.char_separ_dist_y;
                        info.column = 0U;
                        ++info.row;
                    }
                    info.cursor(0) += props.space_size + props.char_separ_dist_x;
                    ++info.column;
                }
                last_white_char_info = info;
                continue;
            case '\r':
                info.cursor(0) = 0.0f;
                info.column = 0U;
                last_white_char_info = info;
                continue;
            case '\n':
                info.cursor(0) = 0.0f;
                info.cursor(1) -= props.char_height + props.char_separ_dist_y;
                info.column = 0U;
                ++info.row;
                last_white_char_info = info;
                continue;
            default:
                if (character < props.min_ascii_code || character > props.max_ascii_code)
                {
                    info.cursor(0) += props.space_size + props.char_separ_dist_x;
                    ++info.column;
                    last_white_char_info = info;
                    continue;
                }
                break;
            }

            if (max_text_width > 0.0f && info.cursor(0) + props.char_width > max_text_width)
            {
                natural_32_bit const  index_delta = info.index - last_white_char_info.index ;
                if (index_delta > 1U && index_delta < max_chars_per_line)
                {
                    for (natural_32_bit  i = 1U; i < index_delta; ++i)
                    {
                        // Low triangle
                        xyz.pop_back(); xyz.pop_back(); xyz.pop_back();
                        uv.pop_back(); uv.pop_back(); uv.pop_back();
                        // High triangle
                        xyz.pop_back(); xyz.pop_back(); xyz.pop_back();
                        uv.pop_back(); uv.pop_back(); uv.pop_back();
                    }
                    info = last_white_char_info;
                    info.cursor(0) = 0.0f;
                    info.cursor(1) -= props.char_height + props.char_separ_dist_y;
                    info.column = 0U;
                    ++info.row;
                    continue;
                }

                info.cursor(0) = 0.0f;
                info.cursor(1) -= props.char_height + props.char_separ_dist_y;
                info.column = 0U;
                ++info.row;
            }

            vector2 const  hi_xy{ info.cursor(0) + props.char_width, info.cursor(1) + props.char_height };

            // Coords of the character in the characters-matrix in the texture.
            natural_32_bit const  char_row = (character - props.min_ascii_code) / props.max_chars_in_row;
            natural_32_bit const  char_column = (character - props.min_ascii_code) % props.max_chars_in_row;

            // uv-coords of the character in the texture.
            vector2 const  lo_uv{
                props.min_u + char_column * (props.char_uv_width + props.char_separ_u),
                1.0f - (props.min_v + props.char_uv_height + char_row * (props.char_uv_height + props.char_separ_v)),
            };
            vector2 const  hi_uv{ lo_uv(0) + props.char_uv_width, lo_uv(1) + props.char_uv_height };

            // Low triangle
            xyz.push_back({ info.cursor(0), info.cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({ hi_xy(0),  info.cursor(1), 0.0f }); uv.push_back({ hi_uv(0), lo_uv(1) });
            xyz.push_back({ hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });

            // High triangle
            xyz.push_back({ info.cursor(0), info.cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({ hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });
            xyz.push_back({ info.cursor(0),  hi_xy(1), 0.0f }); uv.push_back({ lo_uv(0), hi_uv(1) });

            info.cursor(0) += props.char_width + props.char_separ_dist_x;
            ++info.column;
        }
        if (out_info_ptr != nullptr)
            out_info_ptr->num_rows = info.row + 1U;
    }

    if (xyz.empty())
        return batch();

    build_font_batch_template_if_not_built_yet(props, id);

    return batch(
        id.empty() ?
            async::generate_unique_custom_id("/generic/text/batch/" + text.substr(0UL, 50UL))
            :
            "/generic/text/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                        buffer(xyz, true, (id.empty() ? id : "/generic/text/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0,
                        buffer(uv, (id.empty() ? id : "/generic/text/buffer/texcoord/" + id)) },
            },
            id.empty() ? id : "/generic/text/buffers_binding/" + id
            ),
        props.__batch_template__.get_shaders_binding(),
        props.__batch_template__.get_textures_binding(),
        props.__batch_template__.get_effects_config(),
        props.__batch_template__.get_draw_state(),
        props.__batch_template__.get_modelspace(),
        props.__batch_template__.get_skeleton_alignment(),
        props.__batch_template__.get_available_resources(),
        *props.__batch_template__.get_instancing_data_ptr(),
        props.__batch_template__.get_skin_name(),
        nullptr
        );
}


natural_8_bit to_lower(natural_8_bit const  c)
{
    if ((c >= 0x41U && c <= 0x5AU) || (c >= 0xC0U && c <= 0xDEU))
        return c + 0x20U;
    else if (c >= 0xA1U && c <= 0xAFU)
        return c + 0x10U;
    else
        return c;
}


std::string to_lower(std::string const& s)
{
    std::stringstream  sstr;
    for (auto c : s)
        sstr << (char)to_lower(c);
    return sstr.str();
}


natural_8_bit to_upper(natural_8_bit const  c)
{
    if ((c >= 0x61U && c <= 0x7AU) || (c >= 0xE0U && c <= 0xFEU))
        return c - 0x20U;
    else if (c >= 0xB1U && c <= 0xBFU)
        return c - 0x10U;
    else
        return c;
}


std::string to_upper(std::string const& s)
{
    std::stringstream  sstr;
    for (auto c : s)
        sstr << (char)to_upper(c);
    return sstr.str();
}


}
