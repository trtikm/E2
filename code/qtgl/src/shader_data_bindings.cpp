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
    case vertex_shader_input_buffer_binding_location::BINDING_IN_INDICES_OF_MATRICES: return "BINDING_IN_INDICES_OF_MATRICES";
    case vertex_shader_input_buffer_binding_location::BINDING_IN_WEIGHTS_OF_MATRICES: return "BINDING_IN_WEIGHTS_OF_MATRICES";
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
    case vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR: return "DIFFUSE_COLOUR";
    case vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED: return "TRANSFORM_MATRIX_TRANSPOSED";
    default: UNREACHABLE();
    }
}

std::string  uniform_name(vertex_shader_uniform_symbolic_name const  symbolic_name)
{
    switch (symbolic_name)
    {
    case vertex_shader_uniform_symbolic_name::COLOUR_ALPHA: return "UNIFORM_COLOUR_ALPHA";
    case vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR: return "UNIFORM_DIFFUSE_COLOUR";
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

bool  compatible(std::unordered_set<vertex_shader_output_buffer_binding_location> const& vertex_program_output,
                 std::unordered_set<fragment_shader_input_buffer_binding_location> const& fragment_program_input)
{
    for (auto const  input : fragment_program_input)
        switch (input)
        {
        case fragment_shader_input_buffer_binding_location::BINDING_IN_POSITION :
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_POSITION) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_COLOUR   :
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_COLOUR) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_NORMAL   :
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_NORMAL) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD0:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD0) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD1:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD1) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD2:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD2) == 0ULL)
                return false;;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD3:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD3) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD4:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD4) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD5:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD5) == 0ULL)
                return false;;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD6:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD6) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD7:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD7) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD8:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD8) == 0ULL)
                return false;
            break;
        case fragment_shader_input_buffer_binding_location::BINDING_IN_TEXCOORD9:
            if (vertex_program_output.count(vertex_shader_output_buffer_binding_location::BINDING_OUT_TEXCOORD9) == 0ULL)
                return false;
            break;
        default:
            UNREACHABLE();
        }
    return true;
}


}
