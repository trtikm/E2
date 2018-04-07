#ifndef QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED
#   define QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <unordered_set>
#   include <unordered_map>
#   include <string>

namespace qtgl {


enum struct VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    BINDING_IN_POSITION             = 0,
    BINDING_IN_DIFFUSE              = 1,
    BINDING_IN_SPECULAR             = 2,
    BINDING_IN_NORMAL               = 3,
    BINDING_IN_INDICES_OF_MATRICES  = 4,
    BINDING_IN_WEIGHTS_OF_MATRICES  = 5,
    BINDING_IN_TEXCOORD0            = 6,
    BINDING_IN_TEXCOORD1            = 7,
    BINDING_IN_TEXCOORD2            = 8,
    BINDING_IN_TEXCOORD3            = 9,
    BINDING_IN_TEXCOORD4            = 10,
    BINDING_IN_TEXCOORD5            = 11,
    BINDING_IN_TEXCOORD6            = 12,
    BINDING_IN_TEXCOORD7            = 13,
    BINDING_IN_TEXCOORD8            = 14,
    BINDING_IN_TEXCOORD9            = 15,
};

inline natural_32_bit  value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);

inline VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  get_texcoord_binding_location(natural_8_bit const  texcood_index)
{
    return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION(
                value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0) + texcood_index
                );
}

inline constexpr VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  min_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION() noexcept
{ return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION; }
inline constexpr VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  max_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION() noexcept
{ return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_WEIGHTS_OF_MATRICES; }
inline constexpr natural_8_bit  num_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATIONs() noexcept
{ return static_cast<natural_8_bit>(max_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION()) -
         static_cast<natural_8_bit>(min_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION()) +
         1U; }

enum struct VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    BINDING_OUT_POSITION            = 0,
    BINDING_OUT_DIFFUSE             = 1,
    BINDING_OUT_SPECULAR            = 2,
    BINDING_OUT_NORMAL              = 3,
    BINDING_OUT_TEXCOORD0           = 4,
    BINDING_OUT_TEXCOORD1           = 5,
    BINDING_OUT_TEXCOORD2           = 6,
    BINDING_OUT_TEXCOORD3           = 7,
    BINDING_OUT_TEXCOORD4           = 8,
    BINDING_OUT_TEXCOORD5           = 9,
    BINDING_OUT_TEXCOORD6           = 10,
    BINDING_OUT_TEXCOORD7           = 11,
    BINDING_OUT_TEXCOORD8           = 12,
    BINDING_OUT_TEXCOORD9           = 13,
};

inline natural_32_bit  value(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION const  location);


enum struct VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME : natural_8_bit
{
    COLOUR_ALPHA                    = 0,
    DIFFUSE_COLOUR                  = 1,
    TRANSFORM_MATRIX_TRANSPOSED     = 2,
    NUM_MATRICES_PER_VERTEX         = 3,
};

inline natural_32_bit  value(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  name)
{ return static_cast<natural_8_bit>(name); }
std::string  name(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
std::string  uniform_name(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  to_symbolic_uniform_name_of_vertex_shader(std::string  name);
inline constexpr natural_32_bit  uniform_max_transform_matrices() { return 64U; }

inline constexpr VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  min_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::COLOUR_ALPHA; }
inline constexpr VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  max_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::NUM_MATRICES_PER_VERTEX; }
inline constexpr natural_8_bit  num_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAMEs()
{ return static_cast<natural_8_bit>(max_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()) -
         static_cast<natural_8_bit>(min_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()) +
         1U; }


enum struct FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    BINDING_IN_POSITION             = 0,
    BINDING_IN_DIFFUSE              = 1,
    BINDING_IN_SPECULAR             = 2,
    BINDING_IN_NORMAL               = 3,
    BINDING_IN_TEXCOORD0            = 4,
    BINDING_IN_TEXCOORD1            = 5,
    BINDING_IN_TEXCOORD2            = 6,
    BINDING_IN_TEXCOORD3            = 7,
    BINDING_IN_TEXCOORD4            = 8,
    BINDING_IN_TEXCOORD5            = 9,
    BINDING_IN_TEXCOORD6            = 10,
    BINDING_IN_TEXCOORD7            = 11,
    BINDING_IN_TEXCOORD8            = 12,
    BINDING_IN_TEXCOORD9            = 13,
};

inline natural_32_bit  value(FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);


enum struct FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION : natural_8_bit
{
    BINDING_OUT_COLOUR              = 0,
    BINDING_OUT_TEXTURE_POSITION    = 1,
    BINDING_OUT_TEXTURE_NORMAL      = 2,
    BINDING_OUT_TEXTURE_DIFFUSE     = 3,
    BINDING_OUT_TEXTURE_SPECULAR    = 4,
};

inline natural_32_bit  value(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const  location);


enum struct FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME : natural_8_bit
{
    TEXTURE_SAMPLER_DIFFUSE         = 0,
    TEXTURE_SAMPLER_SPECULAR        = 1,
    TEXTURE_SAMPLER_NORMAL          = 2,
    TEXTURE_SAMPLER_POSITION        = 3,

    FOG_COLOUR                      = 4,
    AMBIENT_COLOUR                  = 5,
    DIFFUSE_COLOUR                  = 6,
    SPECULAR_COLOUR                 = 7,

    DIRECTIONAL_LIGHT_POSITION      = 8,
    DIRECTIONAL_LIGHT_COLOUR        = 9,
};

inline natural_32_bit  value(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  binding)
{ return static_cast<natural_8_bit>(binding); }
std::string  name(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform_symbolic_name);
std::string  uniform_name(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform_symbolic_name);
bool  is_texture_sampler(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform_symbolic_name);
FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  to_symbolic_uniform_name_of_fragment_shader(std::string  name);

inline constexpr FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  min_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE; }
inline constexpr FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  max_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::DIRECTIONAL_LIGHT_COLOUR; }
inline constexpr natural_8_bit  num_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAMEs() noexcept
{ return static_cast<natural_8_bit>(max_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()) -
         static_cast<natural_8_bit>(min_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()) +
         1U; }


bool  compatible(std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const& vertex_program_output,
                 std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const& fragment_program_input);


using  texcoord_binding = std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME,VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>;


}

#endif
