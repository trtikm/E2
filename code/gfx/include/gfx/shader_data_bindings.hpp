#ifndef GFX_SHADER_DATA_BINDINGS_HPP_INCLUDED
#   define GFX_SHADER_DATA_BINDINGS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <unordered_set>
#   include <unordered_map>
#   include <string>

namespace gfx {


enum struct VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    IN_POSITION             = 0,    // vec3; always in model space
    IN_DIFFUSE              = 1,    // vec4; each component in range <0,1>
    IN_SPECULAR             = 2,    // vec4; RGB components are in range <0,1>; The alpha component is used as the coeficient for specular lighting.
    IN_NORMAL               = 3,    // vec3; Always in model space; normalised (unit vector)
    IN_TANGENT              = 4,    // vec3; Always in model space; normalised (unit vector)
    IN_BITANGENT            = 5,    // vec3; Always in model space; normalised (unit vector)
    IN_INDICES_OF_MATRICES  = 6,    // uint[VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::NUM_MATRICES_PER_VERTEX]; indices to the array VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA
    IN_WEIGHTS_OF_MATRICES  = 7,    // float[VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::NUM_MATRICES_PER_VERTEX]; weights (in range <0,1> each) of matrices in the array VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRICES_FROM_MODEL_TO_CAMERA
    IN_TEXCOORD0            = 8,    // vec2; texture uv coordinates
    IN_TEXCOORD1            = 9,    // vec2; texture uv coordinates
    IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA = 10,  // mat4=4*vec4; matrices for individual instances
    IN_INSTANCED_DIFFUSE_COLOUR              = 14,  // vec4; diffuse colours for individual instances

};

inline natural_32_bit  value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);
std::string  type_name(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);
inline natural_8_bit  get_num_texcoord_binding_locations()
{
    return value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD1) -
           value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0) + 1U;
}
inline VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  get_texcoord_binding_location(natural_8_bit const  texcood_index)
{
    return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION(
                value(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TEXCOORD0) + texcood_index
                );
}
inline constexpr VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  min_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION() noexcept
{ return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION; }
inline constexpr VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION  max_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION() noexcept
{ return VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INSTANCED_DIFFUSE_COLOUR; }
inline constexpr natural_8_bit  num_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATIONs() noexcept
{ return static_cast<natural_8_bit>(max_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION()) -
         static_cast<natural_8_bit>(min_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION()) +
         1U; }


enum struct VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    PASS_POSITION            = 0,    // vec3; always in camera space; without projection transformation
    PASS_DIFFUSE             = 1,    // vec4; each component in range <0,1>
    PASS_SPECULAR            = 2,    // vec4; The alpha component is used as the coeficient for specular lighting.
    PASS_NORMAL              = 3,    // vec3; Always in camera space; normalised (unit vector)
    PASS_TANGENT             = 4,    // vec3; Always in camera space; normalised (unit vector)
    PASS_BITANGENT           = 5,    // vec3; Always in camera space; normalised (unit vector)
    PASS_TEXCOORD0           = 6,    // vec2; texture uv coordinates
    PASS_TEXCOORD1           = 7,    // vec2; texture uv coordinates
};

inline natural_32_bit  value(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION const  location);
std::string  type_name(VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION const  location);


enum struct VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME : natural_8_bit
{
    MATRIX_FROM_MODEL_TO_CAMERA     = 0,    // mat4; from model space to the the camera space; without a projection transformation
    MATRIX_FROM_CAMERA_TO_CLIPSPACE = 1,    // mat4; a projection matrix
    MATRICES_FROM_MODEL_TO_CAMERA   = 2,    // mat4[64]; an array of matrices, each of the kind MATRIX_FROM_MODEL_TO_CAMERA
    NUM_MATRICES_PER_VERTEX         = 3,    // uint; says how many indices to the array MATRICES_FROM_MODEL_TO_CAMERA there are for each vertex (see VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_INDICES_OF_MATRICES/IN_WEIGHTS_OF_MATRICES)

    AMBIENT_COLOUR                  = 4,    // vec3; each component in range <0,1>
    DIFFUSE_COLOUR                  = 5,    // vec4; each component in range <0,1>
    SPECULAR_COLOUR                 = 6,    // vec4; first 3 components in range <0,1>; the alpha component is used as the coeficient for specular lighting.
    DIRECTIONAL_LIGHT_DIRECTION     = 7,    // vec3; unit vector in camera space
    DIRECTIONAL_LIGHT_COLOUR        = 8,    // vec3; each component in range <0,1>

    FOG_COLOUR                      = 9,    // vec4; first 3 components in range <0,1>; the alpha compunent is used as the fog decay coefficient, in range <1,3>.
    FOG_NEAR                        = 10,   // float; the distance where the fog starts; FOG_NEAR > 0.0
    FOG_FAR                         = 11,   // float; the distance where the fog is maximal; FOG_FAR > FOG_NEAR
};

inline natural_32_bit  value(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  name)
{ return static_cast<natural_8_bit>(name); }
std::string  name(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
std::string  type_name(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
natural_32_bit  num_elements(VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  to_symbolic_uniform_name_of_vertex_shader(std::string  name);
inline constexpr VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  min_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA; }
inline constexpr VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME  max_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_FAR; }
inline constexpr natural_8_bit  num_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAMEs()
{ return static_cast<natural_8_bit>(max_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()) -
         static_cast<natural_8_bit>(min_VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME()) +
         1U; }


enum struct FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION : natural_8_bit
{
    PASS_POSITION             = 0,    // vec3; Always in camera space; without a projection transformation
    PASS_DIFFUSE              = 1,    // vec4; each component in range <0,1>; Fragments with alpha component less than 0.5 are automatically discarded.
    PASS_SPECULAR             = 2,    // vec4; RGB components are in range <0,1>; The alpha component is used as the coeficient for specular lighting.
    PASS_NORMAL               = 3,    // vec3; Always in camera space; without a projection transformation
    PASS_TANGENT              = 4,    // vec3; Always in camera space; normalised (unit vector)
    PASS_BITANGENT            = 5,    // vec3; Always in camera space; normalised (unit vector)
    PASS_TEXCOORD0            = 6,    // vec2; texture uv coordinates
    PASS_TEXCOORD1            = 7,    // vec2; texture uv coordinates
};

inline natural_32_bit  value(FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);
std::string  type_name(FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION const  location);


enum struct FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION : natural_8_bit
{
    OUT_COLOUR              = 0,    // vec4; each component in range <0,1>; The default output to the framgment buffer (screen)
    OUT_TEXTURE_POSITION    = 1,    // vec3; always in camera space; For sampling to output G-buffer
    OUT_TEXTURE_NORMAL      = 2,    // vec3; always in camera space; normalised (unit) vector; For sampling to output G-buffer
    OUT_TEXTURE_DIFFUSE     = 3,    // vec4; each component in range <0,1>; For sampling to output G-buffer
    OUT_TEXTURE_SPECULAR    = 4,    // vec4; RGB components in range <0,1>; The alpha component is used as the coeficient for specular lighting; For sampling to output G-buffer
};

inline natural_32_bit  value(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const  location)
{ return static_cast<natural_8_bit>(location); }
std::string  name(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const  location);
std::string  type_name(FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION const  location);


enum struct FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME : natural_8_bit
{
    TEXTURE_SAMPLER_DIFFUSE         = 0,    // sampler2D; For sampling to diffuse input texture.
    TEXTURE_SAMPLER_SPECULAR        = 1,    // sampler2D; For sampling to specular input texture.
    TEXTURE_SAMPLER_NORMAL          = 2,    // sampler2D; For sampling to normals input texture.

    AMBIENT_COLOUR                  = 3,    // vec3; each component in range <0,1>
    DIFFUSE_COLOUR                  = 4,    // vec4; each component in range <0,1>
    SPECULAR_COLOUR                 = 5,    // vec4; first 3 components in range <0,1>; the alpha component is used as the coeficient for specular lighting.
    DIRECTIONAL_LIGHT_DIRECTION     = 6,    // vec3; unit vector in camera space
    DIRECTIONAL_LIGHT_COLOUR        = 7,    // vec3; each component in range <0,1>

    FOG_COLOUR                      = 8,    // vec4; first 3 components in range <0,1>; the alpha compunent is used as the fog decay coefficient, in range <1,3>.
    FOG_NEAR                        = 9,    // float; the distance where the fog starts; FOG_NEAR > 0.0
    FOG_FAR                         = 10,   // float; the distance where the fog is maximal; FOG_FAR > FOG_NEAR

    ALPHA_TEST_CONSTANT             = 11,   // float; in range <0,1>; fragments with the alpha component less than values of this uniform will be discarded.
};

inline natural_32_bit  value(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  binding)
{ return static_cast<natural_8_bit>(binding); }
std::string  name(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform_symbolic_name);
std::string  type_name(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
natural_32_bit  num_elements(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  symbolic_name);
bool  is_texture_sampler(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  uniform_symbolic_name);
FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  to_symbolic_uniform_name_of_fragment_shader(std::string  name);
inline constexpr FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  min_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE; }
inline constexpr FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  max_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()
{ return FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::FOG_FAR; }
inline constexpr natural_8_bit  num_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAMEs() noexcept
{ return static_cast<natural_8_bit>(max_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()) -
         static_cast<natural_8_bit>(min_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()) +
         1U; }


bool  compatible(std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION> const& vertex_program_output,
                 std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION> const& fragment_program_input);


using  texcoord_binding = std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME,VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>;


}

#endif
