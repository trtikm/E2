#include <qtgl/batch.hpp>
#include <qtgl/shader_data_bindings.hpp>
#include <qtgl/shader_compose.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <boost/functional/hash.hpp>
#include <limits>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <iterator>
#include <unordered_set>
#include <unordered_map>

namespace qtgl { namespace detail {


batch_data::batch_data(
        async::finalise_load_on_destroy_ptr const  finaliser,
        boost::filesystem::path const&  path,
        effects_config const&  effects
        )
{
    TMPROF_BLOCK();

    async::finalise_load_on_destroy_ptr const  available_resources_finaliser =
        async::finalise_load_on_destroy::create(
                std::bind(&batch_data::load, this, effects, std::placeholders::_1),
                finaliser
                );
    m_available_resources.insert_load_request(path.string(), available_resources_finaliser);
}


batch_data::batch_data(
        async::finalise_load_on_destroy_ptr const  finaliser,
        buffers_binding const  buffers_binding_,
        textures_binding const  textures_binding_,
        texcoord_binding const&  texcoord_binding_,
        effects_config const&  effects,
        draw_state const  draw_state_,
        modelspace const  modelspace_,
        skeleton_alignment const  skeleton_alignment_
        )
{
    TMPROF_BLOCK();

    batch_available_resources const  resources(
            buffers_binding_.get_buffers(),
            textures_binding_.empty() ? textures_binding::binding_map_type{} : textures_binding_.bindings_map(),
            texcoord_binding_
            );
    std::vector<std::string>  vs_source;
    std::vector<std::string>  vs_source_instancing;
    std::string  vs_uid;
    std::string  vs_uid_instancing;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input_instancing;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  vs_output;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms_instancing;
    std::vector<std::string>  fs_source;
    std::string  fs_uid;
    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>  fs_input;
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>  fs_output;
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>  fs_uniforms;
    shader_compose_result_type  result{"", effects};
    while (true)
    {
        effects_config const  old_effects = result.second;
        result = compose_vertex_and_fragment_shader(
                        resources,
                        old_effects,
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
        if (old_effects == result.second)   // Was the current configuration successfull? I.e., was it not necessary
                                            // to generate another to result.second?
        {
            if (!result.first.empty())
            {
                UNREACHABLE();  // We got to some errorneour effects configuration from which we cannot recover.
                                // That should never happen.
            }
            break;
        }
        INVARIANT(!result.first.empty());   // The current configuration failed, so new one was proposed (to result.second)
                                            // and the error message (in result.first) must have been produced.
        result.first.clear();
    }

    fragment_shader const  frag_shader{ fs_input, fs_output, fs_uniforms, fs_source, fs_uid, finaliser };

    shaders_binding  shaders_binding_;
    if (!vs_source.empty())
        shaders_binding_ =
            {
                vertex_shader{vs_input, vs_output, vs_uniforms, vs_source, vs_uid, finaliser },
                frag_shader,
                "{" + vs_uid + "}{" + fs_uid + "}",
                finaliser
            };

    batch_instancing_data  instancing_data;
    if (resources.skeletal() == nullptr && !vs_source_instancing.empty())
    {
        for (auto const& location : vs_input_instancing)
        {
            switch (location)
            {
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA:
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_DIFFUSE_COLOUR:
                instancing_data.m_buffers.insert(location);
                break;
            }
        }
        if (!instancing_data.m_buffers.empty())
            instancing_data.m_shaders_binding =
                {
                    vertex_shader{ vs_input_instancing, vs_output, vs_uniforms_instancing, vs_source_instancing, vs_uid_instancing, finaliser },
                    frag_shader,
                    "{" + vs_uid_instancing + "}{" + fs_uid + "}",
                    finaliser
                };
    }

    INVARIANT(!shaders_binding_.empty() || !instancing_data.m_shaders_binding.empty());

    initialise(
            buffers_binding_,
            shaders_binding_,
            textures_binding_,
            draw_state_,
            modelspace_,
            skeleton_alignment_,
            resources,
            instancing_data
            );
}


batch_data::~batch_data()
{
    TMPROF_BLOCK();
}


void  batch_data::load(
        effects_config const&  effects,
        async::finalise_load_on_destroy_ptr const  finaliser
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(get_available_resources().loaded_successfully());

    std::vector<std::string>  vs_source;
    std::vector<std::string>  vs_source_instancing;
    std::string  vs_uid;
    std::string  vs_uid_instancing;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input_instancing;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  vs_output;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms_instancing;
    std::vector<std::string>  fs_source;
    std::string  fs_uid;
    std::unordered_set<FRAGMENT_SHADER_INPUT_BUFFER_BINDING_LOCATION>  fs_input;
    std::unordered_set<FRAGMENT_SHADER_OUTPUT_BINDING_LOCATION>  fs_output;
    std::unordered_set<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME>  fs_uniforms;
    shader_compose_result_type  result{"", effects};
    while (true)
    {
        effects_config const  old_effects = result.second;
        result = compose_vertex_and_fragment_shader(
                        get_available_resources(),
                        old_effects,
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
        if (old_effects == result.second)   // Was the current configuration successfull? I.e., was it not necessary
                                            // to generate another to result.second?
        {
            if (!result.first.empty())
            {
                UNREACHABLE();  // We got to some errorneour effects configuration from which we cannot recover.
                                // That should never happen.
            }
            break;
        }
        INVARIANT(!result.first.empty());   // The current configuration failed, so new one was proposed (to result.second)
                                            // and the error message (in result.first) must have been produced.
        result.first.clear();
    }

    std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, boost::filesystem::path>  buffer_paths;
    std::unordered_map<FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME, boost::filesystem::path>  texture_paths;
    for (auto const& location : vs_input)
    {
        switch (location)
        {
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_SPECULAR:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_NORMAL:
            buffer_paths.insert({location, get_available_resources().buffers().at(location)});
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TANGENT:
            buffer_paths.insert({ location, get_available_resources().buffers().at(location) });
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_BITANGENT:
            buffer_paths.insert({ location, get_available_resources().buffers().at(location) });
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INDICES_OF_MATRICES:
            buffer_paths.insert({location, get_available_resources().skeletal()->indices()});
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_WEIGHTS_OF_MATRICES:
            buffer_paths.insert({location, get_available_resources().skeletal()->weights()});
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD1:
            {
                buffer_paths.insert({location, get_available_resources().buffers().at(location)});
                bool  used = false;
                for (auto const&  elem : get_available_resources().textures())
                    if (fs_uniforms.count(elem.first) != 0UL && elem.second.first == location)
                    {
                        texture_paths.insert({elem.first, elem.second.second});
                        used = true;
                    }
                INVARIANT(used == true);
            }
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_DIFFUSE_COLOUR:
            UNREACHABLE();
            break;
        default:
            UNREACHABLE();
        }
    }

    fragment_shader const  frag_shader(fs_input, fs_output, fs_uniforms, fs_source, fs_uid, finaliser);

    shaders_binding  shaders_binding_;
    if (!vs_source.empty())
        shaders_binding_ =
            {
                vertex_shader(vs_input, vs_output, vs_uniforms, vs_source, vs_uid, finaliser),
                frag_shader,
                "{" + vs_uid + "}{" + fs_uid + "}",
                finaliser
            };

    batch_instancing_data  instancing_data;
    if (get_available_resources().skeletal() == nullptr && !vs_source_instancing.empty())
    {
        for (auto const& location : vs_input_instancing)
        {
            switch (location)
            {
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_MATRIX_FROM_MODEL_TO_CAMERA:
            case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_INSTANCED_DIFFUSE_COLOUR:
                instancing_data.m_buffers.insert(location);
                break;
            }
        }
        if (!instancing_data.m_buffers.empty())
            instancing_data.m_shaders_binding =
                {
                    vertex_shader{ vs_input_instancing, vs_output, vs_uniforms_instancing, vs_source_instancing, vs_uid_instancing, finaliser },
                    frag_shader,
                    "{" + vs_uid_instancing + "}{" + fs_uid + "}",
                    finaliser
                };
    }

    INVARIANT(!shaders_binding_.empty() || !instancing_data.m_shaders_binding.empty());

    initialise(
        get_available_resources().has_index_buffer() ?
            buffers_binding(
                (boost::filesystem::path(get_available_resources().data_root_dir())
                    / get_available_resources().mesh_path()
                    / "indices.txt"
                    ).string(),
                buffer_paths,
                (boost::filesystem::path(get_available_resources().data_root_dir())
                    / get_available_resources().mesh_path()
                    ).string(),
                finaliser
                ):
            buffers_binding(
                get_available_resources().num_indices_per_primitive(),
                buffer_paths,
                (boost::filesystem::path(get_available_resources().data_root_dir())
                    / get_available_resources().mesh_path()
                    ).string(),
                finaliser
                ),
        shaders_binding_,
        textures_binding(texture_paths, "", finaliser),
        get_available_resources().get_draw_state(),
        get_available_resources().skeletal() == nullptr ?
            modelspace() :
            modelspace(
                boost::filesystem::path(get_available_resources().data_root_dir())
                    / "animations"
                    / "skeletal"
                    / get_available_resources().skeletal()->skeleton_name()
                    / "pose.txt",
                finaliser),
        get_available_resources().skeletal() == nullptr ?
            skeleton_alignment() :
            skeleton_alignment(get_available_resources().skeletal()->alignment(), finaliser),
        get_available_resources(),
        instancing_data
        );
}


void  batch_data::initialise(
        buffers_binding const  buffers_binding_,
        shaders_binding const  shaders_binding_,
        textures_binding const  textures_binding_,
        draw_state const  draw_state_,
        modelspace const  modelspace_,
        skeleton_alignment const  skeleton_alignment_,
        batch_available_resources const  available_resources_,
        batch_instancing_data const&  instancing_data
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(modelspace_.empty() == skeleton_alignment_.empty());

    m_buffers_binding = buffers_binding_;
    m_shaders_binding = shaders_binding_;
    m_textures_binding = textures_binding_;
    m_draw_state = draw_state_;
    m_modelspace = modelspace_;
    m_skeleton_alignment = skeleton_alignment_;
    m_available_resources = available_resources_;
    m_instancing_data = instancing_data.m_buffers.empty() || instancing_data.m_shaders_binding.empty() ?
                                nullptr : std::make_unique<batch_instancing_data>(instancing_data);
    m_ready = false;
}


}}

namespace qtgl {


bool  batch::ready() const
{
    TMPROF_BLOCK();

    if (!loaded_successfully())
        return false;
    if (!resource().ready())
    {
        if (!get_buffers_binding().ready() ||
            !get_shaders_binding().ready() ||
            !get_textures_binding().ready() ||
            !get_draw_state().loaded_successfully() ||
            (is_attached_to_skeleton() && (
                !get_modelspace().loaded_successfully() ||
                !get_skeleton_alignment().loaded_successfully() )) ||
            (has_instancing_data() && !get_instancing_data_ptr()->m_shaders_binding.ready()))
            return false;

        const_cast<batch*>(this)->set_ready();
    }

    return true;
}


bool  batch::make_current(draw_state const&  previous_state, bool  for_instancing) const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;

    bool  result;

    result = get_buffers_binding().make_current();
    INVARIANT(result == true);

    ASSUMPTION((!for_instancing || has_instancing_data()) && (for_instancing || !get_shaders_binding().empty()));
    result = for_instancing ? get_instancing_data_ptr()->m_shaders_binding.make_current() :
                              get_shaders_binding().make_current();
    INVARIANT(result == true);

    result = get_textures_binding().make_current();
    INVARIANT(result == true);

    if (previous_state.loaded_successfully())
        result = qtgl::make_current(get_draw_state(), previous_state);
    else
        result = qtgl::make_current(get_draw_state());
    INVARIANT(result == true);

    return true;
}


bool  make_current(batch const&  batch, bool const  for_instancing)
{
    return batch.make_current(draw_state(), for_instancing);
}

bool  make_current(batch const&  batch, draw_state const&  previous_state, bool const  for_instancing)
{
    return batch.make_current(previous_state, for_instancing);
}


}
