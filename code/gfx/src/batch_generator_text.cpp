#include <gfx/batch_generators.hpp>
#include <gfx/batch.hpp>
#include <gfx/texture.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <stdexcept>

namespace gfx {


void  load_font_mono_props(boost::filesystem::path const&  pathname, font_mono_props&  output)
{
    if (!boost::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access the passed font file '" << pathname << "'.");

    boost::filesystem::path  data_root_dir = pathname.parent_path();
    while (true)
    {
        if (data_root_dir.empty())
            throw std::runtime_error(msgstream() << "Cannot find 'font' parent directory in path: " << pathname);
        boost::filesystem::path const  current_dir = data_root_dir.filename();
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


batch  create_text(
        std::string const&  text,
        font_mono_props const&  props,
        float_32_bit const  max_text_width,
        std::string const&  id
        )
{
    TMPROF_BLOCK();

    std::vector< std::array<float_32_bit, 3> >  xyz;
    std::vector< std::array<float_32_bit, 2> >  uv;
    {
        xyz.reserve(text.size() * 6UL);        
        uv.reserve(text.size() * 6UL);

        natural_32_bit  column = 0U;
        vector2  cursor{ 0.0f, 0.0f};
        for (natural_8_bit  character : text)
        {
            switch (character)
            {
            case ' ':
                cursor(0) += props.space_size + props.char_separ_dist_x;
                ++column;
                continue;
            case '\t':
                for (natural_32_bit  i = 0U, n = props.tab_size - (column % props.tab_size); i != n; ++i)
                {
                    if (max_text_width > 0.0f && cursor(0) + props.char_width > max_text_width)
                    {
                        cursor(0) = 0.0f;
                        cursor(1) -= props.char_height + props.char_separ_dist_y;
                        column = 0U;
                    }
                    cursor(0) += props.space_size + props.char_separ_dist_x;
                    ++column;
                }
                continue;
            case '\r':
                cursor(0) = 0.0f;
                column = 0U;
                continue;
            case '\n':
                cursor(0) = 0.0f;
                cursor(1) -= props.char_height + props.char_separ_dist_y;
                column = 0U;
                continue;
            default:
                if (character < props.min_ascii_code || character > props.max_ascii_code)
                {
                    cursor(0) += props.space_size + props.char_separ_dist_x;
                    ++column;
                    continue;
                }
                break;
            }

            if (max_text_width > 0.0f && cursor(0) + props.char_width > max_text_width)
            {
                cursor(0) = 0.0f;
                cursor(1) -= props.char_height + props.char_separ_dist_y;
                column = 0U;
            }

            vector2 const  hi_xy{ cursor(0) + props.char_width, cursor(1) + props.char_height };

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
            xyz.push_back({ cursor(0), cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0), cursor(1), 0.0f }); uv.push_back({ hi_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });

            // High triangle
            xyz.push_back({ cursor(0), cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });
            xyz.push_back({ cursor(0),  hi_xy(1), 0.0f }); uv.push_back({ lo_uv(0), hi_uv(1) });

            cursor(0) += props.char_width + props.char_separ_dist_x;
            ++column;
        }
    }

    if (xyz.empty())
        return batch();

    if (props.__batch_template__.empty())
    {
        std::vector< std::array<float_32_bit, 3> > const  xyz { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } };
        std::vector< std::array<float_32_bit, 2> > const  uv { { 0.0f, 0.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f } };

        buffers_binding const buffers_binding_(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION,
                  buffer(xyz, true, (id.empty() ? id : "/generic/text/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0,
                  buffer(uv, (id.empty() ? id : "/generic/text/buffer/texcoord/" + id)) },
            },
            id.empty() ? id : "/generic/text/buffers_binding/" + id
            );

        textures_binding_paths_map_type const textures_binding_paths{
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, boost::filesystem::path(props.font_texture) }
        };

        texcoord_binding const  texcoord_binding_({
            { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0 }
            });

        draw_state const  dstate(nullptr, true, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        props.__batch_template__ = batch(
            id.empty() ?
                async::generate_unique_custom_id(
                        "/generic/text/batch/__batch_template__<" +
                        boost::filesystem::path(props.font_texture).filename().string() +
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

    return batch(
        id.empty() ?
            async::generate_unique_custom_id("/generic/text/batch/" + text.substr(0UL, 50UL))
            :
            "/generic/text/batch/" + id,
        buffers_binding(
            0U,
            3U,
            {
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION,
                        buffer(xyz, true, (id.empty() ? id : "/generic/text/buffer/vertices/" + id)) },
                { VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0,
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


}
