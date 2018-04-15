#ifndef QTGL_EFFECTS_CONFIG_HPP_INCLUDED
#   define QTGL_EFFECTS_CONFIG_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <map>
#   include <set>

namespace qtgl {


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
            shader_output_types const&  shader_output_types_,
            bool  use_fog
            );

    light_types const&  get_light_types() const { return m_light_types; }
    lighting_data_types const&  get_lighting_data_types() const { return m_lighting_data_types; }
    shader_output_types const&  get_shader_output_types() const { return m_shader_output_types; }

    light_types&  get_light_types() { return m_light_types; }
    lighting_data_types&  get_lighting_data_types() { return m_lighting_data_types; }
    shader_output_types&  get_shader_output_types() { return m_shader_output_types; }

    bool  use_fog() const { return m_use_fog; }

private:

    light_types  m_light_types;
    lighting_data_types  m_lighting_data_types;
    shader_output_types  m_shader_output_types;
    bool  m_use_fog;
};


bool  operator==(effects_config const&  left, effects_config const&  right);


}

#endif
