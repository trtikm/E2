#include <gfx/effects_config.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/msgstream.hpp>

namespace gfx {


std::string  name(FOG_TYPE const  fog_type_)
{
    switch(fog_type_)
    {
    case FOG_TYPE::NONE: return "NONE";
    case FOG_TYPE::INTERPOLATED: return "INTERPOLATED";
    case FOG_TYPE::DETAILED: return "DETAILED";
    default: UNREACHABLE();
    }
}


FOG_TYPE  fog_type_from_name(std::string const&  fog_type_name)
{
    if (fog_type_name == name(FOG_TYPE::NONE))
        return FOG_TYPE::NONE;
    if (fog_type_name == name(FOG_TYPE::INTERPOLATED))
        return FOG_TYPE::INTERPOLATED;
    if (fog_type_name == name(FOG_TYPE::DETAILED))
        return FOG_TYPE::DETAILED;
    UNREACHABLE();
}


}

namespace gfx {


effects_config_data::effects_config_data(
        async::finalise_load_on_destroy_ptr,
        light_types const&  light_types_,
        lighting_data_types const&  lighting_data_types_,
        SHADER_PROGRAM_TYPE const  lighting_algo_location_,
        shader_output_types const&  shader_output_types_,
        FOG_TYPE const  fog_type_,
        SHADER_PROGRAM_TYPE const  fog_algo_location_
        )
    : m_light_types(light_types_)
    , m_lighting_data_types(lighting_data_types_)
    , m_lighting_algo_location(lighting_algo_location_)
    , m_shader_output_types(shader_output_types_)
    , m_fog_type(fog_type_)
    , m_fog_algo_location(fog_algo_location_)
{}


bool  operator==(effects_config_data const&  left, effects_config_data const&  right)
{
    return left.get_light_types() == right.get_light_types() &&
           left.get_lighting_data_types() == right.get_lighting_data_types() &&
           left.get_lighting_algo_location() == right.get_lighting_algo_location() &&
           left.get_shader_output_types() == right.get_shader_output_types() &&
           left.get_fog_type() == right.get_fog_type() &&
           left.get_fog_algo_location() == right.get_fog_algo_location();
}


}

namespace gfx {


std::string  effects_config::compute_generic_unique_id(
        light_types const&  light_types_,
        lighting_data_types const&  lighting_data_types_,
        SHADER_PROGRAM_TYPE const  lighting_algo_location_,
        shader_output_types const&  shader_output_types_,
        FOG_TYPE const  fog_type_,
        SHADER_PROGRAM_TYPE const  fog_algo_location_
        )
{
    msgstream  mstr;
    mstr << "LT{";
    for (auto x : light_types_) mstr << (int)x << ",";
    mstr << "};LDT{";
    for (auto x : lighting_data_types_) mstr << (int)x.first << ":" << (int)x.second << ",";
    mstr << "};LAL{" << (int)lighting_algo_location_ << "};SOT{";
    for (auto x : shader_output_types_) mstr << (int)x << ",";
    mstr << "};FT{" << (int)fog_type_ << "}FAL{" << (int)fog_algo_location_ << "}";
    return mstr.get();
}



effects_config  default_effects_config()
{
    return
        {
            nullptr,
            effects_config::light_types{
                LIGHT_TYPE::AMBIENT,
                LIGHT_TYPE::DIRECTIONAL,
                },
            effects_config::lighting_data_types{
                { LIGHTING_DATA_TYPE::DIRECTION, SHADER_DATA_INPUT_TYPE::UNIFORM },
                { LIGHTING_DATA_TYPE::NORMAL, SHADER_DATA_INPUT_TYPE::TEXTURE },
                { LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE },
                //{ LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::UNIFORM },
                { LIGHTING_DATA_TYPE::SPECULAR, SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            SHADER_PROGRAM_TYPE::VERTEX,
            effects_config::shader_output_types{
                SHADER_DATA_OUTPUT_TYPE::DEFAULT
                },
            FOG_TYPE::NONE,
            SHADER_PROGRAM_TYPE::VERTEX
        };
}


}
