#include <qtgl/effects_config.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

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


effects_config::effects_config(
        light_types const&  light_types_,
        lighting_data_types const&  lighting_data_types_,
        shader_output_types const&  shader_output_types_,
        FOG_TYPE const  fog_type_
        )
    : m_light_types(light_types_)
    , m_lighting_data_types(lighting_data_types_)
    , m_shader_output_types(shader_output_types_)
    , m_fog_type(fog_type_)
{}


bool  operator==(effects_config const&  left, effects_config const&  right)
{
    return left.get_light_types() == right.get_light_types() &&
           left.get_lighting_data_types() == right.get_lighting_data_types() &&
           left.get_shader_output_types() == right.get_shader_output_types() &&
           left.get_fog_type() == right.get_fog_type();
}


}
