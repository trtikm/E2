#ifndef QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED
#   define QTGL_SHADER_DATA_BINDINGS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
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
};

inline natural_8_bit  value(vertex_shader_input_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(vertex_shader_input_buffer_binding_location const  location);


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

inline natural_8_bit  value(vertex_shader_output_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(vertex_shader_output_buffer_binding_location const  location);


enum struct vertex_shader_uniform_symbolic_name : natural_8_bit
{
    COLOUR_ALPHA                    = 0,
    TRANSFORM_MATRIX_TRANSPOSED     = 1,
};

inline natural_8_bit  value(vertex_shader_uniform_symbolic_name const  name)
{ return static_cast<natural_8_bit>(name); }
std::string  uniform_symbolic_name(vertex_shader_uniform_symbolic_name const  symbolic_name);
std::string  uniform_name(vertex_shader_uniform_symbolic_name const  symbolic_name);


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

inline natural_8_bit  value(fragment_shader_input_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(fragment_shader_input_buffer_binding_location const  location);


enum struct fragment_shader_output_buffer_binding_location : natural_8_bit
{
    BINDING_OUT_COLOUR              = 0,
};

inline natural_8_bit  value(fragment_shader_output_buffer_binding_location const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  binding_location_name(fragment_shader_output_buffer_binding_location const  location);


enum struct fragment_shader_texture_sampler_binding : natural_8_bit
{
    BINDING_TEXTURE_DIFFUSE         = 0,
};

inline natural_8_bit  value(fragment_shader_texture_sampler_binding const  binding)
{ return static_cast<natural_8_bit>(binding); }
std::string  sampler_binding_name(fragment_shader_texture_sampler_binding const  texture_binding);


}

#endif
