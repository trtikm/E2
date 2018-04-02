#include <qtgl/batch.hpp>
#include <qtgl/shader_data_bindings.hpp>
#include <qtgl/detail/read_line.hpp>
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


batch_data::batch_data(boost::filesystem::path const&  path, async::finalise_load_on_destroy_ptr)
{
    TMPROF_BLOCK();

    if (!boost::filesystem::exists(path))
        throw std::runtime_error(msgstream() << "The batch file '" << path << "' does not exist.");
    if (!boost::filesystem::is_regular_file(path))
        throw std::runtime_error(msgstream() << "The batch path '" << path << "' does not reference a regular file.");
    if (boost::filesystem::file_size(path) < 4ULL)
        throw std::runtime_error(msgstream() << "The passed file '" << path << "' is not a qtgl file (wrong size).");

    std::ifstream  istr(path.string(),std::ios_base::binary);
    if (!istr.good())
        throw std::runtime_error(msgstream() << "Cannot open the batch file '" << path << "'.");

    std::string  file_type;
    if (!detail::read_line(istr,file_type))
        throw std::runtime_error(msgstream() << "The passed file '" << path
                                             << "' is not a qtgl file (cannot read its type string).");

    if (file_type == "E2::qtgl/batch/indexed/text")
    {
        std::string  line;

        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read a path to vertex shader file in the file '" << path << "'.");
        boost::filesystem::path  vertex_shader_path = boost::filesystem::absolute(path.parent_path() / line);
        if (!boost::filesystem::exists(vertex_shader_path) || !boost::filesystem::is_regular_file(vertex_shader_path))
            throw std::runtime_error(msgstream() << "The vertex shader file '" << vertex_shader_path.string()
                                                 << "' referenced from the batch file '" << path << "' does not exist.");
        vertex_shader_path = canonical_path(vertex_shader_path);

        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read a path to fragment shader file in the file '" << path << "'.");
        boost::filesystem::path  fragment_shader_path = boost::filesystem::absolute(path.parent_path() / line);
        if (!boost::filesystem::exists(fragment_shader_path) || !boost::filesystem::is_regular_file(fragment_shader_path))
            throw std::runtime_error(msgstream() << "The fragment shader file '" << fragment_shader_path.string()
                                                 << "' referenced from the batch file '" << path << "' does not exist.");
        fragment_shader_path = canonical_path(fragment_shader_path);

        if (!detail::read_line(istr,line))
            throw std::runtime_error(msgstream() << "Cannot read a path to the index buffer file in the file '" << path << "'.");
        boost::filesystem::path  index_buffer_path = boost::filesystem::absolute(path.parent_path() / line);
        if (!boost::filesystem::exists(index_buffer_path) || !boost::filesystem::is_regular_file(index_buffer_path))
            throw std::runtime_error(msgstream() << "The index buffer file '" << index_buffer_path.string()
                                                 << "' referenced from the batch file '" << path << "' does not exist.");
        index_buffer_path = canonical_path(index_buffer_path);

        std::unordered_map<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION,boost::filesystem::path>  buffer_paths;
        while (true)
        {
            if (!detail::read_line(istr,line))
                break;
            natural_8_bit  location = value(min_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION());
            bool  found = false;
            for ( ; location <= value(max_VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION()); ++location)
                if (line == name(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION(location)))
                {
                    found = true;
                    break;
                }
            if (!found)
            {
                if (line.find("BINDING_") != 0ULL || line.find("TEXTURE_SAMPLER_") == 0ULL)
                    break;

                throw std::runtime_error(msgstream() << "Unknown vertex shader input buffer binding location '" << line
                                                     << "' in the file '" << path << "'.");
            }
            VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION const  bind_location = VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION(location);
            if (buffer_paths.count(bind_location) != 0ULL)
                throw std::runtime_error(msgstream() << "Vertex shader input buffer binding location '" << line
                                                     << "' appears more than once in the file '" << path << "'.");

            if (!detail::read_line(istr,line))
                throw std::runtime_error(msgstream() << "Cannot read a path to a buffer file to be bound at location '" << line
                                                     << "' in the file '" << path << "'.");
            boost::filesystem::path  buffer_file = boost::filesystem::absolute(path.parent_path() / line);
            if (!boost::filesystem::exists(buffer_file) || !boost::filesystem::is_regular_file(buffer_file))
                throw std::runtime_error(msgstream() << "The buffer file '" << buffer_file.string()
                                                     << "' to be bound to location '" << name(bind_location)
                                                     << "' referenced from the batch file '" << path << "' does not exist.");
            buffer_file = canonical_path(buffer_file);

            buffer_paths.insert({bind_location,buffer_file});
        }

        textures_binding_map_type  textures_binding_map;
        do
        {
            natural_8_bit  location = value(min_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME());
            bool  found = false;
            for ( ; location <= value(max_FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME()); ++location)
                if (line == uniform_name_symbolic(FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME(location)))
                {
                    found = true;
                    break;
                }
            if (!found)
                break;

            FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME const  bind_location = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME(location);
            if (textures_binding_map.count(bind_location) != 0ULL)
                throw std::runtime_error(msgstream() << "Fragment shader texture sampler binding '" << line
                                                     << "' appears more than once in the file '" << path << "'.");

            if (!detail::read_line(istr,line))
                throw std::runtime_error(msgstream() << "Cannot read a path to a texture file to be bound to location '" << line
                                                     << "' in the file '" << path << "'.");
            boost::filesystem::path  texture_file = boost::filesystem::absolute(path.parent_path() / line);
            if (!boost::filesystem::exists(texture_file) || !boost::filesystem::is_regular_file(texture_file))
                throw std::runtime_error(msgstream() << "The texture file '" << texture_file.string()
                                                     << "' to be bound to location '" << uniform_name(bind_location)
                                                     << "' referenced from the batch file '" << path << "' does not exist.");
            texture_file = canonical_path(texture_file);

            textures_binding_map.insert({bind_location,texture(texture_file)});
        }
        while (detail::read_line(istr,line));

        boost::filesystem::path  modelspace_pathname;
        if (line != "NONE" && line != "BACK" && line != "FRONT_AND_BACK")
        {
            boost::filesystem::path const  pathname = canonical_path(path.parent_path() / line);
            if (boost::filesystem::is_regular_file(pathname))
                modelspace_pathname = pathname;
            else
                throw std::runtime_error(msgstream() << "Cannot find batch's model-space coord. systems file '"
                                                     << pathname << "'.");

            detail::read_line(istr, line);
        }

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
            throw std::runtime_error(msgstream() << "Unknown cull face mode '" << line
                                                 << "' in the file '" << path << "'.");

        detail::read_line(istr,line);
        bool  use_alpha_blending;
        if (line == "true")
            use_alpha_blending = true;
        else if (line == "false")
            use_alpha_blending = false;
        else
            throw std::runtime_error(msgstream() << "In the file '" << path
                                                 << "' there is expected 'true'/'false' "
                                                    "for enabling/disabling alpha blending"
                                                 << (line.empty() ? "." : ", but received: ")
                                                 << line);

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
                    throw std::runtime_error(msgstream() << "There is missing alpha blending source function in the file '"
                                                         << path << "'.");
                else
                    throw std::runtime_error(msgstream() << "Unknown alpha blending source function '"
                                                         << line << "' in the file '" << path << "'.");
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
                    throw std::runtime_error(msgstream() << "There is missing alpha blending destination function in the file '"
                                                         << path << "'.");
                else
                    throw std::runtime_error(msgstream() << "Unknown alpha blending destination function '" << line
                                                         << "' in the file '" << path << "'.");
            }
        }

        initialise(
            buffers_binding(index_buffer_path, buffer_paths, "[buffers_binding]:" + path.string()),
            shaders_binding(vertex_shader_path, fragment_shader_path, "[shaders_binding]:" + path.string()),
            textures_binding(textures_binding_map),
            draw_state::create(cull_face_mode, use_alpha_blending, alpha_blending_src_function, alpha_blending_dst_function),
            modelspace_pathname.empty() ? modelspace() : modelspace(modelspace_pathname)
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
        throw std::runtime_error(msgstream() << "The passed batch file '" << path
                                             << "' is of an unknown type '" << file_type << "'.");
}


batch_data::~batch_data()
{
    TMPROF_BLOCK();
}


void  batch_data::initialise(
        buffers_binding const  buffers_binding,
        shaders_binding const  shaders_binding,
        textures_binding const  textures_binding,
        draw_state_ptr const  draw_state,
        modelspace const  modelspace
        )
{
    TMPROF_BLOCK();

    m_buffers_binding = buffers_binding;
    m_shaders_binding = shaders_binding;
    m_textures_binding = textures_binding;
    m_draw_state = draw_state;
    m_modelspace = modelspace;
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
            (!get_modelspace().empty() && !get_modelspace().loaded_successfully()))
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
