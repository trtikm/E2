#ifndef NETLAB_NETWORK_HPP_INCLUDED
#   define NETLAB_NETWORK_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <netlab/network_props.hpp>
#   include <netlab/network_objects.hpp>
#   include <netlab/network_objects_factory.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/ship_controller.hpp>
#   include <netlab/extra_data_for_spikers.hpp>
#   include <netlab/initialiser_of_movement_area_centers.hpp>
#   include <netlab/initialiser_of_ships_in_movement_areas.hpp>
#   include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#   include <netlab/tracked_object_stats.hpp>
#   include <utility/array_of_derived.hpp>
#   include <utility/random.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <memory>
#   include <string>
#   include <deque>
#   include <unordered_set>

namespace netlab {


enum struct  NETWORK_STATE : natural_8_bit 
{
    READY_FOR_CONSTRUCTION                                      = 0U,
    READY_FOR_MOVEMENT_AREA_CENTERS_INITIALISATION              = 1U,
    READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STARTUP           = 2U,
    READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP              = 3U,
    READY_FOR_COMPUTATION_OF_SHIP_DENSITIES_IN_LAYERS           = 4U,
    READY_FOR_LUNCHING_SHIPS_INTO_MOVEMENT_AREAS                = 5U,
    READY_FOR_INITIALISATION_OF_MAP_FROM_DOCK_SECTORS_TO_SHIPS  = 6U,
    READY_FOR_SIMULATION_STEP                                   = 7U,
};


}


std::string const&  to_string(netlab::NETWORK_STATE const  state);


namespace netlab {


/**
 *
 *
 *
 *
 */
struct  network
{
    network(std::shared_ptr<network_props> const  network_properties,
            std::shared_ptr<network_layers_factory> const  layers_factory
            );

    std::shared_ptr<network_props>  properties() const noexcept { return m_properties; }
    NETWORK_STATE  get_state() const noexcept { return m_state; }

    layer_of_spikers const&  get_layer_of_spikers(layer_index_type const  layer_index) const
    { return *m_layers_of_spikers.at(layer_index); }

    layer_of_docks const&  get_layer_of_docks(layer_index_type const  layer_index) const
    { return *m_layers_of_docks.at(layer_index); }

    layer_of_ships const&  get_layer_of_ships(layer_index_type const  layer_index) const
    { return *m_layers_of_ships.at(layer_index); }

    vector3 const&  get_center_of_movement_area(
            layer_index_type const  layer_index,
            object_index_type const  spiker_index   //!< Indeed spiker's index, NOT ship! All ships of the spaker
                                                    //!< share the center of the movement area. So, the center is
                                                    //!< associated to their common spiker.
            ) const
    { return m_movement_area_centers.at(layer_index).at(spiker_index); }

    std::vector<compressed_layer_and_object_indices> const&  get_indices_of_ships_in_dock_sector(
            layer_index_type const  layer_index,
            object_index_type const  dock_sector_index
            ) const
    { return m_ships_in_sectors.at(layer_index).at(dock_sector_index); }

    statistics_of_densities_of_ships_in_layers const&  densities_of_ships() const { return *m_densities_of_ships; }

    natural_64_bit  update_id() const noexcept { return  m_update_id; }

    extra_data_for_spikers_in_one_layer::value_type  get_extra_data_of_spiker(
            layer_index_type const  layer_index,
            object_index_type const  object_index
            ) const;

    using  element_type_in_update_queue_of_ships = compressed_layer_and_object_indices;
    void  enable_usage_of_queues_in_update_of_ships(bool const  enable_state) { m_use_update_queue_of_ships = enable_state; }
    bool  is_update_queue_of_ships_used() const { return m_use_update_queue_of_ships; }
    bool  is_update_queue_of_ships_overloaded() const { return m_is_update_queue_of_ships_overloaded; }
    natural_64_bit  size_of_update_queue_of_ships() const { return m_update_queue_of_ships.size(); }
    natural_64_bit  max_size_of_update_queue_of_ships() const { return m_max_size_of_update_queue_of_ships; }

    void  initialise_movement_area_centers(initialiser_of_movement_area_centers&  area_centers_initialiser);
    void  prepare_for_movement_area_centers_migration(initialiser_of_movement_area_centers&  area_centers_initialiser);
    void  do_movement_area_centers_migration_step(initialiser_of_movement_area_centers&  area_centers_initialiser);
    void  compute_densities_of_ships_in_layers();
    void  lunch_ships_into_movement_areas(initialiser_of_ships_in_movement_areas&  ships_initialiser);
    void  initialise_map_from_dock_sectors_to_ships();

    void  do_simulation_step(
            bool const  use_spiking = true,
            bool const  use_mini_spiking = true,
            bool const  use_movement_of_ships = true,
            tracked_network_object_stats* const  stats_of_tracked_object = nullptr
            );

private:

    network(network const&) = delete;
    network& operator=(network const&) = delete;

    void  update_movement_of_ships(tracked_ship_stats* const  stats_of_tracked_ship);
    void  update_movement_of_ship(
            layer_index_type const  layer_index,
            object_index_type const  ship_index_in_layer,
            tracked_ship_stats*  stats_of_tracked_ship
            );

    void  update_mini_spiking(
            const bool  use_spiking,
            tracked_network_object_stats* const  stats_of_tracked_object
            );

    void  update_spiking(
            tracked_network_object_stats* const  stats_of_tracked_object
            );

    std::shared_ptr<network_props>  m_properties;
    NETWORK_STATE  m_state;

    std::unique_ptr<extra_data_for_spikers_in_layers>  m_extra_data_for_spikers;

    std::vector<std::unique_ptr<layer_of_spikers> >  m_layers_of_spikers;
    std::vector<std::unique_ptr<layer_of_docks> >  m_layers_of_docks;
    std::vector<std::unique_ptr<layer_of_ships> >  m_layers_of_ships;

    std::vector< std::vector<vector3> >  m_movement_area_centers;
    std::vector< std::vector< std::vector<compressed_layer_and_object_indices> > >  m_ships_in_sectors;

    std::unique_ptr<statistics_of_densities_of_ships_in_layers>  m_densities_of_ships;

    natural_64_bit  m_update_id;
    
    /// Data of update queue of ships
    std::deque<element_type_in_update_queue_of_ships>  m_update_queue_of_ships;
    natural_64_bit  m_max_size_of_update_queue_of_ships;
    bool  m_is_update_queue_of_ships_overloaded;
    bool  m_use_update_queue_of_ships;

    random_generator_for_natural_64_bit  m_mini_spiking_random_generator;

    std::unique_ptr< std::unordered_set<compressed_layer_and_object_indices> >  m_current_spikers;
    std::unique_ptr< std::unordered_set<compressed_layer_and_object_indices> >  m_next_spikers;
};


}


#endif
