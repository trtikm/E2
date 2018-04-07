#ifndef QTGL_EFFECTS_CONFIG_HPP_INCLUDED
#   define QTGL_EFFECTS_CONFIG_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <map>
#   include <set>

namespace qtgl {


//namespace X {
//
//
//enum struct  EFFECT_DATA_SOURCE_TYPE : natural_8_bit
//{
//    UNIFORM     = 0,    // stored in vertex/fragment program's 'uniform' variable of any type, except 'sampler?D'
//    VARYING     = 1,    // passed to a vertex/fragment program via a varying 'in' variable.
//    TEXTURE     = 2,    // stored in a ?D texture (always per pixel data), i.e. in a uniform of a 'sampler?D' type
//};
//
//
////enum struct  EFFECT_DATA_TARGET_TYPE : natural_8_bit
////{
////    DEFAULT     = 0,    // pass data from a vertex/fragment program via 'out' variable(s) to another stage of graphic pipeline
////    TEXTURE     = 1,    // save data from a fragment program to a 2D texture, e.g. into a g-buffer of the deferred rendering
////};
//
//
//enum struct  EFFECT_DATA_TYPE : natural_8_bit
//{
//    FOG         = 0,    // RGB of the fog, alpha is ignored
//    AMBIENT     = 1,    // RGB of the ambient light, alpha is ignored
//    DIFFUSE     = 2,    // Material's diffuse RGB, and alpha represents the specular coefficient
//    SPECULAR    = 3,    // Material's specular RGB, and alpha represents the specular coefficient
//};
//
//
//enum struct  EFFECT_HOST_TYPE : natural_8_bit
//{
//    NONE                = 0,    // The effect is disabled.
//    VERTEX_PROGRAM      = 1,
//    FRAGMENT_PROGRAM    = 2,
//    DISTRIBUTED         = 3,    // Computation is started in vertex program and finished in fragment program
//};
//
//
//struct  effects_config
//{
//    struct fog_props
//    {
//        bool  m_enabled;
//        EFFECT_HOST_TYPE  m_host_type;
//    };
//
//    //effects_config(
//    //        lighting_model const&  lighting_model,
//    //        bool const  apply_gamma_correction,
//    //        bool const  apply_fog
//    //        );
//
//    //lighting_model const&  get_lighting_model() const { return m_lighting_model; }
//    //bool  apply_gamma_correction() const { return m_apply_gamma_correction; }
//    //bool  apply_fog() const { return m_apply_fog; }
//
//private:
//
//
//
//
//
//    //lighting_model  m_lighting_model;
//    //bool  m_apply_gamma_correction;
//    //bool  m_apply_fog;
//};
//
//
//
//}
//
//
//
//namespace Y {
//
//
//enum struct  DIFFUSE_COLOR_SOURCE_TYPE : natural_8_bit
//{
//    UNIFORM     = 0,    // stored in an 'uniform' variable of a vertex/fragment program
//    VARYING     = 1,    // passed to a vertex/fragment program via a varying 'in' variable.
//    TEXTURE     = 2,    // stored in a 2D texture (always per pixel data)
//    GBUFFER     = 3,    // 
//};
//
//
//enum struct  SPECULAR_COLOR_SOURCE_TYPE : natural_8_bit
//{
//    UNIFORM     = 0,    // uniform variable in a vertex/fragment program
//    BUFFER      = 1,    // there is a diffuse colour for each vertex
//    TEXTURE     = 2,    // specular colours are stored in a specular texture
//};
//
//
//enum struct  SPECULAR_COEF_SOURCE_TYPE : natural_8_bit
//{
//    UNIFORM     = 0,    // uniform variable in a vertex/fragment program
//    TEXTURE     = 1,    // coefs are stored in a specular texture in the alpha channel.
//};
//
//
//enum struct  NORMAL_SOURCE_TYPE : natural_8_bit
//{
//    BUFFER      = 0,    // there is a normal for each vertex
//    TEXTURE     = 1,    // normals are in a normals map
//};
//
//
//enum struct  LIGHTING_MODEL_TYPE : natural_8_bit
//{
//    NONE            = 0,    // just applies diffuse colour of a vertex
//    AMBIENT         = 1,    // adds ambient colour (in uniform variable) to diffuse colour of vertex
//    DIRECTIONAL     = 2,    // adds ambient colour to modulated diffuse colour of vertex by angle towards the directional light
//    SPECULAR        = 3,    // Phong lighting model, for a single directional light
//};
//
//
//enum struct  LIGHTING_HOST_TYPE : natural_8_bit
//{
//    VERTEX_PROGRAM      = 0,
//    FRAGMENT_PROGRAM    = 1,
//    DISTRIBUTED         = 2,    // Computation is started in vertex program and finished in fragment program
//};
//
//
//enum struct  FOG_HOST_TYPE : natural_8_bit
//{
//    VERTEX_PROGRAM      = 0,
//    FRAGMENT_PROGRAM    = 1,
//    DISTRIBUTED         = 2,    // Computation is started in vertex program and finished in fragment program
//};
//
//
//
//struct  effects_configX
//{
//
//private:
//    DIFFUSE_COLOR_SOURCE_TYPE  m_diffuse_colour_source;
//    SPECULAR_COLOR_SOURCE_TYPE  m_specular_colour_source;
//    SPECULAR_COEF_SOURCE_TYPE  m_specular_coef_source;
//    NORMAL_SOURCE_TYPE  m_normal_source;
//    LIGHTING_MODEL_TYPE  m_lighting_model;
//    LIGHTING_HOST_TYPE  m_lighting_host;
//    FOG_HOST_TYPE  m_fog_host;
//};
//
//
//}
//
//
//
//
//enum struct  LIGHTING_TYPE : natural_8_bit
//{
//    DIFFUSE_COLOR_UNIFORM                           = 0,
//    DIFFUSE_COLOR_BUFFER                            = 1,
//    DIFFUSE_COLOR_TEXTURE                           = 1,
//    DIFFUSE_LIGHTING_IN_VERTEX_SHADER               = 2,
//    DIFFUSE_LIGHTING_IN_FRAGMENT_SHADER             = 3,
//    DIFFUSE_AND_SPEULAR_LIGHTING_IN_VERTEX_SHADER   = 4,
//    DIFFUSE_AND_SPEULAR_LIGHTING_IN_FRAGMENT_SHADER = 5,
//};
//
//inline LIGHTING_TYPE  minimal_lighting_type()
//{
//    return LIGHTING_TYPE::DIFFUSE_COLOR_UNIFORM;
//}
//
//inline LIGHTING_TYPE  maximal_lighting_type()
//{
//    return LIGHTING_TYPE::DIFFUSE_AND_SPEULAR_LIGHTING_IN_FRAGMENT_SHADER;
//}
//
//bool  is_minimal_lighting_type(LIGHTING_TYPE const  type);
//bool  is_maximal_lighting_type(LIGHTING_TYPE const  type);
//LIGHTING_TYPE  next_lighting_type(LIGHTING_TYPE const  type);
//LIGHTING_TYPE  prev_lighting_type(LIGHTING_TYPE const  type);
//
//
//struct  lighting_model
//{
//    lighting_model(
//            LIGHTING_TYPE const  lighting_type,
//            bool const  specular_map_enabled,
//            bool const  normal_map_enabled
//            );
//
//    LIGHTING_TYPE  get_lighting_type() const { return m_lighting_type; }
//    bool  is_specular_map_enabled() const { return m_specular_map_enabled; }
//    bool  is_normal_map_enabled() const { return m_normal_map_enabled; }
//
//private:
//    LIGHTING_TYPE  m_lighting_type;
//    bool  m_specular_map_enabled;
//    bool  m_normal_map_enabled;
//};
//
//
//bool  operator<(lighting_model const&  left, lighting_model const&  right);
//bool  operator==(lighting_model const&  left, lighting_model const&  right);
//
//inline lighting_model  minimal_lighting_model()
//{
//    return { minimal_lighting_type(), false, false };
//}
//
//inline lighting_model  maximal_lighting_model()
//{
//    return{ maximal_lighting_type(), true, true };
//}
//
//bool  is_minimal_lighting_model(lighting_model const&  model);
//bool  is_maximal_lighting_model(lighting_model const&  model);
//lighting_model  maximal_lighting_model();
//lighting_model  next_lighting_model(lighting_model const&  model);
//lighting_model  prev_lighting_model(lighting_model const&  model);
//
//
//struct  effects_config
//{
//    effects_config(
//            lighting_model const&  lighting_model,
//            bool const  apply_gamma_correction,
//            bool const  apply_fog
//            );
//
//    lighting_model const&  get_lighting_model() const { return m_lighting_model; }
//    bool  apply_gamma_correction() const { return m_apply_gamma_correction; }
//    bool  apply_fog() const { return m_apply_fog; }
//
//private:
//    lighting_model  m_lighting_model;
//    bool  m_apply_gamma_correction;
//    bool  m_apply_fog;
//};
//
//
//bool  operator<(effects_config const&  left, effects_config const&  right);
//bool  operator==(effects_config const&  left, effects_config const&  right);
//
//inline effects_config  minimal_effects_config()
//{
//    return { minimal_lighting_model(), false, false };
//}
//
//inline effects_config  maximal_effects_config()
//{
//    return{ maximal_lighting_model(), true, true };
//}
//
//bool  is_minimal_effects_config(effects_config const&  config);
//bool  is_maximal_effects_config(effects_config const&  config);
//effects_config  next_effects_config(effects_config const&  config);
//effects_config  prev_effects_config(effects_config const&  config);
//
//
//std::vector<effects_config> const&  get_effects_configs();


enum struct  SHADER_DATA_INPUT_TYPE : natural_8_bit
{
    UNIFORM     = 0,    // stored in vertex/fragment program's 'uniform' variable of any type, except 'sampler?D'
    BUFFER      = 1,    // passed to a vertex/fragment program via a varying 'in' variable.
    TEXTURE     = 2,    // stored in a ?D texture (always per fragment data), i.e. in a uniform of a 'sampler?D' type
};


enum struct  SHADER_DATA_OUTPUT_TYPE : natural_8_bit
{
    DEFAULT             = 0,    // pass final colour from a fragment program via 'out' variable to the screen
    TEXTURE_POSITION    = 1,    // save data to a 2D texture, e.g. into a g-buffer of the deferred rendering
    TEXTURE_NORMAL      = 2,    // save data to a 2D texture, e.g. into a g-buffer of the deferred rendering
    TEXTURE_DIFFUSE     = 3,    // save data to a 2D texture, e.g. into a g-buffer of the deferred rendering
    TEXTURE_SPECULAR    = 4,    // save data to a 2D texture, e.g. into a g-buffer of the deferred rendering
};


enum struct  LIGHT_TYPE : natural_8_bit
{
    AMBIENT     = 0,
    DIRECTIONAL = 1,
};


enum struct  LIGHTING_DATA_TYPE : natural_8_bit
{
    POSITION    = 0,
    NORMAL      = 1,
    DIFFUSE     = 2,
    SPECULAR    = 3,
};


struct  effects_config
{
    using  light_types = std::set<LIGHT_TYPE>;
    using  lighting_data_types = std::map<LIGHTING_DATA_TYPE, SHADER_DATA_INPUT_TYPE>;
    using  shader_output_types = std::set<SHADER_DATA_OUTPUT_TYPE>;

    effects_config(
            light_types const&  light_types_,
            lighting_data_types const&  lighting_data_types_,
            shader_output_types const&  shader_output_types_
            );

    light_types const&  get_light_types() const { return m_light_types; }
    lighting_data_types const&  get_lighting_data_types() const { return m_lighting_data_types; }
    shader_output_types const&  get_shader_output_types() const { return m_shader_output_types; }

    light_types&  get_light_types() { return m_light_types; }
    lighting_data_types&  get_lighting_data_types() { return m_lighting_data_types; }
    shader_output_types&  get_shader_output_types() { return m_shader_output_types; }

private:

    light_types  m_light_types;
    lighting_data_types  m_lighting_data_types;
    shader_output_types  m_shader_output_types;
};


bool  operator==(effects_config const&  left, effects_config const&  right);


}

#endif
