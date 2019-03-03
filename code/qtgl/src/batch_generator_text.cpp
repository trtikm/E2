#include <qtgl/batch_generators.hpp>
#include <qtgl/batch.hpp>
#include <qtgl/texture.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/filesystem/path.hpp>

namespace qtgl {


std::string  load_font_mono_props(boost::filesystem::path const  pathname, font_mono_props&  output)
{
    return "NOT IMPLEMENTED YET!";
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
                cursor(0) += props.space_size;
                continue;
            case '\t':
                cursor(0) += props.tab_size;
                continue;
            case '\r':
                cursor(0) = 0.0f;
                continue;
            case '\n':
                cursor(0) = 0.0f;
                cursor(0) -= props.char_height + props.char_separ_dist_y;
                continue;
            default:
                if (character < props.min_ascii_code || character > props.max_ascii_code)
                {
                    cursor(0) += props.space_size;
                    continue;
                }
                break;
            }

            if (max_text_width > 0.0f && cursor(0) + props.char_width > max_text_width)
            {
                cursor(0) = 0.0f;
                cursor(0) -= props.char_height + props.char_separ_dist_y;
            }

            vector2 const  hi_xy{ cursor(0) + props.char_width, cursor(1) - props.char_height };

            // Coords of the character in the characters-matrix in the texture.
            natural_32_bit const  char_row = (character - props.min_ascii_code) / props.max_chars_in_row;
            natural_32_bit const  char_column = (character - props.min_ascii_code) % props.max_chars_in_row;

            // uv-coords of the character in the texture.
            vector2 const  lo_uv{ props.min_u + char_column * props.char_width, props.min_v + (char_row + 1U) * props.char_height, };
            vector2 const  hi_uv{ lo_uv(0) + props.char_uv_width, lo_uv(1) - props.char_uv_height };

            // Low triangle
            xyz.push_back({ cursor(0), cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0), cursor(1), 0.0f }); uv.push_back({ hi_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });

            // High triangle
            xyz.push_back({ cursor(0), cursor(1), 0.0f }); uv.push_back({ lo_uv(0), lo_uv(1) });
            xyz.push_back({  hi_xy(0),  hi_xy(1), 0.0f }); uv.push_back({ hi_uv(0), hi_uv(1) });
            xyz.push_back({ cursor(0),  hi_xy(1), 0.0f }); uv.push_back({ lo_uv(0), hi_uv(1) });
        }
    }

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

    textures_binding const  textures_binding_(textures_binding_paths_map_type{
        { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, boost::filesystem::path(props.font_texture) }
        });

    texcoord_binding const  texcoord_binding_({
        { FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE, VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0 }
        });

    batch const  pbatch = batch(
        id.empty() ? id : "/generic/text/batch/" + id,
        buffers_binding_,
        textures_binding_,
        texcoord_binding_,
        effects_config{
            {}, // Light types.
            {}, // Lighting data types.
            {SHADER_DATA_OUTPUT_TYPE::DEFAULT},
            FOG_TYPE::NONE
            },
        draw_state(nullptr),
        modelspace(),
        skeleton_alignment(),
        batch_available_resources(
            buffers_binding_.get_buffers(),
            textures_binding_.empty() ? textures_binding::binding_map_type{} : textures_binding_.bindings_map(),
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

    return pbatch;
}


}
