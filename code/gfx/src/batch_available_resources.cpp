#include <gfx/batch_available_resources.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/msgstream.hpp>
#include <utility/read_line.hpp>
#include <utility/canonical_path.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <stdexcept>

namespace gfx { namespace detail {


batch_available_resources_data::batch_available_resources_data(async::finalise_load_on_destroy_ptr const  finaliser)
    : m_buffers()
    , m_skeletal()
    , m_batch_pathname(canonical_path(finaliser->get_key().get_unique_id()).string())
    , m_data_root_dir()
    , m_mesh_path()
    , m_num_indices_per_primitive(0xFFU)   // i.e. an illegal value (correct value will be set during the load below)
    , m_skins()
{
    TMPROF_BLOCK();

    if (!std::filesystem::is_regular_file(batch_pathname()))
        throw std::runtime_error(msgstream() << "Cannot access the passed batch file '" << batch_pathname() << "'.");

    std::filesystem::path  data_root_dir = batch_pathname();
    while (true)
    {
        if (data_root_dir.empty())
            throw std::runtime_error(msgstream() << "Cannot find 'batch' parent directory in path: " << batch_pathname());
        std::filesystem::path const  current_dir = data_root_dir.filename();
        data_root_dir = data_root_dir.parent_path();
        if (current_dir == "batch")
            break;
    }
    m_data_root_dir = data_root_dir.string();

    boost::property_tree::ptree  batch_ptree;
    boost::property_tree::read_info(batch_pathname(), batch_ptree);

    m_mesh_path = std::string("mesh/") + batch_ptree.get<std::string>("mesh");

    std::filesystem::path const  buffers_dir = data_root_dir / m_mesh_path;
    if (!std::filesystem::is_directory(buffers_dir))
        throw std::runtime_error(msgstream() << "The buffer's directory '" << buffers_dir << "' does not exist.");

    std::unordered_set<natural_8_bit>  valid_texcoord_indices;
    for (std::filesystem::directory_entry const& entry : std::filesystem::directory_iterator(buffers_dir))
        if (std::filesystem::is_regular_file(entry.path()))
        {
            std::string const  fname = entry.path().filename().string();
            if (fname == "indices.txt")
                m_num_indices_per_primitive = 0U;
            else if (fname == "vertices.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION,
                                    canonical_path(entry.path()).string()});
            else if (fname == "normals.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_NORMAL,
                                    canonical_path(entry.path()).string()});
            else if (fname == "tangents.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_TANGENT,
                                    canonical_path(entry.path()).string()});
            else if (fname == "bitangents.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_BITANGENT,
                                    canonical_path(entry.path()).string()});
            else if (fname == "diffuse.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_DIFFUSE,
                                    canonical_path(entry.path()).string()});
            else if (fname == "specular.txt")
                m_buffers.insert({VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_SPECULAR,
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
    if (m_buffers.count(VERTEX_SHADER_INPUT_BUFFER_BINDING_LOCATION::IN_POSITION) == 0UL)
        throw std::runtime_error(msgstream() << "Cannot find the vertex buffer file: " << (buffers_dir / "vertices.txt"));
    if (m_num_indices_per_primitive != 0U) // TODO: Extend structure of 'batch' file on the disk, so that non-index buffer batches could be loaded.
        throw std::runtime_error(msgstream() << "Cannot find the index buffer file: " << (buffers_dir / "indices.txt"));

    boost::property_tree::ptree const&  skins_ptree = batch_ptree.find("skins")->second;
    for (auto  skin_it = skins_ptree.begin(); skin_it != skins_ptree.end(); ++skin_it)
    {
        textures_dictionary_type  textures;
        if (skin_it->second.count("textures") != 0UL)
        {
            boost::property_tree::ptree const&  textures_ptree = skin_it->second.find("textures")->second;
            for (auto it = textures_ptree.begin(); it != textures_ptree.end(); ++it)
            {
                FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME  sampler_location;
                if (it->first == "diffuse")
                    sampler_location = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_DIFFUSE;
                else if (it->first == "specular")
                    sampler_location = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_SPECULAR;
                else if (it->first == "normal")
                    sampler_location = FRAGMENT_SHADER_UNIFORM_SYMBOLIC_NAME::TEXTURE_SAMPLER_NORMAL;
                else
                    throw std::runtime_error(msgstream() << "Unknown/unsupported texture type '" << it->first
                                                            << "' in the batch file '" << batch_pathname() << "'.");

                natural_8_bit const  texcoord_index = it->second.get<unsigned int>("texcoord_index");
                if (valid_texcoord_indices.count(texcoord_index) == 0UL)
                    throw std::runtime_error(msgstream() << "The texture type '" << it->first
                                                            << "' references a non-existing texcoord file of index "
                                                            << texcoord_index);

                std::string const  texure_pathname = (data_root_dir / "texture" / it->second.get<std::string>("pathname")).string();
                if (!std::filesystem::is_regular_file(texure_pathname))
                    throw std::runtime_error(msgstream() << "The texture type '" << it->first
                                                            << "' references a non-existing texture file: "
                                                            << texure_pathname);

                bool const  success =
                    textures.insert({ sampler_location, { get_texcoord_binding_location(texcoord_index), texure_pathname } }).second;
                if (!success)
                    throw std::runtime_error(msgstream() << "The insertion of texture '" << texure_pathname << "' has FAILED.");

            }
        }
        draw_state const  dstate = create_draw_state(skin_it->second.find("draw_state")->second);

        boost::property_tree::ptree const& alpha_testing_ptree = skin_it->second.find("alpha_testing")->second;
        alpha_testing_props const  alpha_testing(
                alpha_testing_ptree.get("use_alpha_testing", false),
                alpha_testing_ptree.get("alpha_test_constant", 0.0f)
                );

        bool const  success = m_skins.insert({ skin_it->first, { textures, dstate, alpha_testing } }).second;
        if (!success)
            throw std::runtime_error(msgstream() << "The insertion of skin '" << skin_it->first << "' has FAILED.");
    }

    if (m_skins.empty())
        throw std::runtime_error(msgstream() << "No skin was found.");

    std::filesystem::path const  skeletal_dir = buffers_dir / "skeletal";
    if (std::filesystem::is_directory(skeletal_dir))
    {
        if (batch_ptree.count("skeleton") == 0UL)
            throw std::runtime_error(msgstream() << "Skeleton name is missing in batch file '" << batch_pathname() << "'.");
        std::filesystem::path const  alignment_pathname = canonical_path(skeletal_dir / "alignment.txt");
        if (!std::filesystem::is_regular_file(alignment_pathname))
            throw std::runtime_error(msgstream() << "Cannot locate file:" << alignment_pathname);

        std::filesystem::path const  indices_pathname = canonical_path(skeletal_dir / "indices.txt");
        if (!std::filesystem::is_regular_file(indices_pathname))
            throw std::runtime_error(msgstream() << "Cannot locate file:" << indices_pathname);

        std::filesystem::path const  weights_pathname = canonical_path(skeletal_dir / "weights.txt");
        if (!std::filesystem::is_regular_file(weights_pathname))
            throw std::runtime_error(msgstream() << "Cannot locate file:" << weights_pathname);

        std::string const  animation_dir = std::string("animation/") + batch_ptree.get<std::string>("skeleton");
        std::filesystem::path const  skeleton_dir = canonical_path(data_root_dir / animation_dir);
        if (!std::filesystem::is_directory(skeleton_dir))
            throw std::runtime_error(msgstream() << "Cannot access the reference skeleton directory:" << skeleton_dir);

        m_skeletal = std::make_shared<skeletal_info>(
                animation_dir,
                alignment_pathname.string(),
                indices_pathname.string(),
                weights_pathname.string()
                );
    }
    else if (batch_ptree.count("skeleton") != 0UL)
        throw std::runtime_error(msgstream() << "Skeletal buffer files are missing for the batch file '" << batch_pathname() << "'.");
}


batch_available_resources_data::batch_available_resources_data(
        async::finalise_load_on_destroy_ptr,
        buffers_dictionaty_type const&  buffers_,
        skeletal_info_const_ptr const&  skeletal_,
        std::string const&  batch_pathname_,
        std::string const&  data_root_dir_,
        std::string const&  mesh_path_,
        natural_8_bit const  num_indices_per_primitive_,
        skins_dictionary const&  skins_
        )
    : m_buffers(buffers_)
    , m_skeletal(skeletal_)
    , m_batch_pathname(batch_pathname_)
    , m_data_root_dir(data_root_dir_)
    , m_mesh_path(mesh_path_)
    , m_num_indices_per_primitive(num_indices_per_primitive_)
    , m_skins(skins_)
{
    ASSUMPTION(!m_skins.empty());
}


}}
