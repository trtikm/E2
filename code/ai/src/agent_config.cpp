#include <ai/agent_config.hpp>
#include <ai/utils_ptree.hpp>
#include <utility/canonical_path.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <boost/algorithm/string.hpp>
#include <filesystem>
#include <fstream>

namespace ai { namespace detail {


agent_config_data::agent_config_data(async::finalise_load_on_destroy_ptr const finaliser)
    : m_defaults()
    , m_state_variables()
    , m_initial_action()
    , m_actions()
    , m_sight()
    , m_cortex()
{
    TMPROF_BLOCK();

    std::filesystem::path const  root_dir = finaliser->get_key().get_unique_id();
    if (!std::filesystem::is_directory(root_dir))
        throw std::runtime_error(msgstream() << "Cannot access directory '" << root_dir << "'.");

    boost::property_tree::ptree  root_cfg;
    load_ptree(root_cfg, root_dir / "config.json");

    std::vector<std::filesystem::path>  root_load_dirs{ finaliser->get_key().get_unique_id() };
    if (root_cfg.count("imports") != 0UL)
    {
        boost::property_tree::ptree const&  imports = get_ptree("imports", root_cfg);
        for (auto  it = imports.begin(); it != imports.end(); ++it)
            root_load_dirs.push_back(root_load_dirs.front() / it->second.data());
    }

    for (std::filesystem::path const&  load_dir : root_load_dirs)
        load_data_from_dir(load_dir);

    ASSUMPTION(m_actions.count(m_initial_action) != 0UL);
    ASSUMPTION(!m_sight.empty());
    ASSUMPTION(!m_cortex.empty());
}


void  agent_config_data::load_data_from_dir(std::filesystem::path const&  root_dir)
{
    if (m_defaults.empty() && std::filesystem::is_regular_file(root_dir / "defaults.json"))
        load_ptree(m_defaults, root_dir / "defaults.json");
    if (m_sight.empty() && std::filesystem::is_regular_file(root_dir / "sight.json"))
        load_ptree(m_sight, root_dir / "sight.json");
    if (m_cortex.empty() && std::filesystem::is_regular_file(root_dir / "cortex.json"))
        load_ptree(m_cortex, root_dir / "cortex.json");
    for (std::filesystem::directory_entry const&  entry : std::filesystem::directory_iterator(root_dir))
        if (std::filesystem::is_directory(entry.path()))
        {
            std::string const  dir_name = entry.path().filename().string();
            if (dir_name == "state_variables")
                load_state_variables_from_dir(entry.path());
            else if (dir_name == "actions")
                load_actions_from_dir(entry.path());
        }
}


void  agent_config_data::load_state_variables_from_dir(std::filesystem::path const&  root_dir)
{
    for (std::filesystem::directory_entry const&  entry : std::filesystem::directory_iterator(root_dir))
        if (std::filesystem::is_regular_file(entry.path()) && entry.path().filename().extension().string() == ".json")
        {
            std::string const  name = entry.path().filename().replace_extension("").string();
            if (m_state_variables.count(name) == 0UL)
                m_state_variables[name] = load_ptree(entry.path());
        }
}


void  agent_config_data::load_actions_from_dir(std::filesystem::path const&  root_dir)
{
    for (std::filesystem::directory_entry const&  entry : std::filesystem::directory_iterator(root_dir))
        if (std::filesystem::is_regular_file(entry.path()))
        {
            if (entry.path().filename().string() == "initial_action.txt")
            {
                if (!m_initial_action.empty())
                    continue;
                std::ifstream  istr;
                istr.open(entry.path().string(), std::ios_base::binary);
                if (!istr.good())
                    throw std::runtime_error(msgstream() << "Cannot open file '" << entry.path().string() << "'.");
                std::getline(istr, m_initial_action);
                boost::algorithm::trim(m_initial_action);
                ASSUMPTION(!m_initial_action.empty());
            }
            else if (entry.path().filename().extension().string() == ".json")
            {
                std::string const  name = entry.path().filename().replace_extension("").string();
                if (m_actions.count(name) == 0UL)
                    m_actions[name] = load_ptree(entry.path());
            }
        }
}


agent_config_data::~agent_config_data()
{}


}}
