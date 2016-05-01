#include <qtgl/shader_data_bindings.hpp>
#include <utility/invariants.hpp>

namespace qtgl {


std::string  binding_location_name(vertex_shader_input_buffer_binding_location const  location)
{
    switch (location)
    {
    case vertex_shader_input_buffer_binding_location::BINDING_IN_POSITION : return "BINDING_IN_POSITION";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_COLOUR   : return "BINDING_IN_COLOUR";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_NORMAL   : return "BINDING_IN_NORMAL";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0: return "BINDING_IN_TEXCOORD0";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD1: return "BINDING_IN_TEXCOORD1";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD2: return "BINDING_IN_TEXCOORD2";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD3: return "BINDING_IN_TEXCOORD3";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD4: return "BINDING_IN_TEXCOORD4";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD5: return "BINDING_IN_TEXCOORD5";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD6: return "BINDING_IN_TEXCOORD6";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD7: return "BINDING_IN_TEXCOORD7";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD8: return "BINDING_IN_TEXCOORD8";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD9: return "BINDING_IN_TEXCOORD9";
    default: UNREACHABLE();
    }
}

std::string  binding_location_name(vertex_shader_output_buffer_binding_location const  location)
{
    switch (location)
    {
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION : return "BINDING_OUT_POSITION";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_COLOUR   : return "BINDING_OUT_COLOUR";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_NORMAL   : return "BINDING_OUT_NORMAL";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD0: return "BINDING_OUT_TEXCOORD0";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD1: return "BINDING_OUT_TEXCOORD1";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD2: return "BINDING_OUT_TEXCOORD2";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD3: return "BINDING_OUT_TEXCOORD3";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD4: return "BINDING_OUT_TEXCOORD4";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD5: return "BINDING_OUT_TEXCOORD5";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD6: return "BINDING_OUT_TEXCOORD6";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD7: return "BINDING_OUT_TEXCOORD7";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD8: return "BINDING_OUT_TEXCOORD8";
    case vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD9: return "BINDING_OUT_TEXCOORD9";
    default: UNREACHABLE();
    }
}

std::string  uniform_symbolic_name(vertex_shader_uniform_symbolic_name const  symbolic_name)
{
    switch (symbolic_name)
    {
    case vertex_shader_uniform_symbolic_name::COLOUR_ALPHA: return "COLOUR_ALPHA";
    case vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED: return "TRANSFORM_MATRIX_TRANSPOSED";
    default: UNREACHABLE();
    }
}

std::string  uniform_name(vertex_shader_uniform_symbolic_name const  symbolic_name)
{
    switch (symbolic_name)
    {
    case vertex_shader_uniform_symbolic_name::COLOUR_ALPHA: return "UNIFORM_COLOUR_ALPHA";
    case vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED: return "UNIFORM_TRANSFORM_MATRIX_TRANSPOSED";
    default: UNREACHABLE();
    }
}

std::string  binding_location_name(fragment_shader_input_buffer_binding_location const  location)
{
    switch (location)
    {
    case fragment_shader_input_buffer_binding_location::BINDING_IN_POSITION : return "BINDING_IN_POSITION";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_COLOUR   : return "BINDING_IN_COLOUR";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_NORMAL   : return "BINDING_IN_NORMAL";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0: return "BINDING_IN_TEXCOORD0";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD1: return "BINDING_IN_TEXCOORD1";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD2: return "BINDING_IN_TEXCOORD2";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD3: return "BINDING_IN_TEXCOORD3";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD4: return "BINDING_IN_TEXCOORD4";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD5: return "BINDING_IN_TEXCOORD5";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD6: return "BINDING_IN_TEXCOORD6";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD7: return "BINDING_IN_TEXCOORD7";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD8: return "BINDING_IN_TEXCOORD8";
    case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD9: return "BINDING_IN_TEXCOORD9";
    default: UNREACHABLE();
    }
}

std::string  binding_location_name(fragment_shader_output_buffer_binding_location const  location)
{
    switch (location)
    {
    case fragment_shader_output_buffer_binding_location::BINDING_OUT_COLOUR: return "BINDING_OUT_COLOUR";
    default: UNREACHABLE();
    }
}

std::string  sampler_binding_name(fragment_shader_texture_sampler_binding const  texture_binding)
{
    switch (texture_binding)
    {
    case fragment_shader_texture_sampler_binding::BINDING_TEXTURE_DIFFUSE: return "BINDING_TEXTURE_DIFFUSE";
    default: UNREACHABLE();
    }
}


}
