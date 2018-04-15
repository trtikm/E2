#include <qtgl/effects_config.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace qtgl {


effects_config::effects_config(
        light_types const&  light_types_,
        lighting_data_types const&  lighting_data_types_,
        shader_output_types const&  shader_output_types_,
        bool  use_fog
        )
    : m_light_types(light_types_)
    , m_lighting_data_types(lighting_data_types_)
    , m_shader_output_types(shader_output_types_)
    , m_use_fog(use_fog)
{}


bool  operator==(effects_config const&  left, effects_config const&  right)
{
    return left.get_light_types() == right.get_light_types() &&
           left.get_lighting_data_types() == right.get_lighting_data_types() &&
           left.get_shader_output_types() == right.get_shader_output_types() &&
           left.use_fog() == right.use_fog();
}


}
