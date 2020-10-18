#ifndef AI_AGENT_CONFIG_HPP_INCLUDED
#   define AI_AGENT_CONFIG_HPP_INCLUDED

#   include <ai/motion_desire_props.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <boost/filesystem/path.hpp>
#   include <string>
#   include <unordered_map>
#   include <memory>

namespace ai { namespace detail {


struct  agent_config_data
{
    explicit agent_config_data(async::finalise_load_on_destroy_ptr const  finaliser);
    ~agent_config_data();

    boost::property_tree::ptree  m_defaults;
    std::unordered_map<std::string, std::shared_ptr<boost::property_tree::ptree> >  m_state_variables;
    std::string  m_initial_action;
    std::unordered_map<std::string, std::shared_ptr<boost::property_tree::ptree> >  m_actions;
    bool  m_use_cortex_mock;

private:
    void  load_data_from_dir(boost::filesystem::path const&  root_dir);
    void  load_state_variables_from_dir(boost::filesystem::path const&  root_dir);
    void  load_actions_from_dir(boost::filesystem::path const&  root_dir);
    void  load_cortex_from_dir(boost::filesystem::path const&  root_dir);
};


}}

namespace ai {


struct  agent_config : public async::resource_accessor<detail::agent_config_data>
{
    agent_config()
        : async::resource_accessor<detail::agent_config_data>()
    {}

    agent_config(
            boost::filesystem::path const&  path,
            async::load_priority_type const  priority,
            async::finalise_load_on_destroy_ptr const  parent_finaliser = nullptr
            )
        : async::resource_accessor<detail::agent_config_data>(
            {"ai::agent_config",path.string()},
            priority,
            parent_finaliser
            )
    {}

    boost::property_tree::ptree const&  defaults() const { return resource().m_defaults; }
    std::unordered_map<std::string, std::shared_ptr<boost::property_tree::ptree> > const&  state_variables() const { return resource().m_state_variables; }
    std::string const&  initial_action() const { return resource().m_initial_action; }
    std::unordered_map<std::string, std::shared_ptr<boost::property_tree::ptree> > const&  actions() const { return resource().m_actions; }
    bool  use_cortex_mock() const { return resource().m_use_cortex_mock; }
};


}

#endif
