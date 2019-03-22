#include <qtgl/batch_generators.hpp>
#include <qtgl/batch.hpp>
#include <qtgl/texture.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <stdexcept>

namespace qtgl {


void  load_font_mono_props(boost::filesystem::path const&  pathname, font_mono_props&  output)
{
    if (!boost::filesystem::is_regular_file(pathname))
        throw std::runtime_error(msgstream() << "Cannot access the passed font file '" << pathname << "'.");

    boost::filesystem::path  data_root_dir = pathname.parent_path();
    while (true)
    {
        if (data_root_dir.empty())
            throw std::runtime_error(msgstream() << "Cannot find 'fonts' parent directory in path: " << pathname);
        boost::filesystem::path const  current_dir = data_root_dir.filename();
        data_root_dir = data_root_dir.parent_path();
        if (current_dir == "fonts")
            break;
    }

    boost::property_tree::ptree  ptree;
    boost::property_tree::read_info(pathname.string(), ptree);

    output.font_texture = (data_root_dir / ptree.get<std::string>("font_texture")).string();
    output.min_ascii_code = ptree.get<natural_8_bit>("min_ascii_code");
    output.max_ascii_code = ptree.get<natural_8_bit>("max_ascii_code");
    output.max_chars_in_row = ptree.get<float_32_bit>("max_chars_in_row");
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

        vector2  cursor{ 0.0f, 0.0f};
        for (natural_8_bit  character : text)
        {
            switch (character)
            {
            case ' ':
                cursor(0) += props.space_size + props.char_separ_dist_x;
                continue;
            case '\t':
                for (natural_32_bit  i = 0U; i != props.tab_size; ++i)
                {
                    if (max_text_width > 0.0f && cursor(0) + props.char_width > max_text_width)
                    {
                        cursor(0) = 0.0f;
                        cursor(1) -= props.char_height + props.char_separ_dist_y;
                    }
                    cursor(0) += props.space_size + props.char_separ_dist_x;
                }
                continue;
            case '\r':
                cursor(0) = 0.0f;
                continue;
            case '\n':
                cursor(0) = 0.0f;
                cursor(1) -= props.char_height + props.char_separ_dist_y;
                continue;
            default:
                if (character < props.min_ascii_code || character > props.max_ascii_code)
                {
                    cursor(0) += props.space_size + props.char_separ_dist_x;
                    continue;
                }
                break;
            }

            if (max_text_width > 0.0f && cursor(0) + props.char_width > max_text_width)
            {
                cursor(0) = 0.0f;
                cursor(1) -= props.char_height + props.char_separ_dist_y;
            }

            vector2 const  hi_xy{ cursor(0) + props.char_width, cursor(1) + props.char_height };

            // Coords of the character in the characters-matrix in the texture.
            natural_32_bit const  char_row = (character - props.min_ascii_code) / props.max_chars_in_row;
            natural_32_bit const  char_column = (character - props.min_ascii_code) % props.max_chars_in_row;

            // uv-coords of the character in the texture.
            vector2 const  lo_uv{
                props.min_u + char_column * (props.char_uv_width + props.char_separ_u),
                1.0f - (props.min_v + (char_row + 1U) * (props.char_uv_height + props.char_separ_v)),
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

        props.__batch_template__ = batch(
            id.empty() ? id : "/generic/text/batch/" + id,
            buffers_binding_,
            textures_binding(textures_binding_paths),
            texcoord_binding_,
            effects_config{
                { LIGHT_TYPE::AMBIENT }, // Light types.
                { { LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE} }, // Lighting data types.
                {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
                FOG_TYPE::NONE
                },
            draw_state(nullptr),
            modelspace(),
            skeleton_alignment(),
            batch_available_resources(
                buffers_binding_.get_buffers(),
                textures_binding_paths,
                texcoord_binding_,
                draw_state(),
                shaders_effects_config_type(
                    true,
                    0.2f,
                    SHADER_PROGRAM_TYPE::VERTEX,
                    SHADER_PROGRAM_TYPE::VERTEX
                    )
                )
            );
    }

    return batch(
        id.empty() ? id : "/generic/text/batch/" + id,
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
        props.__batch_template__.get_draw_state(),
        props.__batch_template__.get_modelspace(),
        props.__batch_template__.get_skeleton_alignment(),
        props.__batch_template__.get_available_resources(),
        *props.__batch_template__.get_instancing_data_ptr(),
        nullptr
        );
}


}