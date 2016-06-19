#include <qtgl/batch.hpp>
#include <qtgl/shader_data_bindings.hpp>
#include <qtgl/detail/batch_cache.hpp>
#include <qtgl/detail/read_line.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
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

namespace qtgl {


std::unordered_set<vertex_shader_uniform_symbolic_name>  batch::s_empty_uniforms;

std::shared_ptr<batch const>  batch::create(boost::filesystem::path const&  path)
{
    return std::make_shared<batch const>(path);
}

batch::batch(boost::filesystem::path const&  path)
    : m_path(path)
    , m_buffers_binding()
    , m_shaders_binding()
    , m_textures_binding()
    , m_draw_state()
{
    ASSUMPTION(!m_path.empty());
    insert_load_request(*this);
}

batch::batch(boost::filesystem::path const&  path,
             buffers_binding_ptr const  buffers_binding,
             shaders_binding_ptr const  shaders_binding,
             textures_binding_ptr const  textures_binding,
             draw_state_ptr const  draw_state
             )
    : m_path(path)
    , m_buffers_binding(buffers_binding)
    , m_shaders_binding(shaders_binding)
    , m_textures_binding(textures_binding)
    , m_draw_state(draw_state)
{
    ASSUMPTION(m_buffers_binding.operator bool());
    ASSUMPTION(m_shaders_binding.operator bool());
    ASSUMPTION(m_textures_binding.operator bool());
    ASSUMPTION(m_draw_state.operator bool());
}

buffers_binding_ptr  batch::buffers_binding() const
{
    if (!m_buffers_binding.operator bool())
    {
        batch_ptr const  pbatch = detail::batch_cache::instance().find(path());
        if (pbatch.operator bool())
            m_buffers_binding = pbatch->m_buffers_binding;
    }
    return m_buffers_binding;
}

shaders_binding_ptr  batch::shaders_binding() const
{
    if (!m_shaders_binding.operator bool())
    {
        batch_ptr const  pbatch = detail::batch_cache::instance().find(path());
        if (pbatch.operator bool())
            m_shaders_binding =  pbatch->m_shaders_binding;
    }
    return m_shaders_binding;
}

textures_binding_ptr  batch::textures_binding() const
{
    if (!m_textures_binding.operator bool())
    {
        batch_ptr const  pbatch = detail::batch_cache::instance().find(path());
        if (pbatch.operator bool())
            m_textures_binding = pbatch->m_textures_binding;
    }
    return m_textures_binding;
}

draw_state_ptr  batch::draw_state() const
{
    if (!m_draw_state.operator bool())
    {
        batch_ptr const  pbatch = detail::batch_cache::instance().find(path());
        if (pbatch.operator bool())
            m_draw_state = pbatch->m_draw_state;
    }
    return m_draw_state;
}

std::unordered_set<vertex_shader_uniform_symbolic_name> const&  batch::symbolic_names_of_used_uniforms() const
{
    return shaders_binding().operator bool() && shaders_binding()->binding_vertex_program_props().operator bool() ?
                shaders_binding()->binding_vertex_program_props()->symbolic_names_of_used_uniforms() :
                s_empty_uniforms ;
}

batch_ptr  load_batch_file(boost::filesystem::path const&  batch_file, std::string&  error_message)
{
    TMPROF_BLOCK();

    ASSUMPTION(error_message.empty());

    if (!boost::filesystem::exists(batch_file))
    {
        error_message = msgstream() << "The batch file '" << batch_file << "' does not exist.";
        return {};
    }
    if (!boost::filesystem::is_regular_file(batch_file))
    {
        error_message = msgstream() << "The batch path '" << batch_file << "' does not reference a regular file.";
        return {};
    }

    if (boost::filesystem::file_size(batch_file) < 4ULL)
    {
        error_message = msgstream() << "The passed file '" << batch_file << "' is not a qtgl file (wrong size).";
        return {};
    }

    std::ifstream  istr(batch_file.string(),std::ios_base::binary);
    if (!istr.good())
    {
        error_message = msgstream() << "Cannot open the batch file '" << batch_file << "'.";
        return {};
    }

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
    {
        error_message = msgstream() << "The passed file '" << batch_file << "' is not a qtgl file (cannot read its type string).";
        return {};
    }

    if (file_type == "E2::qtgl/batch/indexed/text")
    {
        std::string  line;

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to vertex shader file in the file '" << batch_file << "'.";
            return {};
        }
        boost::filesystem::path  vertex_shader = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(vertex_shader) || !boost::filesystem::is_regular_file(vertex_shader))
        {
            error_message = msgstream() << "The vertex shader file '" << vertex_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }
        vertex_shader = boost::filesystem::canonical(vertex_shader);

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to fragment shader file in the file '" << batch_file << "'.";
            return {};
        }
        boost::filesystem::path  fragment_shader = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(fragment_shader) || !boost::filesystem::is_regular_file(fragment_shader))
        {
            error_message = msgstream() << "The fragment shader file '" << fragment_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }
        fragment_shader = boost::filesystem::canonical(fragment_shader);

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to the index buffer file in the file '" << batch_file << "'.";
            return {};
        }
        boost::filesystem::path  index_buffer = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(index_buffer) || !boost::filesystem::is_regular_file(index_buffer))
        {
            error_message = msgstream() << "The fragment shader file '" << fragment_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }
        index_buffer = boost::filesystem::canonical(index_buffer);

        std::unordered_map<vertex_shader_input_buffer_binding_location,boost::filesystem::path>  buffer_paths;
        while (true)
        {
            if (!detail::read_line(istr,line))
                break;
            natural_8_bit  location = value(min_vertex_shader_input_buffer_binding_location());
            bool  found = false;
            for ( ; location <= value(max_vertex_shader_input_buffer_binding_location()); ++location)
                if (line == binding_location_name(vertex_shader_input_buffer_binding_location(location)))
                {
                    found = true;
                    break;
                }
            if (!found)
            {
                if (line.find("BINDING_TEXTURE_") == 0ULL)
                    break;

                error_message = msgstream() << "Unknown vertex shader input buffer binding location '" << line
                                            << "' in the file '" << batch_file << "'.";
                return {};
            }
            vertex_shader_input_buffer_binding_location const  bind_location = vertex_shader_input_buffer_binding_location(location);
            if (buffer_paths.count(bind_location) != 0ULL)
            {
                error_message = msgstream() << "Vertex shader input buffer binding location '" << line
                                            << "' appears more than once in the file '" << batch_file << "'.";
                return {};
            }

            if (!detail::read_line(istr,line))
            {
                error_message = msgstream() << "Cannot read a path to a buffer file to be bound at location '" << line
                                            << "' in the file '" << batch_file << "'.";
                return {};
            }
            boost::filesystem::path  buffer_file = boost::filesystem::absolute(batch_file.parent_path() / line);
            if (!boost::filesystem::exists(buffer_file) || !boost::filesystem::is_regular_file(buffer_file))
            {
                error_message = msgstream() << "The buffer file '" << buffer_file.string()
                                            << "' to be bound to location '" << binding_location_name(bind_location)
                                            << "' referenced from the batch file '" << batch_file << "' does not exist.";
                return {};
            }
            buffer_file = boost::filesystem::canonical(buffer_file);

            buffer_paths.insert({bind_location,buffer_file});
        }

        std::unordered_map<fragment_shader_texture_sampler_binding,boost::filesystem::path>  texture_paths;
        do
        {
            natural_8_bit  location = value(min_fragment_shader_texture_sampler_binding());
            bool  found = false;
            for ( ; location <= value(max_fragment_shader_texture_sampler_binding()); ++location)
                if (line == sampler_binding_name(fragment_shader_texture_sampler_binding(location)))
                {
                    found = true;
                    break;
                }
            if (!found)
                break;

            fragment_shader_texture_sampler_binding const  bind_location = fragment_shader_texture_sampler_binding(location);
            if (texture_paths.count(bind_location) != 0ULL)
            {
                error_message = msgstream() << "Fragment shader texture sampler binding '" << line
                                            << "' appears more than once in the file '" << batch_file << "'.";
                return {};
            }

            if (!detail::read_line(istr,line))
            {
                error_message = msgstream() << "Cannot read a path to a texture file to be bound to location '" << line
                                            << "' in the file '" << batch_file << "'.";
                return {};
            }
            boost::filesystem::path  texture_file = boost::filesystem::absolute(batch_file.parent_path() / line);
            if (!boost::filesystem::exists(texture_file) || !boost::filesystem::is_regular_file(texture_file))
            {
                error_message = msgstream() << "The texture file '" << texture_file.string()
                                            << "' to be bound to location '" << sampler_binding_name(bind_location)
                                            << "' referenced from the batch file '" << batch_file << "' does not exist.";
                return {};
            }
            texture_file = boost::filesystem::canonical(texture_file);

            texture_paths.insert({bind_location,texture_file});
        }
        while (detail::read_line(istr,line));

        natural_32_bit  cull_face_mode;
        if (line == "NONE")
            cull_face_mode = GL_NONE;
        else if (line == "BACK")
            cull_face_mode = GL_BACK;
        else if (line == "FRONT")
            cull_face_mode = GL_FRONT;
        else if (line == "FRONT_AND_BACK")
            cull_face_mode = GL_FRONT_AND_BACK;
        else
        {
            error_message = msgstream() << "Unknown cull face mode '" << line
                                        << "' in the file '" << batch_file << "'.";
            return {};
        }

        detail::read_line(istr,line);
        bool  use_alpha_blending;
        if (line == "true")
            use_alpha_blending = true;
        else if (line == "false")
            use_alpha_blending = false;
        else
        {
            error_message = msgstream() << "In the file '" << batch_file
                                        << "' there is expected 'true'/'false' "
                                           "for enabling/disabling alpha blending"
                                        << (line.empty() ? "." : ", but received: ")
                                        << line;
            return {};
        }

        detail::read_line(istr,line);
        natural_32_bit  alpha_blending_src_function = 0U;
        if (use_alpha_blending)
        {
            if (line == "ZERO")
                alpha_blending_src_function = GL_ZERO;
            else if (line == "ONE")
                alpha_blending_src_function = GL_ONE;
            else if (line == "SRC_ALPHA")
                alpha_blending_src_function = GL_SRC_ALPHA;
            else if (line == "DST_ALPHA")
                alpha_blending_src_function = GL_DST_ALPHA;
            else if (line == "ONE_MINUS_SRC_ALPHA")
                alpha_blending_src_function = GL_ONE_MINUS_SRC_ALPHA;
            else if (line == "ONE_MINUS_DST_ALPHA")
                alpha_blending_src_function = GL_ONE_MINUS_DST_ALPHA;
            else
            {
                if (line.empty())
                    error_message = msgstream() << "There is missing alpha blending source function in the file '"
                                                << batch_file << "'.";
                else
                    error_message = msgstream() << "Unknown alpha blending source function '" << line << "' in the file '"
                                                << batch_file << "'.";
                return {};
            }
        }

        detail::read_line(istr,line);
        natural_32_bit  alpha_blending_dst_function = 0U;
        if (use_alpha_blending)
        {
            if (line == "ZERO")
                alpha_blending_dst_function = GL_ZERO;
            else if (line == "ONE")
                alpha_blending_dst_function = GL_ONE;
            else if (line == "SRC_ALPHA")
                alpha_blending_dst_function = GL_SRC_ALPHA;
            else if (line == "DST_ALPHA")
                alpha_blending_dst_function = GL_DST_ALPHA;
            else if (line == "ONE_MINUS_SRC_ALPHA")
                alpha_blending_dst_function = GL_ONE_MINUS_SRC_ALPHA;
            else if (line == "ONE_MINUS_DST_ALPHA")
                alpha_blending_dst_function = GL_ONE_MINUS_DST_ALPHA;
            else
            {
                if (line.empty())
                    error_message = msgstream() << "There is missing alpha blending destination function in the file '"
                                                << batch_file << "'.";
                else
                    error_message = msgstream() << "Unknown alpha blending destination function '" << line << "' in the file '"
                                                << batch_file << "'.";
                return {};
            }
        }

        return std::make_shared<batch const>(
                    batch_file,
                    buffers_binding::create(index_buffer,buffer_paths),
                    shaders_binding::create(vertex_shader,fragment_shader),
                    textures_binding::create(texture_paths),
                    draw_state::create(cull_face_mode,use_alpha_blending,alpha_blending_src_function,alpha_blending_dst_function)
                    );
    }
    else if (file_type == "E2::qtgl/batch/vertices/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/batch/lines/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/batch/triangles/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else if (file_type == "E2::qtgl/batch/normals/3d/text")
    {
        NOT_IMPLEMENTED_YET();
    }
    else
    {
        error_message = msgstream() << "The passed batch file '" << batch_file
                                    << "' is of an unknown type '" << file_type << "'.";
        return {};
    }
}

void  insert_load_request(batch const&  batch_ref)
{
    if (!detail::batch_cache::instance().find(batch_ref.path()).operator bool())
        detail::batch_cache::instance().insert_load_request(batch_ref.path());
    else
    {
//        if (batch_ref.buffers_binding().operator bool())
//            insert_load_request(*batch_ref.buffers_binding());
        if (batch_ref.shaders_binding().operator bool())
            insert_load_request(*batch_ref.shaders_binding());
        if (batch_ref.textures_binding().operator bool())
            insert_load_request(*batch_ref.textures_binding());
    }
}


bool  make_current(batch const&  binding, draw_state const* const  previous_state)
{
    TMPROF_BLOCK();

    detail::batch_cache::instance().process_pending_batches();

    bool result = true;
    if (!binding.shaders_binding().operator bool() || !make_current(*binding.shaders_binding()))
        result = false;
    if (!binding.buffers_binding().operator bool() || !make_current(*binding.buffers_binding()))
        result = false;
    if (!binding.textures_binding().operator bool() || !make_current(*binding.textures_binding()))
        result = false;

    if (result)
    {
        if (binding.draw_state().operator bool())
            if (previous_state != nullptr)
                make_current(*binding.draw_state(),*previous_state);
            else
                make_current(*binding.draw_state());
        else
            result = false;
    }

    if (!result)
        insert_load_request(binding);

    return result;
}

bool  make_current(batch const&  binding)
{
    return make_current(binding,nullptr);
}

bool  make_current(batch const&  binding, draw_state const&  previous_state)
{
    return make_current(binding,&previous_state);
}


std::pair<bool,bool>  get_batch_chache_state(boost::filesystem::path const&  batch_file)
{
    return { detail::batch_cache::instance().find(batch_file).operator bool(),
             !detail::batch_cache::instance().fail_message(batch_file).empty() };
}

void  get_cached_batches(std::vector< std::pair<boost::filesystem::path,batch_ptr> >&  output)
{
    detail::batch_cache::instance().cached(output);
}

void  get_failed_batches(std::vector< std::pair<boost::filesystem::path,std::string> >&  output)
{
    detail::batch_cache::instance().failed(output);
}


}
