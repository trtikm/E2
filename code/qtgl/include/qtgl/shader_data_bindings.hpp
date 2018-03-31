#ifndef QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED
#   define QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <unordered_set>
#   include <string>

namespace qtgl {


enum struct vertex_shader_input_buffer_binding_location : natural_8_bit
{
    BINDING_IN_POSITION             = 0,
    BINDING_IN_COLOUR               = 1,
    BINDING_IN_NORMAL               = 2,
    BINDING_IN_TEXCOORD0            = 3,
    BINDING_IN_TEXCOORD1            = 4,
    BINDING_IN_TEXCOORD2            = 5,
    BINDING_IN_TEXCOORD3            = 6,
    BINDING_IN_TEXCOORD4            = 7,
    BINDING_IN_TEXCOORD5            = 8,
    BINDING_IN_TEXCOORD6            = 9,
    BINDING_IN_TEXCOORD7            = 10,
    BINDING_IN_TEXCOORD8            = 11,
    BINDING_IN_TEXCOORD9            = 12,
    BINDING_IN_INDICES_OF_MATRICES  = 13,
    BINDING_IN_WEIGHTS_OF_MATRICES  = 14
};

inline natural_32_bit  value(vertex_shader_input_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(vertex_shader_input_buffer_binding_location const  location);

inline constexpr vertex_shader_input_buffer_binding_location  min_vertex_shader_input_buffer_binding_location() noexcept
{ return vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION; }
inline constexpr vertex_shader_input_buffer_binding_location  max_vertex_shader_input_buffer_binding_location() noexcept
{ return vertex_shader_input_buffer_binding_location::BINDING_IN_WEIGHTS_OF_MATRICES; }
inline constexpr natural_8_bit  num_vertex_shader_input_buffer_bindings() noexcept
{ return static_cast<natural_8_bit>(max_vertex_shader_input_buffer_binding_location()) -
         static_cast<natural_8_bit>(min_vertex_shader_input_buffer_binding_location()) +
         1U; }

enum struct vertex_shader_output_buffer_binding_location : natural_8_bit
{
    BINDING_OUT_POSITION            = 0,
    BINDING_OUT_COLOUR              = 1,
    BINDING_OUT_NORMAL              = 2,
    BINDING_OUT_TEXCOORD0           = 3,
    BINDING_OUT_TEXCOORD1           = 4,
    BINDING_OUT_TEXCOORD2           = 5,
    BINDING_OUT_TEXCOORD3           = 6,
    BINDING_OUT_TEXCOORD4           = 7,
    BINDING_OUT_TEXCOORD5           = 8,
    BINDING_OUT_TEXCOORD6           = 9,
    BINDING_OUT_TEXCOORD7           = 10,
    BINDING_OUT_TEXCOORD8           = 11,
    BINDING_OUT_TEXCOORD9           = 12,
};

inline natural_32_bit  value(vertex_shader_output_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(vertex_shader_output_buffer_binding_location const  location);


enum struct vertex_shader_uniform_symbolic_name : natural_8_bit
{
    COLOUR_ALPHA                    = 0,
    DIFFUSE_COLOUR                  = 1,
    TRANSFORM_MATRIX_TRANSPOSED     = 2,
    NUM_MATRICES_PER_VERTEX         = 3,
};

inline natural_32_bit  value(vertex_shader_uniform_symbolic_name const  name)
{ return static_cast<natural_8_bit>(name); }
std::string  uniform_symbolic_name(vertex_shader_uniform_symbolic_name const  symbolic_name);
std::string  uniform_name(vertex_shader_uniform_symbolic_name const  symbolic_name);
vertex_shader_uniform_symbolic_name  to_symbolic_uniform_name_of_vertex_shader(std::string  name);
inline constexpr natural_32_bit  uniform_max_transform_matrices() { return 64U; }

inline constexpr vertex_shader_uniform_symbolic_name  min_vertex_shader_uniform_symbolic_name()
{ return vertex_shader_uniform_symbolic_name::COLOUR_ALPHA; }
inline constexpr vertex_shader_uniform_symbolic_name  max_vertex_shader_uniform_symbolic_name()
{ return vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX; }
inline constexpr natural_8_bit  num_vertex_shader_uniform_symbolic_names()
{ return static_cast<natural_8_bit>(max_vertex_shader_uniform_symbolic_name()) -
         static_cast<natural_8_bit>(min_vertex_shader_uniform_symbolic_name()) +
         1U; }


enum struct fragment_shader_input_buffer_binding_location : natural_8_bit
{
    BINDING_IN_POSITION             = 0,
    BINDING_IN_COLOUR               = 1,
    BINDING_IN_NORMAL               = 2,
    BINDING_IN_TEXCOORD0            = 3,
    BINDING_IN_TEXCOORD1            = 4,
    BINDING_IN_TEXCOORD2            = 5,
    BINDING_IN_TEXCOORD3            = 6,
    BINDING_IN_TEXCOORD4            = 7,
    BINDING_IN_TEXCOORD5            = 8,
    BINDING_IN_TEXCOORD6            = 9,
    BINDING_IN_TEXCOORD7            = 10,
    BINDING_IN_TEXCOORD8            = 11,
    BINDING_IN_TEXCOORD9            = 12,
};

inline natural_32_bit  value(fragment_shader_input_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(fragment_shader_input_buffer_binding_location const  location);


enum struct fragment_shader_output_binding_location : natural_8_bit
{
    BINDING_OUT_COLOUR              = 0,
    BINDING_OUT_TEXTURE_POSITION    = 1,
    BINDING_OUT_TEXTURE_NORMAL      = 2,
    BINDING_OUT_TEXTURE_DIFFUSE     = 3,
    BINDING_OUT_TEXTURE_SPECULAR    = 4,
};

inline natural_32_bit  value(fragment_shader_output_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(fragment_shader_output_binding_location const  location);


enum struct fragment_shader_uniform_symbolic_name : natural_8_bit
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

inline natural_32_bit  value(fragment_shader_uniform_symbolic_name const  binding)
{ return static_cast<natural_8_bit>(binding); }
std::string  uniform_name_symbolic(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name);
std::string  uniform_name(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name);
bool  is_texture_sampler(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name);
fragment_shader_uniform_symbolic_name  to_symbolic_uniform_name_of_fragment_shader(std::string  name);

inline constexpr fragment_shader_uniform_symbolic_name  min_fragment_shader_uniform_symbolic_name()
{ return fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_DIFFUSE; }
inline constexpr fragment_shader_uniform_symbolic_name  max_fragment_shader_uniform_symbolic_name()
{ return fragment_shader_uniform_symbolic_name::DIRECTIONAL_LIGHT_COLOUR; }
inline constexpr natural_8_bit  num_fragment_shader_uniform_symbolic_names() noexcept
{ return static_cast<natural_8_bit>(max_fragment_shader_uniform_symbolic_name()) -
         static_cast<natural_8_bit>(min_fragment_shader_uniform_symbolic_name()) +
         1U; }


bool  compatible(std::unordered_set<vertex_shader_output_buffer_binding_location> const& vertex_program_output,
                 std::unordered_set<fragment_shader_input_buffer_binding_location> const& fragment_program_input);


}

#endif
