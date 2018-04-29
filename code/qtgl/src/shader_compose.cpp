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

static std::string  varying(std::string const&  var_name, VS_IN const  location, std::unordered_set<VS_IN>&  vs_input)
{
    vs_input.insert(location);
    return layout(location) + " in " + type_name(location) + " " + var_name + ";";
}

static std::string  varying(std::string const&  var_name, VS_OUT const  location, std::unordered_set<VS_OUT>&  vs_output)
{
    vs_output.insert(location);
    return layout(location) + " out " + type_name(location) + " " + var_name + ";";
}

static std::string  uniform(VS_UNIFORM const  symbolic_name, std::unordered_set<VS_UNIFORM>&  vs_uniforms)
{
    vs_uniforms.insert(symbolic_name);
    natural_32_bit const  num_elements = qtgl::num_elements(symbolic_name);
    return "uniform " + type_name(symbolic_name) + " " + name(symbolic_name)
                      + (num_elements < 2U ? std::string() : "[" + std::to_string(num_elements) + "]") + ";";
}

static std::string  varying(std::string const&  var_name, FS_IN const  location, std::unordered_set<FS_IN>&  fs_input)
{
    fs_input.insert(location);
    return layout(location) + " in " + type_name(location) + " " + var_name + ";";
}

static std::string  varying(std::string const&  var_name, FS_OUT const  location, std::unordered_set<FS_OUT>&  fs_output)
{
    fs_output.insert(location);
    return layout(location) + " out " + type_name(location) + " " + var_name + ";";
}

static std::string  uniform(FS_UNIFORM const  symbolic_name, std::unordered_set<FS_UNIFORM>&  fs_uniforms)
{
    fs_uniforms.insert(symbolic_name);
    natural_32_bit const  num_elements = qtgl::num_elements(symbolic_name);
    return "uniform " + type_name(symbolic_name) + " " + name(symbolic_name)
                      + (num_elements < 2U ? std::string() : "[" + std::to_string(num_elements) + "]") + ";";
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

    if (effects.get_fog_type() != FOG_TYPE::NONE)
    {
        result.first = E2_QTGL_ERROR_MESSAGE_PREFIX() + "The fog type is not FOG_TYPE::NONE (fog is not supported yet).";
        result.second.set_fog_type(FOG_TYPE::NONE);
        return result;
    }

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
                        if (resources.skeletal().empty())
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    const mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;"
                                "    gl_Position = vec4(in_position,1.0f) * T;",
                                "    out_colour = DIFFUSE_COLOUR;",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("in_indices_of_matrices", VS_IN::BINDING_IN_INDICES_OF_MATRICES, vs_input),
                                varying("in_weights_of_matrices", VS_IN::BINDING_IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    int i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        const vec4 pos = vec4(in_position,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[in_indices_of_matrices[i]];",
                                "        result_position = result_position + in_weights_of_matrices[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    out_colour = DIFFUSE_COLOUR;",
                                "}",
                            };
                        }
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
                        if (resources.skeletal().empty())
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("in_colour", VS_IN::BINDING_IN_DIFFUSE, vs_input),
                                varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    const mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;"
                                "    gl_Position = vec4(in_position,1.0f) * T;",
                                "    out_colour = in_colour;",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("in_colour", VS_IN::BINDING_IN_DIFFUSE, vs_input),
                                varying("in_indices_of_matrices", VS_IN::BINDING_IN_INDICES_OF_MATRICES, vs_input),
                                varying("in_weights_of_matrices", VS_IN::BINDING_IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying("out_colour", VS_OUT::BINDING_OUT_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    int i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        const vec4 pos = vec4(in_position,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[in_indices_of_matrices[i]];",
                                "        result_position = result_position + in_weights_of_matrices[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    out_colour = in_colour;",
                                "}",
                            };
                        }
                        //if (effects.use_fog())
                        //{
                        //    fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                        //        "#version 420",

                        //        varying("in_colour", FS_IN::BINDING_IN_DIFFUSE, fs_input),
                        //        varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                        //        "void main() {",
                        //        "    const vec4 FOG_COLOUR = vec4(1.0f, 0.0f, 0.0f, 1.0f);",
                        //        "    const float FOG_NEAR = 10.0f;",
                        //        "    const float FOG_FAR = 50.0f;",
                        //        "    const float FOG_DECAY_COEF = 2.0f;",

                        //        //"    const float z = 1.0f / gl_FragCoord.w;",
                        //        //"    const float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                        //        //"    const vec3 u = gl_FragCoord.xyz / gl_FragCoord.w;",
                        //        //"    const float distance = length(u);",
                        //        //"    const float D = max(min((distance - FOG_NEAR) / (FOG_FAR - FOG_NEAR), 1.0f), 0.0f);",
                        //        //"    const float coef = pow(D, FOG_DECAY_COEF);",

                        //        "    const float z = 1.0f / gl_FragCoord.w;",
                        //        "    const float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                        //        "    out_colour = (1.0f - coef) * in_colour + coef * FOG_COLOUR;",
                        //        "}",
                        //    };
                        //}
                        //else
                        {
                            fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                "#version 420",

                                varying("in_colour", FS_IN::BINDING_IN_DIFFUSE, fs_input),
                                varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                                "void main() {",
                                "    if (in_colour.a < 0.5f)",
                                "        discard;",
                                "    out_colour = in_colour;",
                                "}",
                            };
                        }
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
                        if (resources.skeletal().empty())
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("in_texture_coords", resources.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first, vs_input),
                                varying("out_texture_coords", VS_OUT::BINDING_OUT_TEXCOORD0, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    const mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;"
                                "    gl_Position = vec4(in_position,1.0f) * T;",
                                "    out_texture_coords = in_texture_coords;",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_QTGL_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                "#version 420",

                                varying("in_position", VS_IN::BINDING_IN_POSITION, vs_input),
                                varying("in_texture_coords", resources.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first, vs_input),
                                varying("in_indices_of_matrices", VS_IN::BINDING_IN_INDICES_OF_MATRICES, vs_input),
                                varying("in_weights_of_matrices", VS_IN::BINDING_IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying("out_texture_coords", VS_OUT::BINDING_OUT_TEXCOORD0, vs_output),

                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    int i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        const vec4 pos = vec4(in_position,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[in_indices_of_matrices[i]];",
                                "        result_position = result_position + in_weights_of_matrices[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    out_texture_coords = in_texture_coords;",
                                "}",
                            };
                        }
                        fs_uid = E2_QTGL_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                            "#version 420",

                            varying("in_texture_coords", FS_IN::BINDING_IN_TEXCOORD0, fs_input),
                            varying("out_colour", FS_OUT::BINDING_OUT_COLOUR, fs_output),

                            uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),

                            "void main() {",
                            "    const vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, in_texture_coords);"
                            "    if (diffuse_colour.a < 0.5f)",
                            "        discard;",
                            "    out_colour = diffuse_colour;",
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
