#include <qtgl/batch.hpp>
#include <qtgl/shader_data_bindings.hpp>
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

batch::batch(boost::filesystem::path const&  path,
             buffers_binding_ptr const  buffers_binding,
             shaders_binding_ptr const  shaders_binding,
             textures_binding_ptr const  textures_binding
             )
    : m_path(path)
    , m_buffers_binding(buffers_binding)
    , m_shaders_binding(shaders_binding)
    , m_textures_binding(textures_binding)
{
    ASSUMPTION(m_buffers_binding.operator bool());
    ASSUMPTION(m_shaders_binding.operator bool());
    ASSUMPTION(m_textures_binding.operator bool());
}

std::unordered_set<vertex_shader_uniform_symbolic_name> const&  batch::symbolic_names_of_used_uniforms() const
{
    return shaders_binding().operator bool() && shaders_binding()->vertex_program_props().operator bool() ?
                shaders_binding()->vertex_program_props()->symbolic_names_of_used_uniforms() :
                s_empty_uniforms ;
}

std::shared_ptr<batch const>  load_batch_file(boost::filesystem::path const&  batch_file, std::string&  error_message)
{
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
        boost::filesystem::path const  vertex_shader = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(vertex_shader) || !boost::filesystem::is_regular_file(vertex_shader))
        {
            error_message = msgstream() << "The vertex shader file '" << vertex_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to fragment shader file in the file '" << batch_file << "'.";
            return {};
        }
        boost::filesystem::path const  fragment_shader = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(fragment_shader) || !boost::filesystem::is_regular_file(fragment_shader))
        {
            error_message = msgstream() << "The fragment shader file '" << fragment_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }

        if (!detail::read_line(istr,line))
        {
            error_message = msgstream() << "Cannot read a path to the index buffer file in the file '" << batch_file << "'.";
            return {};
        }
        boost::filesystem::path const  index_buffer = boost::filesystem::absolute(batch_file.parent_path() / line);
        if (!boost::filesystem::exists(index_buffer) || !boost::filesystem::is_regular_file(index_buffer))
        {
            error_message = msgstream() << "The fragment shader file '" << fragment_shader.string()
                                        << "' referenced from the batch file '" << batch_file << "' does not exist.";
            return {};
        }

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
            boost::filesystem::path const  buffer_file = boost::filesystem::absolute(batch_file.parent_path() / line);
            if (!boost::filesystem::exists(buffer_file) || !boost::filesystem::is_regular_file(buffer_file))
            {
                error_message = msgstream() << "The buffer file '" << buffer_file.string()
                                            << "' to be bound to location '" << binding_location_name(bind_location)
                                            << "' referenced from the batch file '" << batch_file << "' does not exist.";
                return {};
            }

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
            {
                error_message = msgstream() << "Unknown fragment shader texture sampler binding '" << line
                                            << "' in the file '" << batch_file << "'.";
                return {};
            }
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
            boost::filesystem::path const  texture_file = boost::filesystem::absolute(batch_file.parent_path() / line);
            if (!boost::filesystem::exists(texture_file) || !boost::filesystem::is_regular_file(texture_file))
            {
                error_message = msgstream() << "The texture file '" << texture_file.string()
                                            << "' to be bound to location '" << sampler_binding_name(bind_location)
                                            << "' referenced from the batch file '" << batch_file << "' does not exist.";
                return {};
            }

            texture_paths.insert({bind_location,texture_file});
        }
        while (detail::read_line(istr,line));

        return std::make_shared<batch const>(
                    batch_file,
                    buffers_binding::create(index_buffer,buffer_paths),
                    shaders_binding::create(vertex_shader,fragment_shader),
                    textures_binding::create(texture_paths)
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


bool  make_current(batch const&  binding)
{
    if (!make_current(*binding.shaders_binding()))
        return false;
    if (!make_current(*binding.buffers_binding()))
        return false;
    if (!make_current(*binding.textures_binding()))
        return false;
    return true;
}


}
