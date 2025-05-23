#include <gfx/shader_compose.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/config.hpp>

#define E2_GFX_STRINGIFY(X)                            #X
#define E2_GFX_GENERATE_FILE_LINE_STRING_IMPL(X,Y)     X "[" E2_GFX_STRINGIFY(Y) "]"
//#define E2_GFX_GENERATE_FILE_LINE_STRING()             E2_GFX_GENERATE_FILE_LINE_STRING_IMPL(__FILE__,__LINE__)
#define E2_GFX_GENERATE_FILE_LINE_STRING()             E2_GFX_GENERATE_FILE_LINE_STRING_IMPL("shader_compose.cpp",__LINE__)

#define E2_GFX_GENERATE_VERTEX_SHADER_ID()             "/VERTEX_SHADER/" E2_GFX_GENERATE_FILE_LINE_STRING()
#define E2_GFX_GENERATE_FRAGMENT_SHADER_ID()           "/FRAGMENT_SHADER/" E2_GFX_GENERATE_FILE_LINE_STRING()

#define E2_GFX_ERROR_MESSAGE_PREFIX()                  std::string(                                                    \
                                                            E2_GFX_GENERATE_FILE_LINE_STRING_IMPL(__FILE__,__LINE__)   \
                                                            ": ERROR : "                                                \
                                                            )

#define DEFINE_FUNCTION_ambient_and_directional_lighting() \
        "vec4  ambient_and_directional_lighting(",\
        "        vec3  normal,",\
        "        vec3  light_dir,",\
        "        vec3  ambient_colour,",\
        "        vec3  light_colour",\
        "        ) {",\
        "    float  mult = min(1.0f, max(0.5f - 0.5f * dot(normal, light_dir), 0.0f));",\
        "    return vec4((1.0f - mult) * ambient_colour  + mult * light_colour, 1.0f);",\
        "}"


namespace gfx { namespace detail {


using  VS_IN = VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION;
using  VS_OUT = VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION;
using  VS_UNIFORM = VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME;

using  FS_IN = FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION;
using  FS_OUT = FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION;
using  FS_UNIFORM = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME;

static std::string  vs_get_version()
{
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "#version 300 es";
#else
    return "#version 420";
#endif    
}

static std::string  fs_get_version()
{
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "#version 300 es\n"
           "precision highp float;\n";
#else
    return "#version 420";
#endif    
}

template<typename T>
static std::string  layout(T const  location)
{
    return std::string("layout(location=") + std::to_string(value(location)) + ")";
}

static std::string  varying(VS_IN const  location, std::unordered_set<VS_IN>&  vs_input)
{
    vs_input.insert(location);
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "in " + type_name(location) + " " + name(location) + ";";
#else
    return layout(location) + " in " + type_name(location) + " " + name(location) + ";";
#endif
}

static std::string  varying(VS_OUT const  location, std::unordered_set<VS_OUT>&  vs_output)
{
    vs_output.insert(location);
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "out " + type_name(location) + " " + name(location) + ";";
#else
    return layout(location) + " out " + type_name(location) + " " + name(location) + ";";
#endif
}

static std::string  uniform(VS_UNIFORM const  symbolic_name, std::unordered_set<VS_UNIFORM>&  vs_uniforms)
{
    vs_uniforms.insert(symbolic_name);
    natural_32_bit const  num_elements = gfx::num_elements(symbolic_name);
    return "uniform " + type_name(symbolic_name) + " " + name(symbolic_name)
                      + (num_elements < 2U ? std::string() : "[" + std::to_string(num_elements) + "]") + ";";
}

static std::string  varying(FS_IN const  location, std::unordered_set<FS_IN>&  fs_input)
{
    fs_input.insert(location);
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "in " + type_name(location) + " " + name(location) + ";";
#else
    return layout(location) + " in " + type_name(location) + " " + name(location) + ";";
#endif
}

static std::string  varying(FS_OUT const  location, std::unordered_set<FS_OUT>&  fs_output)
{
    fs_output.insert(location);
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
    return "out " + type_name(location) + " " + name(location) + ";";
#else
    return layout(location) + " out " + type_name(location) + " " + name(location) + ";";
#endif
}

static std::string  uniform(FS_UNIFORM const  symbolic_name, std::unordered_set<FS_UNIFORM>&  fs_uniforms)
{
    fs_uniforms.insert(symbolic_name);
    natural_32_bit const  num_elements = gfx::num_elements(symbolic_name);
    return "uniform " + type_name(symbolic_name) + " " + name(symbolic_name)
                      + (num_elements < 2U ? std::string() : "[" + std::to_string(num_elements) + "]") + ";";
}

static std::string  vs_backward_compatibility_declarations()
{
    return "";//"out gl_PerVertex { vec4 gl_Position; float gl_PointSize; float gl_ClipDistance[]; };";
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
        std::string const&  skin_name,
        effects_config_data const&  effects,
        std::vector<std::string>&  vs_source,
        std::vector<std::string>&  vs_source_instancing,
        std::string&  vs_uid,
        std::string&  vs_uid_instancing,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input_instancing,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>&  vs_output,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms_instancing,
        std::vector<std::string>&  fs_source,
        std::string&  fs_uid,
        std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  fs_input,
        std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>&  fs_output,
        std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>&  fs_uniforms
        )
{
    TMPROF_BLOCK();

    shader_compose_result_type  result{ "", effects};

    if (effects.get_fog_type() == FOG_TYPE::INTERPOLATED)
    {
        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "The fog type is FOG_TYPE::INTERPOLATED - not supported yet. Disabling fog.";
        result.second.set_fog_type(FOG_TYPE::NONE);
        return result;
    }

    batch_available_resources::skin_type const&  skin = resources.skins().at(skin_name);

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
                        if (resources.skeletal() == nullptr)
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_DIFFUSE = DIFFUSE_COLOUR;",
                                "}",
                            };
                            vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input_instancing),
                                varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms_instancing),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                "void main() {",
                                "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_DIFFUSE = DIFFUSE_COLOUR;",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    uint i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        vec4 pos = vec4(IN_POSITION,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    PASS_DIFFUSE = DIFFUSE_COLOUR;",
                                "}",
                            };
                        }
                        fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                            fs_get_version(),

                            varying(FS_IN::PASS_DIFFUSE, fs_input),
                            varying(FS_OUT::OUT_COLOUR, fs_output),

                            "void main() {",
                            "    OUT_COLOUR = PASS_DIFFUSE;",
                            "}",
                        };
                    }
                    break;
                case SHADER_DATA_INPUT_TYPE::BUFFER:
                    if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE) == 0UL)
                    {
                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse colour buffer is not available.";
                        result.second.get_lighting_data_types().begin()->second = SHADER_DATA_INPUT_TYPE::UNIFORM;
                    }
                    else
                    {
                        if (resources.skeletal() == nullptr)
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(VS_IN::IN_DIFFUSE, vs_input),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_DIFFUSE = IN_DIFFUSE;",
                                "}",
                            };
                            vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input_instancing),
                                varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                varying(VS_IN::IN_DIFFUSE, vs_input_instancing),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                "void main() {",
                                "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_DIFFUSE = IN_DIFFUSE;",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(VS_IN::IN_DIFFUSE, vs_input),
                                varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    uint i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        vec4 pos = vec4(IN_POSITION,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    PASS_DIFFUSE = IN_DIFFUSE;",
                                "}",
                            };
                        }
                        if (effects.get_fog_type() == FOG_TYPE::DETAILED)
                        {
                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                fs_get_version(),

                                varying(FS_IN::PASS_DIFFUSE, fs_input),
                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                uniform(FS_UNIFORM::FOG_COLOUR, fs_uniforms),
                                uniform(FS_UNIFORM::FOG_NEAR, fs_uniforms),
                                uniform(FS_UNIFORM::FOG_FAR, fs_uniforms),

                                "void main() {",
                                //"    vec4 FOG_COLOUR = vec4(1.0f, 0.0f, 0.0f, 1.0f);",
                                //"    float FOG_NEAR = 10.0f;",
                                //"    float FOG_FAR = 50.0f;",
                                //"    float FOG_DECAY_COEF = 2.0f;",

                                //"    float z = 1.0f / gl_FragCoord.w;",
                                //"    float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                                //"    vec3 u = gl_FragCoord.xyz / gl_FragCoord.w;",
                                //"    float distance = length(u);",
                                //"    float D = max(min((distance - FOG_NEAR) / (FOG_FAR - FOG_NEAR), 1.0f), 0.0f);",
                                //"    float coef = pow(D, FOG_DECAY_COEF);",

                                "    float z = 1.0f / gl_FragCoord.w;",
                                //"    float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                                //"    OUT_COLOUR = (1.0f - coef) * PASS_DIFFUSE + coef * FOG_COLOUR;",
                                "    float z01 = min(max(0.0f, (z - FOG_NEAR) / FOG_FAR), 1.0f);",
                                "    OUT_COLOUR = mix(PASS_DIFFUSE, FOG_COLOUR, z01);",
                                "}",
                            };
                        }
                        else if (skin.alpha_testing().use_alpha_testing())
                        {
                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                fs_get_version(),

                                varying(FS_IN::PASS_DIFFUSE, fs_input),
                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                "void main() {",
                                "    if (PASS_DIFFUSE.a < ALPHA_TEST_CONSTANT)",
                                "        discard;",
                                "    OUT_COLOUR = PASS_DIFFUSE;",
                                "}",
                            };
                        }
                        else
                        {
                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                fs_get_version(),

                                varying(FS_IN::PASS_DIFFUSE, fs_input),
                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                "void main() {",
                                "    OUT_COLOUR = PASS_DIFFUSE;",
                                "}",
                            };
                        }
                    }
                    break;
                case SHADER_DATA_INPUT_TYPE::TEXTURE:
                    if (skin.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE) == 0UL)
                    {
                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse texture is not available.";
                        result.second.get_lighting_data_types().begin()->second = SHADER_DATA_INPUT_TYPE::BUFFER;
                    }
                    else
                    {
                        VS_IN const IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION = skin.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first;
                        std::string const  IN_DIFFUSE_TEXCOORDS = name(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION);

                        if (resources.skeletal() == nullptr)
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                "}",
                            };
                            vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input_instancing),
                                varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input_instancing),
                                varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                "void main() {",
                                "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                "}",
                            };
                        }
                        else
                        {
                            vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input),
                                varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                "void main() {",
                                "    uint i;",
                                "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                "    {",
                                "        vec4 pos = vec4(IN_POSITION,1.0f) *",
                                "                         MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                "    }",
                                "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                "}",
                            };
                        }
                        if (skin.alpha_testing().use_alpha_testing())
                        {
                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                fs_get_version(),

                                varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),
                                uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                "void main() {",
                                "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                "    if (diffuse_colour.a < ALPHA_TEST_CONSTANT)",
                                "        discard;",
                                "    OUT_COLOUR = diffuse_colour;",
                                "}",
                            };
                        }
                        else
                        {
                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                fs_get_version(),

                                varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),

                                "void main() {",
                                "    OUT_COLOUR = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                "}",
                            };
                        }
                    }
                    break;
                case SHADER_DATA_INPUT_TYPE::INSTANCE:
                    {
                        if (resources.skeletal() == nullptr)
                        {
                            vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                vs_get_version(),

                                vs_backward_compatibility_declarations(),

                                varying(VS_IN::IN_POSITION, vs_input_instancing),
                                varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                varying(VS_IN::IN_INSTANCED_DIFFUSE_COLOUR, vs_input_instancing),
                                varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                "void main() {",
                                "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                "    PASS_DIFFUSE = IN_INSTANCED_DIFFUSE_COLOUR;",
                                "}",
                            };
                            if (skin.alpha_testing().use_alpha_testing())
                            {
                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                    fs_get_version(),

                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                    uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                    "void main() {",
                                    "    if (PASS_DIFFUSE.a < ALPHA_TEST_CONSTANT)",
                                    "        discard;",
                                    "    OUT_COLOUR = PASS_DIFFUSE;",
                                    "}",
                                };
                            }
                            else
                            {
                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                    fs_get_version(),

                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                    "void main() {",
                                    "    OUT_COLOUR = PASS_DIFFUSE;",
                                    "}",
                                };
                            }
                        }
                        else
                        {
                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Instancing is not supported for skeletal-animated objects.";
                            result.second.get_lighting_data_types().begin()->second =
                                    resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE) == 0UL ?
                                            SHADER_DATA_INPUT_TYPE::UNIFORM :
                                            SHADER_DATA_INPUT_TYPE::BUFFER  ;
                        }
                    }
                    break;
                }
            }
            else
            {
                result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "The lighting is not the diffuse one.";
                result.second.get_lighting_data_types() = {{LIGHTING_DATA_TYPE::DIFFUSE, SHADER_DATA_INPUT_TYPE::TEXTURE }};
            }
        }
        else if (effects.get_light_types().count(LIGHT_TYPE::AMBIENT) != 0UL)
        {
            if (effects.get_light_types().count(LIGHT_TYPE::DIRECTIONAL) != 0UL)
            {
                if (effects.get_lighting_data_types().size() >= 3UL &&
                    effects.get_lighting_data_types().count(LIGHTING_DATA_TYPE::DIRECTION) != 0UL &&
                    effects.get_lighting_data_types().count(LIGHTING_DATA_TYPE::NORMAL) != 0UL &&
                    effects.get_lighting_data_types().count(LIGHTING_DATA_TYPE::DIFFUSE) != 0UL
                    )
                {
                    if (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::DIRECTION) == SHADER_DATA_INPUT_TYPE::UNIFORM)
                    {
                        if (effects.get_lighting_data_types().count(LIGHTING_DATA_TYPE::SPECULAR) == 0UL)
                        {
                            if (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::NORMAL) == SHADER_DATA_INPUT_TYPE::BUFFER)
                            {
                                if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_NORMAL) == 0UL)
                                {
                                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Normals buffer is not available.";
                                    result.second.get_lighting_data_types() = { { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE } };
                                    result.second.get_light_types().clear();
                                }
                                else
                                {
                                    switch (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::DIFFUSE))
                                    {
                                    case SHADER_DATA_INPUT_TYPE::UNIFORM:
                                        {
                                            if (resources.skeletal() == nullptr)
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * (MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE);",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * DIFFUSE_COLOUR;",
                                                    "}",
                                                };
                                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                                    varying(VS_IN::IN_NORMAL, vs_input_instancing),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * (IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE);",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * DIFFUSE_COLOUR;",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                                    varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIFFUSE_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    uint i;",
                                                    "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_normal = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                                    "    {",
                                                    "        vec4 pos = vec4(IN_POSITION,1.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                                    "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                                    "        vec3 nor = vec3(vec4(IN_NORMAL,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]);",
                                                    "        result_normal = result_normal + IN_WEIGHTS_OF_MATRICES[i] * nor;",
                                                    "    }",
                                                    "    result_normal = normalize(result_normal);",
                                                    "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        result_normal,",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * DIFFUSE_COLOUR;",
                                                    "}",
                                                };
                                            }
                                            fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                fs_get_version(),

                                                varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                varying(FS_OUT::OUT_COLOUR, fs_output),

                                                "void main() {",
                                                "    OUT_COLOUR = PASS_DIFFUSE;",
                                                "}",
                                            };
                                        }
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::BUFFER:
                                        if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE) == 0UL)
                                        {
                                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse colour buffer is not available.";
                                            result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::DIFFUSE] = SHADER_DATA_INPUT_TYPE::UNIFORM;
                                        }
                                        else
                                        {
                                            if (resources.skeletal() == nullptr)
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_IN::IN_DIFFUSE, vs_input),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * IN_DIFFUSE;",
                                                    "}",
                                                };
                                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                                    varying(VS_IN::IN_NORMAL, vs_input_instancing),
                                                    varying(VS_IN::IN_DIFFUSE, vs_input_instancing),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * IN_DIFFUSE;",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_IN::IN_DIFFUSE, vs_input),
                                                    varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                                    varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    uint i;",
                                                    "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_normal = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                                    "    {",
                                                    "        vec4 pos = vec4(IN_POSITION,1.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                                    "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                                    "        vec3 nor = vec3(vec4(IN_NORMAL,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]);",
                                                    "        result_normal = result_normal + IN_WEIGHTS_OF_MATRICES[i] * nor;",
                                                    "    }",
                                                    "    result_normal = normalize(result_normal);",
                                                    "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        result_normal,",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * IN_DIFFUSE;",
                                                    "}",
                                                };
                                            }
                                            if (effects.get_fog_type() == FOG_TYPE::DETAILED)
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::FOG_COLOUR, fs_uniforms),
                                                    uniform(FS_UNIFORM::FOG_NEAR, fs_uniforms),
                                                    uniform(FS_UNIFORM::FOG_FAR, fs_uniforms),

                                                    "void main() {",
                                                    //"    vec4 FOG_COLOUR = vec4(1.0f, 0.0f, 0.0f, 1.0f);",
                                                    //"    float FOG_NEAR = 10.0f;",
                                                    //"    float FOG_FAR = 50.0f;",
                                                    //"    float FOG_DECAY_COEF = 2.0f;",

                                                    //"    float z = 1.0f / gl_FragCoord.w;",
                                                    //"    float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                                                    //"    vec3 u = gl_FragCoord.xyz / gl_FragCoord.w;",
                                                    //"    float distance = length(u);",
                                                    //"    float D = max(min((distance - FOG_NEAR) / (FOG_FAR - FOG_NEAR), 1.0f), 0.0f);",
                                                    //"    float coef = pow(D, FOG_DECAY_COEF);",

                                                    "    float z = 1.0f / gl_FragCoord.w;",
                                                    //"    float coef = pow(min(z / FOG_FAR, 1.0f), FOG_DECAY_COEF);",

                                                    //"    OUT_COLOUR = (1.0f - coef) * PASS_DIFFUSE + coef * FOG_COLOUR;",
                                                    "    float z01 = min(max(0.0f, (z - FOG_NEAR) / FOG_FAR), 1.0f);",
                                                    "    OUT_COLOUR = mix(PASS_DIFFUSE, FOG_COLOUR, z01);",
                                                    "}",
                                                };
                                            }
                                            else if (skin.alpha_testing().use_alpha_testing())
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                                    "void main() {",
                                                    "    if (PASS_DIFFUSE.a < ALPHA_TEST_CONSTANT)",
                                                    "        discard;",
                                                    "    OUT_COLOUR = PASS_DIFFUSE;",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    "void main() {",
                                                    "    OUT_COLOUR = PASS_DIFFUSE;",
                                                    "}",
                                                };
                                            }
                                        }
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::TEXTURE:
                                        if (skin.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE) == 0UL)
                                        {
                                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse texture is not available.";
                                            result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::DIFFUSE] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                        }
                                        else
                                        {
                                            VS_IN const IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION = skin.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first;
                                            std::string const  IN_DIFFUSE_TEXCOORDS = name(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION);

                                            if (resources.skeletal() == nullptr)
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "    PASS_DIFFUSE = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "}",
                                                };
                                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                                    varying(VS_IN::IN_NORMAL, vs_input_instancing),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input_instancing),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "    PASS_DIFFUSE = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                                    varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                                    varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms),
                                                    uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    uint i;",
                                                    "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_normal = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                                    "    {",
                                                    "        vec4 pos = vec4(IN_POSITION,1.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                                    "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                                    "        vec3 nor = vec3(vec4(IN_NORMAL,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]);",
                                                    "        result_normal = result_normal + IN_WEIGHTS_OF_MATRICES[i] * nor;",
                                                    "    }",
                                                    "    result_normal = normalize(result_normal);",
                                                    "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "    PASS_DIFFUSE = ambient_and_directional_lighting(",
                                                    "        result_normal,",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "}",
                                                };
                                            }
                                            if (skin.alpha_testing().use_alpha_testing())
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),
                                                    uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                                    "void main() {",
                                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                                    "    if (diffuse_colour.a < ALPHA_TEST_CONSTANT)",
                                                    "        discard;",
                                                    "    OUT_COLOUR = PASS_DIFFUSE * diffuse_colour;",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),

                                                    "void main() {",
                                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                                    "    OUT_COLOUR = PASS_DIFFUSE * diffuse_colour;",
                                                    "}",
                                                };
                                            }
                                        }
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::INSTANCE:
                                        {
                                            if (resources.skeletal() == nullptr)
                                            {
                                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                                    varying(VS_IN::IN_NORMAL, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_DIFFUSE_COLOUR, vs_input_instancing),
                                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, vs_uniforms_instancing),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                                    "    vec4 colour_mult = ambient_and_directional_lighting(",
                                                    "        vec3(vec4(IN_NORMAL, 0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    PASS_DIFFUSE = colour_mult * IN_INSTANCED_DIFFUSE_COLOUR;",
                                                    "}",
                                                };
                                                if (skin.alpha_testing().use_alpha_testing())
                                                {
                                                    fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                        fs_get_version(),

                                                        varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                        varying(FS_OUT::OUT_COLOUR, fs_output),

                                                        uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                                        "void main() {",
                                                        "    if (PASS_DIFFUSE.a < ALPHA_TEST_CONSTANT)",
                                                        "        discard;",
                                                        "    OUT_COLOUR = PASS_DIFFUSE;",
                                                        "}",
                                                    };
                                                }
                                                else
                                                {
                                                    fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                        vs_get_version(),

                                                        varying(FS_IN::PASS_DIFFUSE, fs_input),
                                                        varying(FS_OUT::OUT_COLOUR, fs_output),

                                                        "void main() {",
                                                        "    OUT_COLOUR = PASS_DIFFUSE;",
                                                        "}",
                                                    };
                                                }
                                            }
                                            else
                                            {
                                                result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Instancing is not supported for skeletal-deformed objects.";
                                                result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::DIFFUSE] =
                                                        resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE) == 0UL ?
                                                                SHADER_DATA_INPUT_TYPE::UNIFORM :
                                                                SHADER_DATA_INPUT_TYPE::BUFFER  ;
                                            }
                                        }
                                        break;
                                    }
                                }
                            }
                            else if (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::NORMAL) == SHADER_DATA_INPUT_TYPE::TEXTURE)
                            {
                                if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_NORMAL) == 0UL)
                                {
                                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Normals buffer is not available.";
                                    result.second.get_lighting_data_types() = { { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE } };
                                    result.second.get_light_types().clear();
                                }
                                else if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TANGENT) == 0UL)
                                {
                                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Tangents buffer is not available. Normals texture cannot be used.";
                                    result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                }
                                else if (resources.buffers().count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_BITANGENT) == 0UL)
                                {
                                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Bitangents buffer is not available. Normals texture cannot be used.";
                                    result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                }
                                else if (skin.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_NORMAL) == 0UL)
                                {
                                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Normals texture is not available.";
                                    result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                }
                                else
                                {
                                    switch (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::DIFFUSE))
                                    {
                                    case SHADER_DATA_INPUT_TYPE::UNIFORM:
                                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Uniform diffuse colour for normals in texture is not implemented yet.";
                                        result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::BUFFER:
                                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse colour buffer for normals in texture is not implemented yet.";
                                        result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::TEXTURE:
                                        if (skin.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE) == 0UL)
                                        {
                                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse texture is not available.";
                                            result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::DIFFUSE] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                        }
                                        else
                                        {
                                            VS_IN const IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION = skin.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first;
                                            std::string const  IN_DIFFUSE_TEXCOORDS = name(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION);

                                            if (resources.skeletal() == nullptr)
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_IN::IN_TANGENT, vs_input),
                                                    varying(VS_IN::IN_BITANGENT, vs_input),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),

                                                    varying(VS_OUT::PASS_NORMAL, vs_output),
                                                    varying(VS_OUT::PASS_TANGENT, vs_output),
                                                    varying(VS_OUT::PASS_BITANGENT, vs_output),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                                    uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    "void main() {",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    PASS_NORMAL = (vec4(IN_NORMAL,0.0f) * MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_TANGENT = (vec4(IN_TANGENT,0.0f) * MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_BITANGENT = (vec4(IN_BITANGENT,0.0f) * MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "}",
                                                };
                                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                                    varying(VS_IN::IN_NORMAL, vs_input_instancing),
                                                    varying(VS_IN::IN_TANGENT, vs_input_instancing),
                                                    varying(VS_IN::IN_BITANGENT, vs_input_instancing),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input_instancing),

                                                    varying(VS_OUT::PASS_NORMAL, vs_output),
                                                    varying(VS_OUT::PASS_TANGENT, vs_output),
                                                    varying(VS_OUT::PASS_BITANGENT, vs_output),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                                    "void main() {",
                                                    "    gl_Position = vec4(IN_POSITION,1.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    PASS_NORMAL = (vec4(IN_NORMAL,0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_TANGENT = (vec4(IN_TANGENT,0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_BITANGENT = (vec4(IN_BITANGENT,0.0f) * IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA).xyz;",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                                    vs_get_version(),

                                                    vs_backward_compatibility_declarations(),

                                                    varying(VS_IN::IN_POSITION, vs_input),
                                                    varying(VS_IN::IN_NORMAL, vs_input),
                                                    varying(VS_IN::IN_TANGENT, vs_input),
                                                    varying(VS_IN::IN_BITANGENT, vs_input),
                                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                                    varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                                    varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),

                                                    varying(VS_OUT::PASS_NORMAL, vs_output),
                                                    varying(VS_OUT::PASS_TANGENT, vs_output),
                                                    varying(VS_OUT::PASS_BITANGENT, vs_output),
                                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),

                                                    uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                                    "void main() {",
                                                    "    uint i;",
                                                    "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_normal = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_tangent = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    vec3 result_bitangent = vec3(0.0f, 0.0f, 0.0f);",
                                                    "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                                    "    {",
                                                    "        vec4 pos = vec4(IN_POSITION,1.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                                    "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",

                                                    "        vec3 normal = (vec4(IN_NORMAL,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]).xyz;",
                                                    "        result_normal = result_normal + IN_WEIGHTS_OF_MATRICES[i] * normal;",

                                                    "        vec3 tangent = (vec4(IN_TANGENT,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]).xyz;",
                                                    "        result_tangent = result_tangent + IN_WEIGHTS_OF_MATRICES[i] * tangent;",

                                                    "        vec3 bitangent = (vec4(IN_BITANGENT,0.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]]).xyz;",
                                                    "        result_bitangent = result_bitangent + IN_WEIGHTS_OF_MATRICES[i] * bitangent;",
                                                    "    }",
                                                    "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                                    "    PASS_NORMAL = normalize(result_normal);",
                                                    "    PASS_TANGENT = normalize(result_tangent);",
                                                    "    PASS_BITANGENT = normalize(result_bitangent);",
                                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                                    "}",
                                                };
                                            }
                                            if (skin.alpha_testing().use_alpha_testing())
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_NORMAL, fs_input),
                                                    varying(FS_IN::PASS_TANGENT, fs_input),
                                                    varying(FS_IN::PASS_BITANGENT, fs_input),
                                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),
                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_NORMAL, fs_uniforms),
                                                    uniform(FS_UNIFORM::AMBIENT_COLOUR, fs_uniforms),
                                                    uniform(FS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, fs_uniforms),
                                                    uniform(FS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, fs_uniforms),
                                                    uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                                    "    if (diffuse_colour.a < ALPHA_TEST_CONSTANT)",
                                                    "        discard;",
                                                    "    vec3  N = normalize(2.0 * texture(TEXTURE_SAMPLER_NORMAL, PASS_TEXCOORD0).rgb - 1.0);",
                                                    "    vec3  normal = N.x * PASS_TANGENT + N.y * PASS_BITANGENT + N.z * PASS_NORMAL;",
                                                    "    vec4  colour_mult = ambient_and_directional_lighting(",
                                                    "        normalize(normal),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    OUT_COLOUR = colour_mult * diffuse_colour;",
                                                    "}",
                                                };
                                            }
                                            else
                                            {
                                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                                    fs_get_version(),

                                                    varying(FS_IN::PASS_NORMAL, fs_input),
                                                    varying(FS_IN::PASS_TANGENT, fs_input),
                                                    varying(FS_IN::PASS_BITANGENT, fs_input),
                                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),
                                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_NORMAL, fs_uniforms),
                                                    uniform(FS_UNIFORM::AMBIENT_COLOUR, fs_uniforms),
                                                    uniform(FS_UNIFORM::DIRECTIONAL_LIGHT_COLOUR, fs_uniforms),
                                                    uniform(FS_UNIFORM::DIRECTIONAL_LIGHT_DIRECTION, fs_uniforms),

                                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                                    "void main() {",
                                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                                    "    vec3  N = normalize(2.0 * texture(TEXTURE_SAMPLER_NORMAL, PASS_TEXCOORD0).rgb - 1.0);",
                                                    "    vec3  normal = N.x * PASS_TANGENT + N.y * PASS_BITANGENT + N.z * PASS_NORMAL;",
                                                    "    vec4  colour_mult = ambient_and_directional_lighting(",
                                                    "        normalize(normal),",
                                                    "        DIRECTIONAL_LIGHT_DIRECTION,",
                                                    "        AMBIENT_COLOUR,",
                                                    "        DIRECTIONAL_LIGHT_COLOUR",
                                                    "        );",
                                                    "    OUT_COLOUR = colour_mult * diffuse_colour;",
                                                    "}",
                                                };
                                            }
                                        }
                                        break;
                                    case SHADER_DATA_INPUT_TYPE::INSTANCE:
                                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Instancing of diffuse colours for normals in texture is not implemented yet.";
                                        result.second.get_lighting_data_types()[LIGHTING_DATA_TYPE::NORMAL] = SHADER_DATA_INPUT_TYPE::BUFFER;
                                        break;
                                    }
                                }
                            }
                            else
                            {
                                result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "The lighting data type NORMAL is assigned to neither BUFFER nor TEXTURE shader input type.";
                                result.second.get_lighting_data_types() = { { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE } };
                                result.second.get_light_types().clear();
                            }
                        }
                        else
                        {
                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "SPECULAR lighting data type is not supported yet.";
                            result.second.get_lighting_data_types().erase(gfx::LIGHTING_DATA_TYPE::SPECULAR);
                        }
                    }
                    else
                    {
                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "DIRECTION lighting data type is not assigned to UNIFOR shader input.";
                        result.second.get_lighting_data_types()[gfx::LIGHTING_DATA_TYPE::DIRECTION] = SHADER_DATA_INPUT_TYPE::UNIFORM;
                    }
                }
                else
                {
                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "The lighting data types does not contain all DIRECTION, NORMAL, and DIFFUSE.";
                    result.second.get_lighting_data_types() = { { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::TEXTURE } };
                    result.second.get_light_types().clear();
                }
            }
            else
            {
                if (effects.get_lighting_data_types().size() == 1UL &&
                    effects.get_lighting_data_types().count(LIGHTING_DATA_TYPE::DIFFUSE) != 0UL
                    )
                {
                    if (effects.get_lighting_data_types().at(LIGHTING_DATA_TYPE::DIFFUSE) == SHADER_DATA_INPUT_TYPE::TEXTURE)
                    {
                        if (skin.textures().count(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE) != 0UL)
                        {
                            VS_IN const IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION = skin.textures().at(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE).first;
                            std::string const  IN_DIFFUSE_TEXCOORDS = name(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION);

                            if (resources.skeletal() == nullptr)
                            {
                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                    vs_get_version(),

                                    vs_backward_compatibility_declarations(),

                                    varying(VS_IN::IN_POSITION, vs_input),
                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                    uniform(VS_UNIFORM::MATRIX_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                    "void main() {",
                                    "    mat4 T = MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                    "    PASS_DIFFUSE = vec4(AMBIENT_COLOUR, 1.0f);",
                                    "}",
                                };
                                vs_uid_instancing = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source_instancing = {
                                    vs_get_version(),

                                    vs_backward_compatibility_declarations(),

                                    varying(VS_IN::IN_POSITION, vs_input_instancing),
                                    varying(VS_IN::IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA, vs_input_instancing),
                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input_instancing),
                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms_instancing),
                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms_instancing),

                                    "void main() {",
                                    "    mat4 T = IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                    "    gl_Position = vec4(IN_POSITION,1.0f) * T;",
                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                    "    PASS_DIFFUSE = vec4(AMBIENT_COLOUR, 1.0f);",
                                    "}",
                                };
                            }
                            else
                            {
                                vs_uid = E2_GFX_GENERATE_VERTEX_SHADER_ID(); vs_source = {
                                    vs_get_version(),

                                    vs_backward_compatibility_declarations(),

                                    varying(VS_IN::IN_POSITION, vs_input),
                                    varying(IN_DIFFUSE_TEXCOORDS_BINDING_LOCATION, vs_input),
                                    varying(VS_IN::IN_INDICES_OF_MATRICES, vs_input),
                                    varying(VS_IN::IN_WEIGHTS_OF_MATRICES, vs_input),
                                    varying(VS_OUT::PASS_TEXCOORD0, vs_output),
                                    varying(VS_OUT::PASS_DIFFUSE, vs_output),

                                    uniform(VS_UNIFORM::AMBIENT_COLOUR, vs_uniforms),
                                    uniform(VS_UNIFORM::NUM_MATRICES_PER_VERTEX, vs_uniforms),
                                    uniform(VS_UNIFORM::MATRICES_FROM_MODEL_TO_CAMERA, vs_uniforms),
                                    uniform(VS_UNIFORM::MATRIX_FROM_CAMERA_TO_CLIPSPACE, vs_uniforms),

                                    DEFINE_FUNCTION_ambient_and_directional_lighting(),

                                    "void main() {",
                                    "    uint i;",
                                    "    vec4 result_position = vec4(0.0f, 0.0f, 0.0f, 0.0f);",
                                    "    for (i = 0U; i != NUM_MATRICES_PER_VERTEX; ++i)",
                                    "    {",
                                    "        vec4 pos = vec4(IN_POSITION,1.0f) * MATRICES_FROM_MODEL_TO_CAMERA[IN_INDICES_OF_MATRICES[i]];",
                                    "        result_position = result_position + IN_WEIGHTS_OF_MATRICES[i] * pos;",
                                    "    }",
                                    "    gl_Position = result_position * MATRIX_FROM_CAMERA_TO_CLIPSPACE;",
                                    "    PASS_TEXCOORD0 = " + IN_DIFFUSE_TEXCOORDS + ";",
                                    "    PASS_DIFFUSE = vec4(AMBIENT_COLOUR, 1.0f);",
                                    "}",
                                };
                            }
                            if (skin.alpha_testing().use_alpha_testing())
                            {
                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                    fs_get_version(),

                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),
                                    uniform(FS_UNIFORM::ALPHA_TEST_CONSTANT, fs_uniforms),

                                    "void main() {",
                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                    "    if (diffuse_colour.a < ALPHA_TEST_CONSTANT)",
                                    "        discard;",
                                    "    OUT_COLOUR = PASS_DIFFUSE * diffuse_colour;",
                                    "}",
                                };
                            }
                            else
                            {
                                fs_uid = E2_GFX_GENERATE_FRAGMENT_SHADER_ID(); fs_source = {
                                    fs_get_version(),

                                    varying(FS_IN::PASS_TEXCOORD0, fs_input),
                                    varying(FS_IN::PASS_DIFFUSE, fs_input),
                                    varying(FS_OUT::OUT_COLOUR, fs_output),

                                    uniform(FS_UNIFORM::TEXTURE_SAMPLER_DIFFUSE, fs_uniforms),

                                    "void main() {",
                                    "    vec4  diffuse_colour = texture(TEXTURE_SAMPLER_DIFFUSE, PASS_TEXCOORD0);",
                                    "    OUT_COLOUR = PASS_DIFFUSE * diffuse_colour;",
                                    "}",
                                };
                            }
                        }
                        else
                        {
                            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Diffuse texture is not available for Ambient and Diffuse lighting.";
                            result.second.get_lighting_data_types() = { { gfx::LIGHTING_DATA_TYPE::DIFFUSE, gfx::SHADER_DATA_INPUT_TYPE::BUFFER } };
                            result.second.get_light_types().clear();
                        }
                    }
                    else
                    {
                        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Ambient lighting is only implemented for Diffuse colour in Texture.";
                        result.second.get_light_types().clear();
                    }
                }
                else
                {
                    result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "The set of supported light types does not contain only AMBIENT.";
                    result.second.get_light_types().clear();
                }
            }
        }
        else
        {
            result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Light types are not empty and the AMBIENT ligt type is not present.";
            result.second.get_light_types().insert(LIGHT_TYPE::AMBIENT);
        }
    }
    else
    {
        result.first = E2_GFX_ERROR_MESSAGE_PREFIX() + "Output type from the shader is not SHADER_DATA_OUTPUT_TYPE::DEFAULT.";
        result.second.get_shader_output_types() = {SHADER_DATA_OUTPUT_TYPE::DEFAULT};
    }

    return result;
}


}}

namespace gfx {


shader_compose_result_type  compose_vertex_and_fragment_shader(
        batch_available_resources const  resources,
        std::string const&  skin_name,
        effects_config_data const&  effects,
        std::vector<std::string>&  vs_source,
        std::vector<std::string>&  vs_source_instancing,
        std::string&  vs_uid,
        std::string&  vs_uid_instancing,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input,
        std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>&  vs_input_instancing,
        std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>&  vs_output,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms,
        std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>&  vs_uniforms_instancing,
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
                    skin_name,
                    effects,
                    vs_source,
                    vs_source_instancing,
                    vs_uid,
                    vs_uid_instancing,
                    vs_input,
                    vs_input_instancing,
                    vs_output,
                    vs_uniforms,
                    vs_uniforms_instancing,
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
        detail::add_new_line_terminators(vs_source_instancing);
        detail::add_new_line_terminators(fs_source);
    }

    return result;
}


}
