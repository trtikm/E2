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
    using  network_props_creator =
            std::function<std::shared_ptr<netlab::network_props>()>;
    using  network_layers_factory_creator =
            std::function<std::shared_ptr<netlab::network_layers_factory>()>;
    using  initialiser_of_movement_area_centers_creator =
            std::function<std::shared_ptr<netlab::initialiser_of_movement_area_centers>()>;
    using  initialiser_of_ships_in_movement_areas_creator =
            std::function<std::shared_ptr<netlab::initialiser_of_ships_in_movement_areas>()>;
    using  tracked_spiker_stats_creator =
            std::function<std::shared_ptr<netlab::tracked_spiker_stats>(netlab::compressed_layer_and_object_indices)>;
    using  tracked_dock_stats_creator =
            std::function<std::shared_ptr<netlab::tracked_dock_stats>(netlab::compressed_layer_and_object_indices)>;
    using  tracked_ship_stats_creator =
            std::function<std::shared_ptr<netlab::tracked_ship_stats>(netlab::compressed_layer_and_object_indices)>;

    static experiment_factory&  instance();

    bool  register_experiment(
            std::string const&  experiment_unique_name,
            network_props_creator const&  props_creator_fn,
            network_layers_factory_creator const&  factory_creator_fn,
            initialiser_of_movement_area_centers_creator const&  initialiser_of_area_centers_fn,
            initialiser_of_ships_in_movement_areas_creator const&  initialiser_of_ships_fn,
            tracked_spiker_stats_creator const&  spiker_stats_creator_fn,
            tracked_dock_stats_creator const&  dock_stats_creator_fn,
            tracked_ship_stats_creator const&  ship_stats_creator_fn,
            std::string const&  experiment_description
            );

    void  get_names_of_registered_experiments(std::vector<std::string>&  output);
    std::string const&  get_experiment_description(std::string const&  experiment_unique_name) const;

    std::shared_ptr<netlab::network_props>  create_network_props(
            std::string const&  experiment_unique_name
            ) const;
    std::shared_ptr<netlab::network_layers_factory>  create_network_layers_factory(
            std::string const&  experiment_unique_name
            ) const;
    std::shared_ptr<netlab::initialiser_of_movement_area_centers>  create_initialiser_of_movement_area_centers(
            std::string const&  experiment_unique_name
            ) const;
    std::shared_ptr<netlab::initialiser_of_ships_in_movement_areas>  create_initialiser_of_ships_in_movement_areas(
            std::string const&  experiment_unique_name
            ) const;
    std::shared_ptr<netlab::tracked_spiker_stats>  create_tracked_spiker_stats(
            std::string const&  experiment_unique_name,
            netlab::compressed_layer_and_object_indices const  indices
            ) const;
    std::shared_ptr<netlab::tracked_dock_stats>  create_tracked_dock_stats(
            std::string const&  experiment_unique_name,
            netlab::compressed_layer_and_object_indices const  indices
            ) const;
    std::shared_ptr<netlab::tracked_ship_stats>  create_tracked_ship_stats(
            std::string const&  experiment_unique_name,
            netlab::compressed_layer_and_object_indices const  indices
            ) const;

private:
    experiment_factory();

    experiment_factory(experiment_factory const&) = delete;
    experiment_factory& operator=(experiment_factory const&) = delete;
    experiment_factory(experiment_factory&&) = delete;

    std::unordered_map<std::string,network_props_creator>  m_props_creators;
    std::unordered_map<std::string,network_layers_factory_creator>  m_factory_creators;
    std::unordered_map<std::string,initialiser_of_movement_area_centers_creator>  m_area_centers_creators;
    std::unordered_map<std::string,initialiser_of_ships_in_movement_areas_creator>  m_ships_creators;
    std::unordered_map<std::string,tracked_spiker_stats_creator>  m_spiker_stats_creators;
    std::unordered_map<std::string,tracked_dock_stats_creator>  m_dock_stats_creators;
    std::unordered_map<std::string,tracked_ship_stats_creator>  m_ship_stats_creators;

    std::unordered_map<std::string,std::string>  m_descriptions;
};


#define  NETEXP_NAME_OF_EXPERIMENT_REGISTERATION_FUNCTION()  __experiment_registration_function__

#define  NETEXP_DEFINE_EXPERIMENT_REGISTERATION_FUNCTION(               \
            namespace_name,                                             \
            experiment_name,                                            \
            network_props_creator_fn,                                   \
            network_layers_factory_creator_fn,                          \
            initialiser_of_movement_area_centers_creator_fn,            \
            initialiser_of_ships_in_movement_areas_creator_fn,          \
            tracked_spiker_stats_creator_fn,                            \
            tracked_dock_stats_creator_fn,                              \
            tracked_ship_stats_creator_fn,                              \
            experiment_description_text                                 \
            )                                                           \
        namespace netexp { namespace namespace_name {                   \
            void NETEXP_NAME_OF_EXPERIMENT_REGISTERATION_FUNCTION()(    \
                    netexp::experiment_factory&  factory                \
                    )                                                   \
            {                                                           \
                factory.register_experiment(                            \
                    experiment_name,                                    \
                    network_props_creator_fn,                           \
                    network_layers_factory_creator_fn,                  \
                    initialiser_of_movement_area_centers_creator_fn,    \
                    initialiser_of_ships_in_movement_areas_creator_fn,  \
                    tracked_spiker_stats_creator_fn,                    \
                    tracked_dock_stats_creator_fn,                      \
                    tracked_ship_stats_creator_fn,                      \
                    experiment_description_text                         \
                    );                                                  \
            }                                                           \
        }}


}


#endif
