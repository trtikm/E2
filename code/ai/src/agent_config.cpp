#include <ai/agent_config.hpp>
#include <utility/canonical_path.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/json_parser.hpp>

namespace ai { namespace detail { namespace {


void  load_ptree(boost::property_tree::ptree&  ptree, boost::filesystem::path const&  ptree_pathname)
{
    if (!boost::filesystem::is_regular_file(ptree_pathname))
        throw std::runtime_error(msgstream() << "Cannot access the file '" << ptree_pathname << "'.");
    boost::property_tree::read_json(ptree_pathname.string(), ptree);
}


std::shared_ptr<boost::property_tree::ptree>  load_ptree(boost::filesystem::path const&  ptree_pathname)
{
    std::shared_ptr<boost::property_tree::ptree>  ptree(new boost::property_tree::ptree);
    load_ptree(*ptree, ptree_pathname);
    return ptree;
}


}}}

namespace ai { namespace detail {


agent_config_data::agent_config_data(async::finalise_load_on_destroy_ptr const finaliser)
    : m_defaults()
    , m_state_variables()
    , m_initial_action()
    , m_actions()
    , m_use_cortex_mock()
    , m_cortex_mock()
{
    TMPROF_BLOCK();

    boost::filesystem::path const  root_dir = finaliser->get_key().get_unique_id();
    if (!boost::filesystem::is_directory(root_dir))
        throw std::runtime_error(msgstream() << "Cannot access directory '" << root_dir << "'.");

    boost::property_tree::ptree  root_cfg;
    detail::load_ptree(root_cfg, root_dir / "config.json");

    m_use_cortex_mock = root_cfg.get<bool>("use_cortex_mock");

    std::vector<boost::filesystem::path>  root_load_dirs{ finaliser->get_key().get_unique_id() };
    if (root_cfg.count("imports") != 0UL)
    {
        boost::property_tree::ptree const&  imports = root_cfg.find("imports")->second;
        for (auto  it = imports.begin(); it != imports.end(); ++it)
            root_load_dirs.push_back(root_load_dirs.front() / it->second.data());
    }

    for (boost::filesystem::path const&  load_dir : root_load_dirs)
        load_data_from_dir(load_dir);
}


void  agent_config_data::load_data_from_dir(boost::filesystem::path const&  root_dir)
{
    if (m_defaults.empty() && boost::filesystem::is_regular_file(root_dir / "defaults.json"))
        load_ptree(m_defaults, root_dir / "defaults.json");
    for (boost::filesystem::directory_entry const&  entry : boost::filesystem::directory_iterator(root_dir))
        if (boost::filesystem::is_directory(entry.path()))
        {
            std::string const  dir_name = entry.path().filename().string();
            if (dir_name == "state_variables")
                load_state_variables_from_dir(entry.path());
            else if (dir_name == "actions")
                load_actions_from_dir(entry.path());
            else if (dir_name == "cortex")
                load_cortex_from_dir(entry.path());
        }
}


void  agent_config_data::load_state_variables_from_dir(boost::filesystem::path const&  root_dir)
{
    for (boost::filesystem::directory_entry const&  entry : boost::filesystem::directory_iterator(root_dir))
        if (boost::filesystem::is_regular_file(entry.path()) && entry.path().filename().extension().string() == ".json")
        {
            std::string const  name = entry.path().filename().replace_extension("").string();
            if (m_state_variables.count(name) == 0UL)
                m_state_variables[name] = load_ptree(entry.path());
        }
}


void  agent_config_data::load_actions_from_dir(boost::filesystem::path const&  root_dir)
{
    for (boost::filesystem::directory_entry const&  entry : boost::filesystem::directory_iterator(root_dir))
        if (boost::filesystem::is_regular_file(entry.path()))
        {
            if (entry.path().filename().string() == "initial_action.txt")
            {
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
    ASSUMPTION(m_actions.count(m_initial_action) != 0UL);
}


void  agent_config_data::load_cortex_from_dir(boost::filesystem::path const&  root_dir)
{
    for (boost::filesystem::directory_entry const&  entry : boost::filesystem::directory_iterator(root_dir))
        if (boost::filesystem::is_regular_file(entry.path()) && entry.path().filename().extension().string() == ".json")
        {
            std::string const  name = entry.path().filename().replace_extension("").string();
            if (name == "mock")
            {
                if (m_use_cortex_mock && m_cortex_mock.empty())
                    load_ptree(m_cortex_mock, entry.path());
                continue;
            }

        }
}


agent_config_data::~agent_config_data()
{}


}}
