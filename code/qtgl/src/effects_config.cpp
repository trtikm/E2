#include <qtgl/effects_config.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

namespace qtgl {


//bool  is_minimal_lighting_type(LIGHTING_TYPE const  type)
//{
//    return type == minimal_lighting_type();
//}
//
//bool  is_maximal_lighting_type(LIGHTING_TYPE const  type)
//{
//    return type == maximal_lighting_type();
//}
//
//LIGHTING_TYPE  next_lighting_type(LIGHTING_TYPE const  type)
//{
//    return is_maximal_lighting_type(type) ? type : static_cast<LIGHTING_TYPE>(static_cast<natural_8_bit>(type) + 1U) ;
//}
//
//LIGHTING_TYPE  prev_lighting_type(LIGHTING_TYPE const  type)
//{
//    return is_minimal_lighting_type(type) ? type : static_cast<LIGHTING_TYPE>(static_cast<natural_8_bit>(type) - 1U);
//}
//
//
//static bool  is_consistent_lighting_model(
//        LIGHTING_TYPE const  lighting_type,
//        bool const  specular_map_enabled,
//        bool const  normal_map_enabled
//        )
//{
//    return (!(lighting_type < LIGHTING_TYPE::DIFFUSE_LIGHTING_IN_VERTEX_SHADER) || normal_map_enabled == false) &&
//           (!(lighting_type < LIGHTING_TYPE::DIFFUSE_AND_SPEULAR_LIGHTING_IN_VERTEX_SHADER) || specular_map_enabled == false);
//}
//
//
//lighting_model::lighting_model(
//        LIGHTING_TYPE const  lighting_type,
//        bool const  specular_map_enabled,
//        bool const  normal_map_enabled
//        )
//    : m_lighting_type(lighting_type)
//    , m_specular_map_enabled(specular_map_enabled)
//    , m_normal_map_enabled(normal_map_enabled)
//{
//    ASSUMPTION(is_consistent_lighting_model(m_lighting_type, m_normal_map_enabled, m_specular_map_enabled));
//}
//
//
//bool  operator<(lighting_model const&  left, lighting_model const&  right)
//{
//    if (left.get_lighting_type() < right.get_lighting_type())
//        return true;
//    if (left.get_lighting_type() == right.get_lighting_type())
//    {
//        if (!left.is_specular_map_enabled() && right.is_specular_map_enabled())
//            return true;
//        if (left.is_specular_map_enabled() == right.is_specular_map_enabled())
//            return !left.is_normal_map_enabled() && right.is_normal_map_enabled();
//    }
//
//    return false;
//}
//
//
//bool  operator==(lighting_model const&  left, lighting_model const&  right)
//{
//    return left.get_lighting_type() == right.get_lighting_type() &&
//           left.is_specular_map_enabled() == right.is_specular_map_enabled() &&
//           left.is_normal_map_enabled() == right.is_normal_map_enabled();
//}
//
//
//bool  is_minimal_lighting_model(lighting_model const&  model)
//{
//    return !model.is_normal_map_enabled() &&
//           !model.is_specular_map_enabled() &&
//           is_minimal_lighting_type(model.get_lighting_type());
//}
//
//bool  is_maximal_lighting_model(lighting_model const&  model)
//{
//    return model.is_normal_map_enabled() &&
//           model.is_specular_map_enabled() &&
//           is_maximal_lighting_type(model.get_lighting_type());
//}
//
//lighting_model  next_lighting_model(lighting_model const&  model)
//{
//    LIGHTING_TYPE  lighting_type = model.get_lighting_type();
//    bool  specular_map_enabled = model.is_specular_map_enabled();
//    bool  normal_map_enabled = model.is_normal_map_enabled();
//    do
//    {
//        if (!normal_map_enabled)
//            normal_map_enabled = true;
//        else if (!specular_map_enabled)
//        {
//            specular_map_enabled = true;
//            normal_map_enabled = false;
//        }
//        else if (!is_maximal_lighting_type(lighting_type))
//        {
//            lighting_type = next_lighting_type(lighting_type);
//            specular_map_enabled = false;
//            normal_map_enabled = false;
//        }
//    }
//    while(!is_consistent_lighting_model(lighting_type, specular_map_enabled, normal_map_enabled));
//    lighting_model const  next_model{ lighting_type, specular_map_enabled, normal_map_enabled };
//    INVARIANT(model < next_model || model == next_model);
//    return next_model;
//}
//
//lighting_model  prev_lighting_model(lighting_model const&  model)
//{
//    LIGHTING_TYPE  lighting_type = model.get_lighting_type();
//    bool  specular_map_enabled = model.is_specular_map_enabled();
//    bool  normal_map_enabled = model.is_normal_map_enabled();
//    do
//    {
//        if (normal_map_enabled)
//            normal_map_enabled = false;
//        else if (specular_map_enabled)
//        {
//            specular_map_enabled = false;
//            normal_map_enabled = true;
//        }
//        else if (!is_minimal_lighting_type(lighting_type))
//        {
//            lighting_type = prev_lighting_type(lighting_type);
//            specular_map_enabled = true;
//            normal_map_enabled = true;
//        }
//    }
//    while(!is_consistent_lighting_model(lighting_type, specular_map_enabled, normal_map_enabled));
//    lighting_model const  prev_model{ lighting_type, specular_map_enabled, normal_map_enabled };
//    INVARIANT(prev_model < model || prev_model == model);
//    return prev_model;
//}
//
//
//effects_config::effects_config(
//        lighting_model const&  lighting_model,
//        bool const  apply_gamma_correction,
//        bool const  apply_fog
//        )
//    : m_lighting_model(lighting_model)
//    , m_apply_gamma_correction(apply_gamma_correction)
//    , m_apply_fog(apply_fog)
//{}
//
//
//bool  operator<(effects_config const&  left, effects_config const&  right)
//{
//    if (left.get_lighting_model() < right.get_lighting_model())
//        return true;
//    if (left.get_lighting_model() == right.get_lighting_model())
//    {
//        if (!left.apply_gamma_correction() && right.apply_gamma_correction())
//            return true;
//        if (left.apply_gamma_correction() == right.apply_gamma_correction())
//            return !left.apply_fog() && right.apply_fog();
//    }
//    return false;
//}
//
//
//bool  operator==(effects_config const&  left, effects_config const&  right)
//{
//    return left.get_lighting_model() == right.get_lighting_model() &&
//           left.apply_fog() == right.apply_fog() &&
//           left.apply_gamma_correction() == right.apply_gamma_correction();
//}
//
//
//bool  is_minimal_effects_config(effects_config const&  config)
//{
//    return !config.apply_gamma_correction() &&
//           !config.apply_fog() &&
//           is_minimal_lighting_model(config.get_lighting_model());
//}
//
//bool  is_maximal_effects_config(effects_config const&  config)
//{
//    return config.apply_gamma_correction() &&
//           config.apply_fog() &&
//           is_maximal_lighting_model(config.get_lighting_model());
//}
//
//effects_config  next_effects_config(effects_config const&  config)
//{
//    if (!config.apply_fog())
//        return { config.get_lighting_model(), config.apply_gamma_correction(), true };
//    if (!config.apply_gamma_correction())
//        return{ config.get_lighting_model(), true, false };
//    if (is_maximal_lighting_model(config.get_lighting_model()))
//        return config;
//    return { next_lighting_model(config.get_lighting_model()), false, false };
//}
//
//effects_config  prev_effects_config(effects_config const&  config)
//{
//    if (config.apply_fog())
//        return{ config.get_lighting_model(), config.apply_gamma_correction(), false };
//    if (config.apply_gamma_correction())
//        return{ config.get_lighting_model(), false, true };
//    if (is_minimal_lighting_model(config.get_lighting_model()))
//        return config;
//    return{ prev_lighting_model(config.get_lighting_model()), true, true };
//}
//
//
//std::vector<effects_config> const&  get_effects_configs()
//{
//    static std::vector<effects_config> const  configs(
//        []() -> std::vector<effects_config> {
//            std::vector<effects_config> result{ minimal_effects_config() };
//            do
//                result.push_back(next_effects_config(result.back()));
//            while (!is_maximal_effects_config(result.back()));
//            return result;
//            }()
//        );
//    return configs;
//}


effects_config::effects_config(
        light_types const&  light_types_,
        lighting_data_types const&  lighting_data_types_,
        shader_output_types const&  shader_output_types_
        )
    : m_light_types(light_types_)
    , m_lighting_data_types(lighting_data_types_)
    , m_shader_output_types(shader_output_types_)
{}


bool  operator==(effects_config const&  left, effects_config const&  right)
{
    return left.get_light_types() == right.get_light_types() &&
           left.get_lighting_data_types() == right.get_lighting_data_types() &&
           left.get_shader_output_types() == right.get_shader_output_types();
}


}
