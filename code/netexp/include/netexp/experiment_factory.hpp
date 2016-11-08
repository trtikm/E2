#ifndef NETEXP_EXPERIMENT_FACTORY_HPP_INCLUDED
#   define NETEXP_EXPERIMENT_FACTORY_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <netlab/tracked_object_stats.hpp>
#   include <memory>
#   include <string>
#   include <unordered_map>
#   include <functional>

namespace netexp {


struct  experiment_factory
{
    using  network_creator = std::function<std::shared_ptr<netlab::network>()>;

    using  tracked_spiker_stats_creator = std::function<std::shared_ptr<netlab::tracked_spiker_stats>()>;
    using  tracked_dock_stats_creator = std::function<std::shared_ptr<netlab::tracked_dock_stats>()>;
    using  tracked_ship_stats_creator = std::function<std::shared_ptr<netlab::tracked_ship_stats>()>;

    static experiment_factory&  instance();

    bool  register_experiment(
            std::string const&  experiment_unique_name,
            network_creator const&  network_creator_fn,
            tracked_spiker_stats_creator const&  spiker_stats_creator_fn,
            tracked_dock_stats_creator const&  dock_stats_creator_fn,
            tracked_ship_stats_creator const&  ship_stats_creator_fn,
            std::string const&  experiment_description
            );

    void  get_names_of_registered_experiments(std::vector<std::string>&  output);
    std::string const&  get_experiment_description(std::string const&  experiment_unique_name) const;

    std::shared_ptr<netlab::network>  create_network(std::string const&  experiment_unique_name) const;

    std::shared_ptr<netlab::tracked_spiker_stats>  create_tracked_spiker_stats(std::string const&  experiment_unique_name) const;
    std::shared_ptr<netlab::tracked_dock_stats>  create_tracked_dock_stats(std::string const&  experiment_unique_name) const;
    std::shared_ptr<netlab::tracked_ship_stats>  create_tracked_ship_stats(std::string const&  experiment_unique_name) const;

private:
    experiment_factory();

    experiment_factory(experiment_factory const&) = delete;
    experiment_factory& operator=(experiment_factory const&) = delete;
    experiment_factory(experiment_factory&&) = delete;

    std::unordered_map<std::string,network_creator>  m_network_creators;

    std::unordered_map<std::string,tracked_spiker_stats_creator>  m_spiker_stats_creators;
    std::unordered_map<std::string,tracked_dock_stats_creator>  m_dock_stats_creators;
    std::unordered_map<std::string,tracked_ship_stats_creator>  m_ship_stats_creators;

    std::unordered_map<std::string,std::string>  m_descriptions;
};


//#define  NETEXP_REGISTER_EXPERIMENT( \
//                experiment_name, \
//                network_creator, \
//                spiker_stats_creator, \
//                dock_stats_creator, \
//                ship_stats_creator, \
//                experiment_description \
//                ) \
//                volatile bool  _____E2_netexp_auxiliary_variable_netexp_dummy_var______ = \
//                    netexp::experiment_factory::instance().register_experiment( \
//                            experiment_name, \
//                            network_creator, \
//                            spiker_stats_creator, \
//                            dock_stats_creator, \
//                            ship_stats_creator, \
//                            experiment_description \
    //                            );


}


#endif
