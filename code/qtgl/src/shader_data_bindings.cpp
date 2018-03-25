#include <qtgl/shader_data_bindings.hpp>
#include <utility/invariants.hpp>
#include <utility/assumptions.hpp>
#include <unordered_map>

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
    case vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX: return "NUM_MATRICES_PER_VERTEX";
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
    case vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX: return "UNIFORM_NUM_MATRICES_PER_VERTEX";
    default: UNREACHABLE();
    }
}

vertex_shader_uniform_symbolic_name  to_symbolic_uniform_name_of_vertex_shader(std::string  name)
{
    if (name.find("UNIFORM_") == 0UL)
        name = name.substr(std::strlen("UNIFORM_"));

    if (name == "COLOUR_ALPHA")
        return vertex_shader_uniform_symbolic_name::COLOUR_ALPHA;
    if (name == "DIFFUSE_COLOUR")
        return vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR;
    if (name == "TRANSFORM_MATRIX_TRANSPOSED")
        return vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED;
    if (name == "NUM_MATRICES_PER_VERTEX")
        return vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX;

    UNREACHABLE();
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

std::string  uniform_name_symbolic(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name)
{
    switch (uniform_symbolic_name)
    {
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_DIFFUSE: return "TEXTURE_SAMPLER_DIFFUSE";
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_SPECULAR: return "TEXTURE_SAMPLER_SPECULAR";
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_NORMAL: return "TEXTURE_SAMPLER_NORMAL";
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_POSITION: return "TEXTURE_SAMPLER_POSITION";

    case fragment_shader_uniform_symbolic_name::FOG_COLOUR: return "FOG_COLOUR";
    case fragment_shader_uniform_symbolic_name::AMBIENT_COLOUR: return "AMBIENT_COLOUR";
    case fragment_shader_uniform_symbolic_name::DIFFUSE_COLOUR: return "DIFFUSE_COLOUR";
    case fragment_shader_uniform_symbolic_name::SPECULAR_COLOUR: return "SPECULAR_COLOUR";

    case fragment_shader_uniform_symbolic_name::DIRECTIONAL_LIGHT_POSITION: return "DIRECTIONAL_LIGHT_POSITION";
    case fragment_shader_uniform_symbolic_name::DIRECTIONAL_LIGHT_COLOUR: return "DIRECTIONAL_LIGHT_COLOUR";

    default: UNREACHABLE();
    }
}

std::string  uniform_name(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name)
{
    return "UNIFORM_" + uniform_name_symbolic(uniform_symbolic_name);
}

bool  is_texture_sampler(fragment_shader_uniform_symbolic_name const  uniform_symbolic_name)
{
    switch (uniform_symbolic_name)
    {
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_DIFFUSE:
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_SPECULAR:
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_NORMAL:
    case fragment_shader_uniform_symbolic_name::TEXTURE_SAMPLER_POSITION:
        return true;
    default:
        return false;
    }
}

fragment_shader_uniform_symbolic_name  to_symbolic_uniform_name_of_fragment_shader(std::string  name)
{
    if (name.find("UNIFORM_") == 0UL)
        name = name.substr(std::strlen("UNIFORM_"));

    static std::unordered_map<std::string, fragment_shader_uniform_symbolic_name> const  map =
        []() -> std::unordered_map<std::string, fragment_shader_uniform_symbolic_name> {
            std::unordered_map<std::string, fragment_shader_uniform_symbolic_name> map;
            for (natural_32_bit i = 0U; i < num_fragment_shader_uniform_symbolic_names(); ++i)
                map.insert({
                    uniform_name_symbolic(fragment_shader_uniform_symbolic_name(i)),
                    fragment_shader_uniform_symbolic_name(i)
                    });
            return map;
        }();

    auto const  it = map.find(name);
    ASSUMPTION(it != map.cend());
    return it->second;

    //{
    //    { "TEXTURE_SAMPLER_DIFFUSE",
    //    { "TEXTURE_SAMPLER_SPECULAR",
    //    { "TEXTURE_SAMPLER_NORMAL",
    //    { "TEXTURE_SAMPLER_POSITION",

    //    { "FOG_COLOUR",
    //    { "AMBIENT_COLOUR",
    //    { "DIFFUSE_COLOUR",
    //    { "SPECULAR_COLOUR",

    //    { "DIRECTIONAL_LIGHT_POSITION",
    //    { "DIRECTIONAL_LIGHT_COLOUR",
    //};

    //if (name == "COLOUR_ALPHA")
    //    return vertex_shader_uniform_symbolic_name::COLOUR_ALPHA;
    //if (name == "DIFFUSE_COLOUR")
    //    return vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR;
    //if (name == "TRANSFORM_MATRIX_TRANSPOSED")
    //    return vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED;
    //if (name == "NUM_MATRICES_PER_VERTEX")
    //    return vertex_shader_uniform_symbolic_name::NUM_MATRICES_PER_VERTEX;

    UNREACHABLE();
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
