#ifndef GFX_SHADER_COMPOSE_HPP_INCLUDED
#   define GFX_SHADER_COMPOSE_HPP_INCLUDED

#   include <gfx/shader_data_bindings.hpp>
#   include <gfx/batch_available_resources.hpp>
#   include <gfx/effects_config.hpp>
#   include <unordered_set>
#   include <string>
#   include <vector>
#   include <tuple>

namespace gfx {


using  shader_compose_result_type = std::pair<std::string, effects_config_data>;


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
        );


}

#endif
