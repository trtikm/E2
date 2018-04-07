#include <qtgl/shader_compose.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

#define E2_QTGL_STRINGIFY(X)                            #X
#define E2_QTGL_GENERATE_FILE_LINE_STRING_IMPL(X,Y)     X "[" E2_QTGL_STRINGIFY(Y) "]"
#define E2_QTGL_GENERATE_FILE_LINE_STRING()             E2_QTGL_GENERATE_FILE_LINE_STRING_IMPL(__FILE__,__LINE__)

#define E2_QTGL_GENERATE_VERTEX_SHADER_ID()             "VERTEX_SHADER@" E2_QTGL_GENERATE_FILE_LINE_STRING()
#define E2_QTGL_GENERATE_FRAGMENT_SHADER_ID()           "FRAGMENT_SHADER@" E2_QTGL_GENERATE_FILE_LINE_STRING()

#define E2_QTGL_ERROR_MESSAGE_PREFIX()                  std::string(                                                    \
                                                            E2_QTGL_GENERATE_FILE_LINE_STRING_IMPL(__FILE__,__LINE__)   \
                                                            ": ERROR : "                                                \
                                                            )

namespace qtgl { namespace detail {


using  VS_IN = VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION;
using  VS_OUT = VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION;
using  VS_UNIFORM = VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME;

using  FS_IN = FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION;
using  FS_OUT = FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION;
using  FS_UNIFORM = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME;


template<typename T>
static std::string  layout(T const  location)
{
    return std::string("layout(location=") + std::to_string(value(location)) + ")";
}


static std::string  get_varying_type_name(VS_IN const  location)
{
    switch (location)
    {
    case VS_IN::BINDING_IN_POSITION:
        return "vec3";
    case VS_IN::BINDING_IN_DIFFUSE:
        return "vec4";
    case VS_IN::BINDING_IN_SPECULAR:
        return "vec4";
    case VS_IN::BINDING_IN_NORMAL:
        return "vec3";
    case VS_IN::BINDING_IN_INDICES_OF_MATRICES:
        return "ivec4";
    case VS_IN::BINDING_IN_WEIGHTS_OF_MATRICES:
        return "vec4";
    case VS_IN::BINDING_IN_TEXCOORD0:
    case VS_IN::BINDING_IN_TEXCOORD1:
    case VS_IN::BINDING_IN_TEXCOORD2:
    case VS_IN::BINDING_IN_TEXCOORD3:
    case VS_IN::BINDING_IN_TEXCOORD4:
    case VS_IN::BINDING_IN_TEXCOORD5:
    case VS_IN::BINDING_IN_TEXCOORD6:
    case VS_IN::BINDING_IN_TEXCOORD7:
    case VS_IN::BINDING_IN_TEXCOORD8:
    case VS_IN::BINDING_IN_TEXCOORD9:
        return "vec2";
    default: UNREACHABLE();
    }
}


static std::string  varying(std::string const&  var_name, VS_IN const  location, std::unordered_set<VS_IN>&  vs_input)
{
    vs_input.insert(location);
    return layout(location) + " in " + get_varying_type_name(location) + " " + var_name + ";";
}


static std::string  get_varying_type_name(VS_OUT const  location)
{
    switch (location)
    {
    case VS_OUT::BINDING_OUT_POSITION:
        return "vec3";
    case VS_OUT::BINDING_OUT_DIFFUSE:
        return "vec4";
    case VS_OUT::BINDING_OUT_SPECULAR:
        return "vec4";
    case VS_OUT::BINDING_OUT_NORMAL:
        return "vec3";
    case VS_OUT::BINDING_OUT_TEXCOORD0:
    case VS_OUT::BINDING_OUT_TEXCOORD1:
    case VS_OUT::BINDING_OUT_TEXCOORD2:
    case VS_OUT::BINDING_OUT_TEXCOORD3:
    case VS_OUT::BINDING_OUT_TEXCOORD4:
    case VS_OUT::BINDING_OUT_TEXCOORD5:
    case VS_OUT::BINDING_OUT_TEXCOORD6:
    case VS_OUT::BINDING_OUT_TEXCOORD7:
    case VS_OUT::BINDING_OUT_TEXCOORD8:
    case VS_OUT::BINDING_OUT_TEXCOORD9:
        return "vec2";
    default: UNREACHABLE();
    }
}


static std::string  varying(std::string const&  var_name, VS_OUT const  location, std::unordered_set<VS_OUT>&  vs_output)
{
    vs_output.insert(location);
    return layout(location) + " out " + get_varying_type_name(location) + " " + var_name + ";";
}


static std::string  get_uniform_type_name(VS_UNIFORM const  symbolic_name)
{
    switch (symbolic_name)
    {
    case VS_UNIFORM::COLOUR_ALPHA:
        UNREACHABLE(); // This uniform is about to be removed, so no support here.
    case VS_UNIFORM::DIFFUSE_COLOUR:
        return "ver4";
    case VS_UNIFORM::TRANSFORM_MATRIX_TRANSPOSED:
        return "mat4";
    case VS_UNIFORM::NUM_MATRICES_PER_VERTEX:
        return "uint";
    default: UNREACHABLE();
    }
}


static std::string  uniform(VS_UNIFORM const  symbolic_name, std::unordered_set<VS_UNIFORM>&  vs_uniforms)
{
    vs_uniforms.insert(symbolic_name);
    return "uniform " + get_uniform_type_name(symbolic_name) + " " + name(symbolic_name) + ";";
}


static std::string  get_varying_type_name(FS_IN const  location)
{
    switch (location)
    {
    case FS_IN::BINDING_IN_POSITION:
        return "vec3";
    case FS_IN::BINDING_IN_DIFFUSE:
        return "vec4";
    case FS_IN::BINDING_IN_SPECULAR:
        return "vec4";
    case FS_IN::BINDING_IN_NORMAL:
        return "vec3";
    case FS_IN::BINDING_IN_TEXCOORD0:
    case FS_IN::BINDING_IN_TEXCOORD1:
    case FS_IN::BINDING_IN_TEXCOORD2:
    case FS_IN::BINDING_IN_TEXCOORD3:
    case FS_IN::BINDING_IN_TEXCOORD4:
    case FS_IN::BINDING_IN_TEXCOORD5:
    case FS_IN::BINDING_IN_TEXCOORD6:
    case FS_IN::BINDING_IN_TEXCOORD8:
    case FS_IN::BINDING_IN_TEXCOORD9:
        return "vec2";
    default: UNREACHABLE();
    }
}


static std::string  varying(std::string const&  var_name, FS_IN const  location, std::unordered_set<FS_IN>&  fs_input)
{
    fs_input.insert(location);
    return layout(location) + " in " + get_varying_type_name(location) + " " + var_name + ";";
}


static std::string  get_varying_type_name(FS_OUT const  location)
{
    switch (location)
    {
    case FS_OUT::BINDING_OUT_COLOUR:
        return "vec4";
    case FS_OUT::BINDING_OUT_TEXTURE_POSITION:
        return "vec3";
    case FS_OUT::BINDING_OUT_TEXTURE_NORMAL:
        return "vec3";
    case FS_OUT::BINDING_OUT_TEXTURE_DIFFUSE:
        return "vec4";
    case FS_OUT::BINDING_OUT_TEXTURE_SPECULAR:
        return "vec4";
    default: UNREACHABLE();
    }
}


static std::string  varying(std::string const&  var_name, FS_OUT const  location, std::unordered_set<FS_OUT>&  fs_output)
{
    fs_output.insert(location);
    return layout(location) + " out " + get_varying_type_name(location) + " " + var_name + ";";
}


static std::string  get_uniform_type_name(FS_UNIFORM const  symbolic_name)
{
    switch (symbolic_name)
    {
    case FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE:
    case FS_UNIFORM::TEXTURE_SAMPLER_SPECULAR:
    case FS_UNIFORM::TEXTURE_SAMPLER_NORMAL:
    case FS_UNIFORM::TEXTURE_SAMPLER_POSITION:
        return "sampler2D";
    case FS_UNIFORM::FOG_COLOUR:
        return "vec4";
    case FS_UNIFORM::AMBIENT_COLOUR:
        return "vec4";
    case FS_UNIFORM::DIFFUSE_COLOUR:
        return "vec4";
    case FS_UNIFORM::SPECULAR_COLOUR:
        return "vec4";
    case FS_UNIFORM::DIRECTIONAL_LIGHT_POSITION:
        return "vec3";
    case FS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR:
        return "vec4";
    default: UNREACHABLE();
    }
}


static std::string  uniform(FS_UNIFORM const  symbolic_name, std::unordered_set<FS_UNIFORM>&  fs_uniforms)
{
    fs_uniforms.insert(symbolic_name);
    return "uniform " + get_uniform_type_name(symbolic_name) + " " + name(symbolic_name) + ";";
}


static void  add_new_line_terminators(std::vector<std::string>&  lines_of_shader_code)
{
    for (auto& line : lines_of_shader_code)
    {
        line.push_back('\n');
        line.push_back('\0');
    }
}


static shader_compose_result_type  compose_vertex_and_fragment_shader(
        batch_available_resources const  resources,
        effects_config const&  effects,
        std::vector<std::string>&  vs_source,
        std::string&  vs_uid,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>&  vs_output,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms,
        std::vector<std::string>&  fs_source,
        std::string&  fs_uid,
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  fs_input,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>&  fs_output,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>&  fs_uniforms
        )
{
    TMPROF_BLOCK();

    shader_compose_result_type  result{ "", effects};

    if (effects.get_shader_output_types().size() == 1UL &&
        *effects.get_shader_output_types().cbegin() == SHADER_DATA_OUTPUT_TYPE::DEFAULT)
    {
        if (effects.get_light_types().empty())
        {
            if (effects.get_lighting_data_types().size() == 1UL &&
                effects.get_lighting_data_types().cbegin()->first == LIGHTING_DATA_TYPE::DIFFUSE)
            {
                switch (effects.get_lighting_data_types().cbegin()->second)
                {
                case SHADER_DATA_INPUT_TYPE::UNIFORM:
                    {
                        vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                            "#version 420",

                            varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                            varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                            uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                            uniform(VS_UNIFORM::TRANSFORM_MATRIX_TRANSPOSED, vs_uniforms),

                            "void main() {",
                            "    gl_Position = vec4(in_position,1.0f) * TRANSFORM_MATRIX_TRANSPOSED;",
                            "    out_colour = DIFFUSE_COLOUR;",
                            "}",
                        };
                        fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                            "#version 420",

                            varying("in_colour", FS_IN::BINDING_IN_DIFFUSE, fs_input),
                            varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                            "void main() {",
                            "    out_colour = in_colour;",
                            "}",
                        };
                    }
                    break;
                case SHADER_DATA_INPUT_TYPE::BUFFER:
                    if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE) == 0UL)
                    {
                        result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "Diffuse colour buffer is not available.";
                        result.second.get_lighting_data_types().begin()->second = SHADER_DATA_INPUT_TYPE::UNIFORM;
                    }
                    else
                    {
                        vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                            "#version 420",

                            varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                            varying("in_colour", VS_IN::BINDING_IN_DIFFUSE, vs_input),
                            varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                            uniform(VS_UNIFORM::TRANSFORM_MATRIX_TRANSPOSED, vs_uniforms),

                            "void main() {",
                            "    gl_Position = vec4(in_position,1.0f) * TRANSFORM_MATRIX_TRANSPOSED;",
                            "    out_colour = in_colour;",
                            "}",
                        };
                        fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                            "#version 420",

                            varying("in_colour", FS_IN::BINDING_IN_DIFFUSE, fs_input),
                            varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                            "void main() {",
                            "    out_colour = in_colour;",
                            "}",
                        };
                    }
                    break;
                case SHADER_DATA_INPUT_TYPE::TEXTURE:
                    if (resources.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE) == 0UL)
                    {
                        result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "Diffuse texture is not available.";
                        result.second.get_lighting_data_types().begin()->second = SHADER_DATA_INPUT_TYPE::BUFFER;
                    }
                    else
                    {
                        vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                            "#version 420",

                            varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                            varying("in_texture_coords", resources.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first, vs_input),
                            varying("out_texture_coords", VS_OUT::BINDING_OUT_TEXCOORD0, vs_output),

                            uniform(VS_UNIFORM::TRANSFORM_MATRIX_TRANSPOSED, vs_uniforms),

                            "void main() {",
                            "    gl_Position = vec4(in_position,1.0f) * TRANSFORM_MATRIX_TRANSPOSED;",
                            "    out_texture_coords = in_texture_coords;",
                            "}",
                        };
                        fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                            "#version 420",

                            varying("in_texture_coords", FS_IN::BINDING_IN_TEXCOORD0, fs_input),
                            varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                            uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),

                            "void main() {",
                            "    out_colour = texture(TEXTURE_SAMPLER_DIFFUSE, in_texture_coords);",
                            "}",
                        };
                    }
                    break;
                }
            }
            else
            {
                result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "The lighting is not the diffuse one.";
                result.second.get_lighting_data_types() = {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE }};
            }
        }
        else
        {
            result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "The set of supported light types is not empty.";
            result.second.get_light_types().clear();
        }
    }
    else
    {
        result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "Output type from the shader is not SHADER_DATA_OUTPUT_TYPE::DEFAULT.";
        result.second.get_shader_output_types() = {SHADER_DATA_OUTPUT_TYPE::DEFAULT};
    }

    return result;
}


}}

namespace qtgl {


shader_compose_result_type  compose_vertex_and_fragment_shader(
        batch_available_resources const  resources,
        effects_config const&  effects,
        std::vector<std::string>&  vs_source,
        std::string&  vs_uid,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>&  vs_output,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms,
        std::vector<std::string>&  fs_source,
        std::string&  fs_uid,
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  fs_input,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>&  fs_output,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>&  fs_uniforms
        )
{
    TMPROF_BLOCK();

    shader_compose_result_type const  result =
            detail::compose_vertex_and_fragment_shader(
                    resources,
                    effects,
                    vs_source,
                    vs_uid,
                    vs_input,
                    vs_output,
                    vs_uniforms,
                    fs_source,
                    fs_uid,
                    fs_input,
                    fs_output,
                    fs_uniforms
                    );
    if (result.first.empty())
    {
        INVARIANT(compatible(vs_output, fs_input));
        detail::add_new_line_terminators(vs_source);
        detail::add_new_line_terminators(fs_source);
    }

    return result;
}


}
