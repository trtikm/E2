#ifndef NETLAB_NETWORK_HPP_INCLUDED
#   define NETLAB_NETWORK_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <netlab/network_props.hpp>
#   include <netlab/network_objects.hpp>
#   include <netlab/network_object_id.hpp>
#   include <netlab/ship_controller.hpp>
#   include <netlab/tracked_object_stats.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <memory>

namespace netlab {


/**
 *
 *
 *
 *
 */
struct network
{
    explicit  network(std::shared_ptr<network_props> const  properties);

    std::shared_ptr<network_props>  properties() const noexcept { return m_properties; }
    natural_64_bit  update_id() const noexcept { return  m_update_id; }

    void  update(
            bool const  use_spiking = true,
            bool const  use_mini_spiking = true,
            bool const  use_movement_of_ships = true,
            tracked_spiker_stats* const  stats_of_tracked_spiker = nullptr,
            tracked_dock_stats* const  stats_of_tracked_dock = nullptr,
            tracked_ship_stats* const  stats_of_tracked_ship = nullptr
            );

private:

    natural_8_bit  find_layer_index(float_32_bit const  coord_along_c_axis);

//    void  update_spiking(
//            bool const  update_only_potential,
//            tracked_spiker_stats* const  stats_of_tracked_spiker,
//            tracked_dock_stats* const  stats_of_tracked_dock,
//            tracked_ship_stats* const  stats_of_tracked_ship
//            );
//    void  update_mini_spiking(
//            tracked_spiker_stats* const  stats_of_tracked_spiker,
//            tracked_dock_stats* const  stats_of_tracked_dock,
//            tracked_ship_stats* const  stats_of_tracked_ship
//            );

    void  update_movement_of_ships();
    void  update_movement_of_ship(natural_8_bit const  layer_index, natural_64_bit const  ship_index_in_layer);


    std::shared_ptr<network_props>  m_properties;
    std::vector<float_32_bit>  m_max_coods_along_c_axis;

    std::vector< std::vector<spiker> >  m_spikers;

    std::vector< std::vector<dock> >  m_docks;

    std::vector< std::vector<ship> >  m_ships;
    std::vector< std::vector<vector3> >  m_movement_area_centers;
    std::vector< std::vector< std::vector<network_object_id> > >  m_ships_in_sectors;

    natural_64_bit  m_update_id;

//    TODO!:
//    std::unique_ptr< std::unordered_set<cell*> >  m_current_spikers;
//    std::unique_ptr< std::unordered_set<cell*> >  m_next_spikers;
};


}

#endif
