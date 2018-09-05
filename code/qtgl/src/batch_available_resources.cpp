#include <qtgl/batch_available_resources.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/read_line.hpp>
#include <utility/canonical_path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace qtgl { namespace detail {


batch_available_resources_data::batch_available_resources_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_buffers()
    , m_textures()
    , m_skeletal()
    , m_index_buffer()
    , m_root_dir(canonical_path(finaliser->get_key().get_unique_id()).string())
    , m_draw_state_file()
    , m_effects_file()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  path = m_root_dir;

    if (!boost::filesystem::is_directory(path))
        throw std::runtime_error(msgstream() << "The passed mesh path '" << path << "' is not an existing directory.");

    boost::filesystem::path  data_root_dir = path;
    while (true)
    {
        if (data_root_dir.empty())
            throw std::runtime_error(msgstream() << "Cannot find 'meshes' parent directory in path: " << path);
        boost::filesystem::path const  current_dir = data_root_dir.filename();
        data_root_dir = data_root_dir.parent_path();
        if (current_dir == "meshes")
            break;
    }

    m_draw_state_file = (path / "draw_state.txt").string();
    if (!boost::filesystem::is_regular_file(m_draw_state_file))
        throw std::runtime_error(msgstream() << "Cannot access 'draw_state' file '" << m_draw_state_file << "'.");

    m_effects_file = (path / "effects.txt").string();
    if (!boost::filesystem::is_regular_file(m_effects_file))
        throw std::runtime_error(msgstream() << "Cannot access 'effects' file '" << m_effects_file << "'.");

    boost::filesystem::path const  buffers_dir = path / "buffers";
    if (!boost::filesystem::is_directory(buffers_dir))
        throw std::runtime_error(msgstream() << "The buffer's directory '" << buffers_dir << "' does not exist.");

    std::unordered_set<natural_8_bit>  valid_texcoord_indices;
    for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(buffers_dir))
        if (boost::filesystem::is_regular_file(entry.path()))
        {
            std::string const  fname = entry.path().filename().string();
            if (fname == "indices.txt")
                m_index_buffer = entry.path().string();
            else if (fname == "vertices.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION,
                                  canonical_path(entry.path()).string()});
            else if (fname == "normals.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_NORMAL,
                                  canonical_path(entry.path()).string()});
            else if (fname == "tangents.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_TANGENT,
                                  canonical_path(entry.path()).string()});
            else if (fname == "bitangents.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_BITANGENT,
                                  canonical_path(entry.path()).string()});
            else if (fname == "diffuse.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_DIFFUSE,
                                  canonical_path(entry.path()).string()});
            else if (fname == "specular.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_SPECULAR,
                                  canonical_path(entry.path()).string()});
            else if (fname.find("texcoords") == 0UL && entry.path().filename().extension() == ".txt")
                for (natural_8_bit  i = 0U, n = get_num_texcoord_binding_locations(); i != n; ++i)
                    if (fname == std::string("texcoords") + std::to_string(i) + ".txt")
                    {
                        m_buffers.insert({get_texcoord_binding_location(i), canonical_path(entry.path()).string()});
                        valid_texcoord_indices.insert(i);
                        break;
                    }
        }
    if (m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::BINDING_IN_POSITION) == 0UL)
        throw std::runtime_error(msgstream() << "Cannot find the vertex buffer file: " << (buffers_dir / "vertices.txt"));
    if (m_index_buffer.empty())
        throw std::runtime_error(msgstream() << "Cannot find the index buffer file: " << (buffers_dir / "indices.txt"));

    boost::filesystem::path const  texture_links_dir = path / "textures";
    if (boost::filesystem::is_directory(texture_links_dir))
        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(texture_links_dir))
            if (boost::filesystem::is_regular_file(entry.path()))
            {
                auto const  load_link_file =
                    [&data_root_dir, &valid_texcoord_indices](
                            boost::filesystem::path const&  pathname
                            ) -> std::pair<VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION, std::string>
                    {
                        std::ifstream  istr(pathname.string());
                        std::vector<std::string> const  lines{
                                read_line(istr),
                                canonical_path(data_root_dir/read_line(istr)).string()
                                };
                        if (lines.front().empty())
                            throw std::runtime_error(msgstream() << "Cannot read texcoord index in the texture link file: "
                                                                 << pathname);
                        natural_8_bit const  texcoord_index = std::atoi(lines.front().c_str());
                        if (valid_texcoord_indices.count(texcoord_index) == 0UL)
                            throw std::runtime_error(msgstream() << "The texture link file '" << pathname
                                                                 << "' references a non-existing texcoord file of index "
                                                                 << texcoord_index);
                        if (!boost::filesystem::is_regular_file(lines.back()))
                            throw std::runtime_error(msgstream() << "The texture link file '" << pathname
                                                                 << "' references a non-existing texture file: "
                                                                 << lines.back());
                        return { get_texcoord_binding_location(texcoord_index), lines.back() };
                    };

                std::string const  fname = entry.path().filename().string();
                if (fname == "diffuse.txt")
                {
                    auto const  location_and_path = load_link_file(entry.path());
                    m_textures.insert({
                        FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE,
                        {location_and_path.first, location_and_path.second}
                        });
                }
                else if (fname == "specular.txt")
                {
                    auto const  location_and_path = load_link_file(entry.path());
                    m_textures.insert({
                        FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_SPECULAR,
                        {location_and_path.first, location_and_path.second}
                        });
                }
                else if (fname == "normal.txt")
                {
                    auto const  location_and_path = load_link_file(entry.path());
                    m_textures.insert({
                        FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_NORMAL,
                        {location_and_path.first, location_and_path.second}
                        });
                }
            }

    boost::filesystem::path const  skeletal_dir = path / "skeletal";
    if (boost::filesystem::is_directory(skeletal_dir))
        for (boost::filesystem::directory_entry& entry : boost::filesystem::directory_iterator(skeletal_dir))
            if (boost::filesystem::is_directory(entry.path()))
            {
                boost::filesystem::path const  alignment_pathname = canonical_path(entry.path() / "alignment.txt");
                if (!boost::filesystem::is_regular_file(alignment_pathname))
                    throw std::runtime_error(msgstream() << "Cannot locate file:" << alignment_pathname);

                boost::filesystem::path const  indices_pathname = canonical_path(entry.path() / "indices.txt");
                if (!boost::filesystem::is_regular_file(indices_pathname))
                    throw std::runtime_error(msgstream() << "Cannot locate file:" << indices_pathname);

                boost::filesystem::path const  weights_pathname = canonical_path(entry.path() / "weights.txt");
                if (!boost::filesystem::is_regular_file(weights_pathname))
                    throw std::runtime_error(msgstream() << "Cannot locate file:" << weights_pathname);

                boost::filesystem::path const  skeleton_dir =
                        canonical_path(data_root_dir / "animations" / "skeletal" / entry.path().filename());
                if (!boost::filesystem::is_directory(skeleton_dir))
                    throw std::runtime_error(msgstream() << "Cannot access the reference skeleton directory:" << skeleton_dir);

                m_skeletal.insert({
                    skeleton_dir.string(),
                    {
                        alignment_pathname.string(),
                        indices_pathname.string(),
                        weights_pathname.string()
                    }
                });
            }
}


batch_available_resources_data::batch_available_resources_data(
        async::finalise_load_on_destroy_ptr,
        buffers_dictionaty_type const&  buffers_,
        textures_dictionary_type const&  textures_,
        skeletal_dictionary_type const&  skeletal_,
        std::string const&  index_buffer,
        std::string const&  draw_state_file,
        std::string const&  effects_file
        )
    : m_buffers(buffers_)
    , m_textures(textures_)
    , m_skeletal(skeletal_)
    , m_index_buffer(index_buffer)
    , m_root_dir()
    , m_draw_state_file(draw_state_file)
    , m_effects_file(effects_file)
{}


}}
