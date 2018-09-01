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
        async::finalise_load_on_destroy_ptr  finaliser,
        boost::filesystem::path const&  path,
        effects_config const&  effects,
        std::string const&  skeleton_name
        )
{
    TMPROF_BLOCK();

    m_available_resources.insert_load_request(
            path.string(),
            { std::bind(&batch_data::load, this, effects, skeleton_name, std::placeholders::_1), finaliser }
            );
}


batch_data::batch_data(
        async::finalise_load_on_destroy_ptr,
        buffers_binding const  buffers_binding_,
        textures_binding const  textures_binding_,
        texcoord_binding const&  texcoord_binding_,
        effects_config const&  effects,
        draw_state_ptr const  draw_state_,
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
    std::string  vs_uid;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  vs_output;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms;
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
    shaders_binding const  shaders_binding_{
        vertex_shader{vs_input, vs_output, vs_uniforms, vs_source, vs_uid},
        fragment_shader{fs_input, fs_output, fs_uniforms, fs_source, fs_uid},
        "{" + vs_uid + "}{" + fs_uid + "}"
    };
    initialise(buffers_binding_,shaders_binding_,textures_binding_,draw_state_,modelspace_,skeleton_alignment_,resources);
}


batch_data::~batch_data()
{
    TMPROF_BLOCK();
}


void  batch_data::load(
        effects_config const&  effects,
        std::string const&  skeleton_name,
        async::finalise_load_on_destroy_ptr  finaliser
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(get_available_resources().loaded_successfully());

    batch_available_resources::skeletal_dictionary_type::const_iterator  skeletal_info =
        get_available_resources().skeletal().cend();
    if (skeleton_name.empty() && !get_available_resources().skeletal().empty())
    {
        if (get_available_resources().skeletal().size() != 1UL)
            throw std::runtime_error(
                    msgstream() << "Passed empty skeleton name to batch '"
                                << get_available_resources().root_dir()
                                << "' associated with multiple skeletons.");
        skeletal_info = get_available_resources().skeletal().cbegin();
    }
    else
        for (auto  skeletal_info = get_available_resources().skeletal().cbegin();
                skeletal_info != get_available_resources().skeletal().cend();
                ++skeletal_info)
            if (boost::filesystem::path(skeletal_info->first).filename().string() == skeleton_name)
                break;

    std::vector<std::string>  vs_source;
    std::string  vs_uid;
    std::unordered_set<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION>  vs_input;
    std::unordered_set<VERTEX_SHADER_OUTPUT_BUFFER_BINDING_LOCATION>  vs_output;
    std::unordered_set<VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME>  vs_uniforms;
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
            INVARIANT(skeletal_info != get_available_resources().skeletal().cend());
            buffer_paths.insert({location, skeletal_info->second.indices()});
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_WEIGHTS_OF_MATRICES:
            INVARIANT(skeletal_info != get_available_resources().skeletal().cend());
            buffer_paths.insert({location, skeletal_info->second.weights()});
            break;
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD0:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD1:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD2:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD3:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD4:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD5:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD6:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD7:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD8:
        case VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TEXCOORD9:
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
        default:
            UNREACHABLE();
        }
    }

    initialise(
        buffers_binding(
            get_available_resources().index_buffer(),
            buffer_paths,
            get_available_resources().root_dir()
            ),
        shaders_binding{
            vertex_shader{vs_input, vs_output, vs_uniforms, vs_source, vs_uid},
            fragment_shader{fs_input, fs_output, fs_uniforms, fs_source, fs_uid},
            "{" + vs_uid + "}{" + fs_uid + "}"
            },
        textures_binding(texture_paths),
        draw_state::create(GL_BACK, false, 0U, 0U),
        skeletal_info == get_available_resources().skeletal().cend() ?
            modelspace() :
            modelspace(boost::filesystem::path(skeletal_info->first) / "pose.txt"),
        skeletal_info == get_available_resources().skeletal().cend() ?
            skeleton_alignment() :
            skeleton_alignment(skeletal_info->second.alignment()),
        get_available_resources()
        );
}


void  batch_data::initialise(
        buffers_binding const  buffers_binding_,
        shaders_binding const  shaders_binding_,
        textures_binding const  textures_binding_,
        draw_state_ptr const  draw_state_,
        modelspace const  modelspace_,
        skeleton_alignment const  skeleton_alignment_,
        batch_available_resources const  available_resources_
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
            (is_attached_to_skeleton() && (
                !get_modelspace().loaded_successfully() ||
                !get_skeleton_alignment().loaded_successfully() )))
            return false;

        const_cast<batch*>(this)->set_ready();
    }

    return true;
}


bool  batch::make_current(draw_state const* const  previous_state) const
{
    TMPROF_BLOCK();

    if (!ready())
        return false;

    bool  result;

    result = get_buffers_binding().make_current();
    INVARIANT(result == true);

    result = get_shaders_binding().make_current();
    INVARIANT(result == true);

    result = get_textures_binding().make_current();
    INVARIANT(result == true);

    if (previous_state != nullptr)
        qtgl::make_current(*get_draw_state(), *previous_state);
    else
        qtgl::make_current(*get_draw_state());

    return true;
}


bool  make_current(batch const&  batch)
{
    return batch.make_current(nullptr);
}

bool  make_current(batch const&  batch, draw_state const&  previous_state)
{
    return batch.make_current(&previous_state);
}


}
