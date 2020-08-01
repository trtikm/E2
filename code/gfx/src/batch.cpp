#include <gfx/batch.hpp>
#include <gfx/shader_data_bindings.hpp>
#include <gfx/shader_compose.hpp>
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

namespace gfx { namespace detail {


batch_data::~batch_data()
{
    TMPROF_BLOCK();
}


batch_data::batch_data(
        async::finalise_load_on_destroy_ptr const  finaliser,
        buffers_binding const  buffers_binding_,
        textures_binding const  textures_binding_,
        texcoord_binding const&  texcoord_binding_,
        effects_config const  effects_,
        draw_state const  draw_state_,
        modelspace const  modelspace_,
        skeleton_alignment const  skeleton_alignment_,
        batch_available_resources const  resources_,
        std::string const&  skin_name_,
        std::string const&  id_
        )
    : m_buffers_binding(buffers_binding_)
    , m_shaders_binding()
    , m_textures_binding(textures_binding_)
    , m_effects_config(effects_)
    , m_draw_state(draw_state_)
    , m_modelspace(modelspace_)
    , m_skeleton_alignment(skeleton_alignment_)
    , m_available_resources(resources_)
    , m_instancing_data()
    , m_skin_name(skin_name_)
    , m_id(id_)
    , m_ready(false)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        !m_available_resources.empty() &&
        !m_available_resources.skins().empty() &&
        m_available_resources.skins().find(m_skin_name) != m_available_resources.skins().end()
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
    shader_compose_result_type  result{"", get_effects_config().resource_const()};
    while (true)
    {
        effects_config_data const  old_effects = result.second;
        result = compose_vertex_and_fragment_shader(
                        get_available_resources(),
                        get_skin_name(),
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
    m_effects_config = effects_config::make(result.second);

    fragment_shader const  frag_shader{ fs_input, fs_output, fs_uniforms, fs_source, fs_uid, finaliser };

    if (!vs_source.empty())
        m_shaders_binding =
            {
                vertex_shader{vs_input, vs_output, vs_uniforms, vs_source, vs_uid, finaliser },
                frag_shader,
                "{" + vs_uid + "}{" + fs_uid + "}",
                finaliser
            };

    batch_instancing_data  instancing_data;
    if (m_available_resources.skeletal() == nullptr && !vs_source_instancing.empty())
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

    INVARIANT(!m_shaders_binding.empty() || !instancing_data.m_shaders_binding.empty());

    m_instancing_data = make_instancing_data_from(instancing_data);
}


void  batch_data::load(async::finalise_load_on_destroy_ptr const  finaliser)
{
    TMPROF_BLOCK();

    ASSUMPTION(
        get_available_resources().loaded_successfully() &&
        !m_available_resources.empty() &&
        !m_available_resources.skins().empty() &&
        m_available_resources.skins().find(m_skin_name) != m_available_resources.skins().end()
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
    shader_compose_result_type  result{"", get_effects_config().resource_const()};
    while (true)
    {
        effects_config_data const  old_effects = result.second;
        result = compose_vertex_and_fragment_shader(
                        get_available_resources(),
                        get_skin_name(),
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
    m_effects_config = effects_config::make(result.second);

    fragment_shader const  frag_shader(fs_input, fs_output, fs_uniforms, fs_source, fs_uid, finaliser);

    if (!vs_source.empty())
        m_shaders_binding =
            {
                vertex_shader(vs_input, vs_output, vs_uniforms, vs_source, vs_uid, finaliser),
                frag_shader,
                "{" + vs_uid + "}{" + fs_uid + "}",
                finaliser
            };

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
                for (auto const&  elem : get_available_resources().skins().at(get_skin_name()).textures())
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

    std::string  buffers_binding_uid;
    {
        msgstream  sstr;
        sstr << "PATH="
             << (boost::filesystem::path(get_available_resources().data_root_dir()) / get_available_resources().mesh_path()).string()
             << ",BUFFERS=";
        std::set<std::string>  names;
        for (auto const& x : buffer_paths)
            names.insert(name(x.first).substr(11)); // 11 == len("BINDING_IN_");
        for (auto const&  name : names)
            sstr << name << ",";
        buffers_binding_uid = sstr.get();
    }

    m_buffers_binding = 
            get_available_resources().has_index_buffer() ?
                    buffers_binding(
                        (boost::filesystem::path(get_available_resources().data_root_dir())
                            / get_available_resources().mesh_path()
                            / "indices.txt"
                            ).string(),
                        buffer_paths,
                        buffers_binding_uid,
                        finaliser
                        )
                    :
                    buffers_binding(
                        get_available_resources().num_indices_per_primitive(),
                        buffer_paths,
                        buffers_binding_uid,
                        finaliser
                        )
                    ;

    std::string  textures_binding_uid;
    {
        std::string const  root_dir = boost::filesystem::path(get_available_resources().data_root_dir()).normalize().string();
        std::set<std::string>  names;
        for (auto const& x : texture_paths)
        {
            std::string  p = boost::filesystem::path(x.second).normalize().string();
            if (p.find(root_dir) == 0UL)
                p = p.substr(root_dir.size());
            names.insert(msgstream() << name(x.first).substr(16) << "=" << p); // 16 == len("TEXTURE_SAMPLER_");
        }
        msgstream  sstr;
        for (auto const& name : names)
            sstr << name << ",";
        textures_binding_uid = sstr.get();
    }

    m_textures_binding = textures_binding(texture_paths, textures_binding_uid, finaliser);

    m_draw_state = get_available_resources().skins().at(get_skin_name()).get_draw_state();
    m_modelspace = 
            get_available_resources().skeletal() == nullptr ?
                    modelspace()
                    :
                    modelspace(
                        boost::filesystem::path(get_available_resources().data_root_dir())
                        / get_available_resources().skeletal()->animation_dir()
                        / "pose.txt",
                        10U,
                        finaliser)
                    ;
    m_skeleton_alignment =
            get_available_resources().skeletal() == nullptr ?
                    skeleton_alignment()
                    :
                    skeleton_alignment(get_available_resources().skeletal()->alignment(), finaliser)
                    ;

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

    INVARIANT(!m_shaders_binding.empty() || !instancing_data.m_shaders_binding.empty());

    m_instancing_data = make_instancing_data_from(instancing_data);
}

bool  batch_data::is_translucent() const
{
    if (!get_draw_state().loaded_successfully())
        return false;
    return get_draw_state().use_alpha_blending() ||
           get_available_resources().skins().at(get_skin_name()).alpha_testing().use_alpha_testing();
}


}}

namespace gfx {


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
        result = gfx::make_current(get_draw_state(), previous_state);
    else
        result = gfx::make_current(get_draw_state());
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
