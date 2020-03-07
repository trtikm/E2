#include <qtgl/effects_config.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/msgstream.hpp>

namespace qtgl {


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

namespace qtgl {


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

namespace qtgl {


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


}
