#ifndef QTGL_EFFECTS_CONFIG_HPP_INCLUDED
#   define QTGL_EFFECTS_CONFIG_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/async_resource_load.hpp>
#   include <map>
#   include <set>
#   include <string>

namespace qtgl {


enum struct SHADER_PROGRAM_TYPE : natural_32_bit
{
    VERTEX      = 0,
    FRAGMENT    = 1
};


enum struct  SHADER_DATA_INPUT_TYPE : natural_8_bit
{
    UNIFORM     = 0,    // stored in vertex/fragment program's 'uniform' variable of any type, except 'sampler?D'
    BUFFER      = 1,    // passed to a vertex/fragment program via a varying 'in' variable.
    TEXTURE     = 2,    // stored in a ?D texture (always per fragment data), i.e. in a uniform of a 'sampler?D' type
    INSTANCE    = 3,    // passed to a vertex/fragment program via a varying 'in' variable; bata changes only per instance.
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
    DIRECTION   = 0,
    NORMAL      = 1,
    DIFFUSE     = 2,
    SPECULAR    = 3,
};


enum struct FOG_TYPE : natural_8_bit
{
    NONE            = 0,    // No fog (disabled)
    INTERPOLATED    = 1,    // Computer in vertex shader (so, fragment shader then only uses the interpolated the fog colour). 
    DETAILED        = 2,    // Computed in fragment shader.
};

std::string  name(FOG_TYPE const  fog_type_);


}

namespace qtgl {


struct  effects_config_data
{
    using  light_types = std::set<LIGHT_TYPE>;
    using  lighting_data_types = std::map<LIGHTING_DATA_TYPE, SHADER_DATA_INPUT_TYPE>;
    using  shader_output_types = std::set<SHADER_DATA_OUTPUT_TYPE>;

    effects_config_data()
        : m_light_types()
        , m_lighting_data_types()
        , m_lighting_algo_location(SHADER_PROGRAM_TYPE::VERTEX)
        , m_shader_output_types()
        , m_fog_type(FOG_TYPE::NONE)
        , m_fog_algo_location(SHADER_PROGRAM_TYPE::VERTEX)
    {}

    effects_config_data(
            async::finalise_load_on_destroy_ptr,
            light_types const&  light_types_,
            lighting_data_types const&  lighting_data_types_,
            SHADER_PROGRAM_TYPE const  lighting_algo_location_,
            shader_output_types const&  shader_output_types_,
            FOG_TYPE const  fog_type_,
            SHADER_PROGRAM_TYPE const  fog_algo_location_
            );

    light_types const&  get_light_types() const { return m_light_types; }
    light_types&  get_light_types() { return m_light_types; }

    lighting_data_types const&  get_lighting_data_types() const { return m_lighting_data_types; }
    lighting_data_types&  get_lighting_data_types() { return m_lighting_data_types; }

    shader_output_types const&  get_shader_output_types() const { return m_shader_output_types; }
    shader_output_types& get_shader_output_types() { return m_shader_output_types; }

    SHADER_PROGRAM_TYPE  get_lighting_algo_location() const { return m_lighting_algo_location; }
    void  set_lighting_algo_location(SHADER_PROGRAM_TYPE const  location) { m_lighting_algo_location = location; }


    FOG_TYPE  get_fog_type() const { return m_fog_type; }
    void  set_fog_type(FOG_TYPE const  type) { m_fog_type = type; }

    SHADER_PROGRAM_TYPE  get_fog_algo_location() const { return m_fog_algo_location; }
    void  set_fog_algo_location(SHADER_PROGRAM_TYPE const  location) { m_fog_algo_location = location; }

private:

    light_types  m_light_types;
    lighting_data_types  m_lighting_data_types;
    SHADER_PROGRAM_TYPE  m_lighting_algo_location;
    shader_output_types  m_shader_output_types;
    FOG_TYPE  m_fog_type;
    SHADER_PROGRAM_TYPE  m_fog_algo_location;
};


bool  operator==(effects_config_data const&  left, effects_config_data const&  right);
inline bool  operator!=(effects_config_data const&  left, effects_config_data const&  right) { return !(left == right); }


}

namespace qtgl {


struct  effects_config : public async::resource_accessor<effects_config_data>
{
    using  super = async::resource_accessor<effects_config_data>;

    using  light_types = effects_config_data::light_types;
    using  lighting_data_types = effects_config_data::lighting_data_types;
    using  shader_output_types = effects_config_data::shader_output_types;

    effects_config()
        : super()
    {}

    effects_config(
            async::finalise_load_on_destroy_ptr const  parent_finaliser,
            light_types const&  light_types_,
            lighting_data_types const&  lighting_data_types_,
            SHADER_PROGRAM_TYPE const  lighting_algo_location_,
            shader_output_types const&  shader_output_types_,
            FOG_TYPE const  fog_type_,
            SHADER_PROGRAM_TYPE const  fog_algo_location_
            )
        : super(
            async::key_type(
                "qtgl::effects_config",
                compute_generic_unique_id(
                    light_types_,
                    lighting_data_types_,
                    lighting_algo_location_,
                    shader_output_types_,
                    fog_type_,
                    fog_algo_location_
                    )
                ),
            parent_finaliser,
            light_types_,
            lighting_data_types_,
            lighting_algo_location_,
            shader_output_types_,
            fog_type_,
            fog_algo_location_
            )
    {}

    static inline effects_config  make(effects_config_data const&  data)
    {
        return effects_config(
                    nullptr,
                    data.get_light_types(),
                    data.get_lighting_data_types(),
                    data.get_lighting_algo_location(),
                    data.get_shader_output_types(),
                    data.get_fog_type(),
                    data.get_fog_algo_location()
                    );
    }

    light_types const&  get_light_types() const { return resource().get_light_types(); }
    lighting_data_types const&  get_lighting_data_types() const { return resource().get_lighting_data_types(); }
    shader_output_types const&  get_shader_output_types() const { return resource().get_shader_output_types(); }
    SHADER_PROGRAM_TYPE  get_lighting_algo_location() const { return resource().get_lighting_algo_location(); }
    FOG_TYPE  get_fog_type() const { return resource().get_fog_type(); }
    SHADER_PROGRAM_TYPE  get_fog_algo_location() const { return resource().get_fog_algo_location(); }

    bool  operator==(super const&  other) const override { return resource() == other.resource(); }

private:

    using super::resource; // use 'super::resource_const' to directly access the resource object.

    static std::string  compute_generic_unique_id(
            light_types const&  light_types_,
            lighting_data_types const&  lighting_data_types_,
            SHADER_PROGRAM_TYPE const  lighting_algo_location_,
            shader_output_types const&  shader_output_types_,
            FOG_TYPE const  fog_type_,
            SHADER_PROGRAM_TYPE const  fog_algo_location_
            );
};


inline bool  operator==(effects_config const  left, effects_config const  right) { return left.operator==(right); }
inline bool  operator!=(effects_config const  left, effects_config const  right) { return !(left == right); }


}

#endif
