#include <netexp/experiment_factory.hpp>

/// Bellow add a declaration of a function of your experiment which performs it registeration to the factory.
namespace netexp { namespace calibration { extern void __register__(netexp::experiment_factory&  factory); }}
namespace netexp { namespace performance { extern void __register__(netexp::experiment_factory&  factory); } }

namespace netexp {


experiment_factory::experiment_factory()
    : m_network_creators()
    , m_spiker_stats_creators()
    , m_dock_stats_creators()
    , m_ship_stats_creators()
    , m_descriptions()
{
    /// Bellow add a call to the registration function of your experiment declared above.
    netexp::calibration::__register__(*this);
    netexp::performance::__register__(*this);
}

experiment_factory&  experiment_factory::instance()
{
    static experiment_factory  ef;
    return ef;
}

bool  experiment_factory::register_experiment(
        std::string const&  experiment_unique_name,
        network_creator const&  network_creator_fn,
        tracked_spiker_stats_creator const&  spiker_stats_creator_fn,
        tracked_dock_stats_creator const&  dock_stats_creator_fn,
        tracked_ship_stats_creator const&  ship_stats_creator_fn,
        std::string const&  experiment_description
        )
{
    auto const  it = m_network_creators.find(experiment_unique_name);
    if (it != m_network_creators.cend())
        return false;
    m_network_creators.insert({experiment_unique_name,network_creator_fn});
    m_spiker_stats_creators.insert({experiment_unique_name,spiker_stats_creator_fn});
    m_dock_stats_creators.insert({experiment_unique_name,dock_stats_creator_fn});
    m_ship_stats_creators.insert({experiment_unique_name,ship_stats_creator_fn});
    m_descriptions.insert({experiment_unique_name,experiment_description});
    return true;
}

void  experiment_factory::get_names_of_registered_experiments(std::vector<std::string>&  output)
{
    for (auto it = m_network_creators.cbegin(); it != m_network_creators.cend(); ++it)
        output.push_back(it->first);
}


std::shared_ptr<netlab::network>  experiment_factory::create_network(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_network_creators.find(experiment_unique_name);
    return (it != m_network_creators.cend()) ? it->second() : std::shared_ptr<netlab::network>();
}

std::shared_ptr<netlab::tracked_spiker_stats>  experiment_factory::create_tracked_spiker_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_spiker_stats_creators.find(experiment_unique_name);
    return (it != m_spiker_stats_creators.cend()) ?  it->second(indices) : std::shared_ptr<netlab::tracked_spiker_stats>();
}

std::shared_ptr<netlab::tracked_dock_stats>  experiment_factory::create_tracked_dock_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_dock_stats_creators.find(experiment_unique_name);
    return (it != m_dock_stats_creators.cend()) ? it->second(indices) : std::shared_ptr<netlab::tracked_dock_stats>();
}

std::shared_ptr<netlab::tracked_ship_stats>  experiment_factory::create_tracked_ship_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_ship_stats_creators.find(experiment_unique_name);
    return (it != m_ship_stats_creators.cend()) ? it->second(indices) : std::shared_ptr<netlab::tracked_ship_stats>();
}


std::string const&  experiment_factory::get_experiment_description(
        std::string const&  experiment_unique_name
        ) const
{
    static std::string  no_description;
    auto const  it = m_descriptions.find(experiment_unique_name);
    return (it != m_descriptions.cend()) ? it->second : no_description;
}


}
