#include <netexp/experiment_factory.hpp>

#define  NETEXP_DECLARE_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(namespace_name_of_the_experiment) \
            namespace netexp { namespace namespace_name_of_the_experiment { \
                extern void NETEXP_NAME_OF_EXPERIMENT_REGISTERATION_FUNCTION()(netexp::experiment_factory&  factory); \
            }}
#define  NETEXP_CALL_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(namespace_name_of_the_experiment) \
            netexp::namespace_name_of_the_experiment::NETEXP_NAME_OF_EXPERIMENT_REGISTERATION_FUNCTION()(*this);


/// Here append a declaration of the function of your experiment which performs the registeration into this factory.
NETEXP_DECLARE_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(calibration)
NETEXP_DECLARE_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(performance)
NETEXP_DECLARE_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(dbg_spiking_develop)

namespace netexp {


experiment_factory::experiment_factory()
    : m_props_creators()
    , m_factory_creators()
    , m_area_centers_creators()
    , m_ships_creators()
    , m_spiker_stats_creators()
    , m_dock_stats_creators()
    , m_ship_stats_creators()
    , m_descriptions()
{
    /// Here append a call to the registration function of your experiment declared above.
    NETEXP_CALL_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(calibration)
    NETEXP_CALL_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(performance)
    NETEXP_CALL_EXPERIMENT_REGISTERATION_FUNCTION_IN_NAMESPACE(dbg_spiking_develop)
}

experiment_factory&  experiment_factory::instance()
{
    static experiment_factory  ef;
    return ef;
}

bool  experiment_factory::register_experiment(
        std::string const&  experiment_unique_name,
        network_props_creator const&  props_creator_fn,
        network_layers_factory_creator const&  factory_creator_fn,
        initialiser_of_movement_area_centers_creator const&  initialiser_of_area_centers_fn,
        initialiser_of_ships_in_movement_areas_creator const&  initialiser_of_ships_fn,
        tracked_spiker_stats_creator const&  spiker_stats_creator_fn,
        tracked_dock_stats_creator const&  dock_stats_creator_fn,
        tracked_ship_stats_creator const&  ship_stats_creator_fn,
        std::string const&  experiment_description
        )
{
    auto const  it = m_props_creators.find(experiment_unique_name);
    if (it != m_props_creators.cend())
        return false;
    m_props_creators.insert({experiment_unique_name,props_creator_fn});
    m_factory_creators.insert({experiment_unique_name,factory_creator_fn});
    m_area_centers_creators.insert({experiment_unique_name,initialiser_of_area_centers_fn});
    m_ships_creators.insert({experiment_unique_name,initialiser_of_ships_fn});
    m_spiker_stats_creators.insert({experiment_unique_name,spiker_stats_creator_fn});
    m_dock_stats_creators.insert({experiment_unique_name,dock_stats_creator_fn});
    m_ship_stats_creators.insert({experiment_unique_name,ship_stats_creator_fn});
    m_descriptions.insert({experiment_unique_name,experiment_description});
    return true;
}

void  experiment_factory::get_names_of_registered_experiments(std::vector<std::string>&  output)
{
    for (auto it = m_props_creators.cbegin(); it != m_props_creators.cend(); ++it)
        output.push_back(it->first);
}

std::shared_ptr<netlab::network_props>  experiment_factory::create_network_props(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_props_creators.find(experiment_unique_name);
    return (it != m_props_creators.cend()) ? it->second() : nullptr;
}

std::shared_ptr<netlab::network_layers_factory>  experiment_factory::create_network_layers_factory(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_factory_creators.find(experiment_unique_name);
    return (it != m_factory_creators.cend()) ? it->second() : nullptr;
}

std::shared_ptr<netlab::initialiser_of_movement_area_centers>  experiment_factory::create_initialiser_of_movement_area_centers(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_area_centers_creators.find(experiment_unique_name);
    return (it != m_area_centers_creators.cend()) ? it->second() : nullptr;
}

std::shared_ptr<netlab::initialiser_of_ships_in_movement_areas>  experiment_factory::create_initialiser_of_ships_in_movement_areas(
        std::string const&  experiment_unique_name
        ) const
{
    auto const  it = m_ships_creators.find(experiment_unique_name);
    return (it != m_ships_creators.cend()) ? it->second() : nullptr;
}

std::shared_ptr<netlab::tracked_spiker_stats>  experiment_factory::create_tracked_spiker_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_spiker_stats_creators.find(experiment_unique_name);
    return (it != m_spiker_stats_creators.cend()) ?  it->second(indices) : nullptr;
}

std::shared_ptr<netlab::tracked_dock_stats>  experiment_factory::create_tracked_dock_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_dock_stats_creators.find(experiment_unique_name);
    return (it != m_dock_stats_creators.cend()) ? it->second(indices) : nullptr;
}

std::shared_ptr<netlab::tracked_ship_stats>  experiment_factory::create_tracked_ship_stats(
        std::string const&  experiment_unique_name,
        netlab::compressed_layer_and_object_indices const  indices
        ) const
{
    auto const  it = m_ship_stats_creators.find(experiment_unique_name);
    return (it != m_ship_stats_creators.cend()) ? it->second(indices) : nullptr;
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
