#include <netlab/network.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <algorithm>
#include <limits>

namespace netlab {


network::network(std::shared_ptr<network_props> const  network_properties,
                 network_objects_factory const&  objects_factory,
                 initialiser_of_movement_area_centers&  area_centers_initialiser,
                 initialiser_of_ships_in_movement_areas&  ships_initialiser
                 )
    : m_properties(network_properties)
    , m_spikers()
    , m_docks()
    , m_ships()
    , m_movement_area_centers()
    , m_ships_in_sectors()
    , m_update_id(0UL)
{
    ASSUMPTION(this->properties().operator bool());

    m_spikers.reserve(properties()->layer_props().size());
    m_docks.reserve(properties()->layer_props().size());
    m_ships.reserve(properties()->layer_props().size());
    m_spikers.reserve(properties()->layer_props().size());

    m_movement_area_centers.resize(properties()->layer_props().size());
    m_ships_in_sectors.resize(properties()->layer_props().size());

    for (layer_index_type  layer_index = 0U; layer_index < properties()->layer_props().size(); ++layer_index)
    {
        network_layer_props const&  layer_props = properties()->layer_props().at(layer_index);

        m_spikers.push_back( objects_factory.create_array_of_spikers(layer_index,layer_props.num_spikers()) );
        ASSUMPTION(m_spikers.back()->size() == layer_props.num_spikers());

        m_docks.push_back( objects_factory.create_array_of_docks(layer_index,layer_props.num_docks()) );
        ASSUMPTION(m_docks.back()->empty() || m_docks.back()->size() == layer_props.num_docks());

        m_ships.push_back( objects_factory.create_array_of_ships(layer_index,layer_props.num_ships()) );
        ASSUMPTION(m_ships.back()->size() == layer_props.num_ships());

        {
            area_centers_initialiser.on_next_layer(layer_index, *properties());
            ships_initialiser.on_next_layer(layer_index, *properties());

            std::vector<vector3>&  centers = m_movement_area_centers.at(layer_index);
            centers.resize(layer_props.num_spikers());

            array_of_derived<ship>&  ships = *m_ships.at(layer_index);

            object_index_type  spiker_index = 0UL;
            for (sector_coordinate_type  c = 0U; c < layer_props.num_spikers_along_c_axis(); ++c)
                for (sector_coordinate_type  y = 0U; y < layer_props.num_spikers_along_y_axis(); ++y)
                    for (sector_coordinate_type  x = 0U; x < layer_props.num_spikers_along_x_axis(); ++x)
                    {
                        INVARIANT(spiker_index == layer_props.spiker_sector_index(x,y,c));

                        layer_index_type  area_layer_index;
                        area_centers_initialiser.compute_movement_area_center_for_ships_of_spiker(
                                    layer_index,
                                    spiker_index,
                                    x,y,c,
                                    *properties(),
                                    area_layer_index,
                                    centers.at(spiker_index)
                                    );
                        
                        ASSUMPTION(area_layer_index < properties()->layer_props().size());

                        network_layer_props const&  area_layer_props = properties()->layer_props().at(area_layer_index);

                        ASSUMPTION(
                            centers.at(spiker_index)(0) >= area_layer_props.low_corner_of_ships()(0) &&
                            centers.at(spiker_index)(0) <= area_layer_props.high_corner_of_ships()(0)
                            );
                        ASSUMPTION(
                            centers.at(spiker_index)(1) >= area_layer_props.low_corner_of_ships()(1) &&
                            centers.at(spiker_index)(1) <= area_layer_props.high_corner_of_ships()(1)
                            );
                        ASSUMPTION(
                            centers.at(spiker_index)(2) >= area_layer_props.low_corner_of_ships()(2) &&
                            centers.at(spiker_index)(2) <= area_layer_props.high_corner_of_ships()(2)
                            );

                        {
                            ships_initialiser.on_next_area(layer_index, spiker_index, *properties());

                            object_index_type const  ships_begin_index = layer_props.ships_begin_index_of_spiker(spiker_index);
                            for (sector_coordinate_type i = 0U; i < layer_props.num_ships_per_spiker(); ++i)
                            {
                                ships_initialiser.compute_ship_position_and_velocity_in_movement_area(
                                            centers.at(spiker_index),
                                            i,
                                            area_layer_props,
                                            ships.at(ships_begin_index + i)
                                            );
                                ASSUMPTION(
                                        [](vector3 const&  center, network_layer_props const&  props, ship const& ship_ref) {
                                            float_32_bit  speed = length(ship_ref.velocity());
                                            if (center(0) - 0.5f * props.size_of_ship_movement_area_along_x_axis_in_meters()
                                                    > ship_ref.position()(0) ||
                                                center(0) + 0.5f * props.size_of_ship_movement_area_along_x_axis_in_meters()
                                                    < ship_ref.position()(0) ||

                                                center(1) - 0.5f * props.size_of_ship_movement_area_along_y_axis_in_meters()
                                                    > ship_ref.position()(1) ||
                                                center(1) + 0.5f * props.size_of_ship_movement_area_along_y_axis_in_meters()
                                                    < ship_ref.position()(1) ||

                                                center(2) - 0.5f * props.size_of_ship_movement_area_along_c_axis_in_meters()
                                                    > ship_ref.position()(2) ||
                                                center(2) + 0.5f * props.size_of_ship_movement_area_along_c_axis_in_meters()
                                                    < ship_ref.position()(2) ||

                                                speed < props.min_speed_of_ship_in_meters_per_second() ||
                                                speed > props.max_speed_of_ship_in_meters_per_second()
                                                )
                                                return false;
                                            return true;
                                            }(centers.at(spiker_index), area_layer_props,ships.at(ships_begin_index + i))
                                        );
                            }
                        }

                        ++spiker_index;
                    }
        }

        {
            std::vector<compressed_layer_and_object_indices>  empty_dock_sector;
            empty_dock_sector.reserve(3UL + layer_props.num_ships() / layer_props.num_docks());
            m_ships_in_sectors.at(layer_index).resize(layer_props.num_docks(), empty_dock_sector);
        }
    }

    for (layer_index_type  layer_index = 0U; layer_index < properties()->layer_props().size(); ++layer_index)
        for (object_index_type  ship_index = 0UL; ship_index < m_ships.at(layer_index)->size(); ++ship_index)
        {
            vector3 const& ship_pos = m_ships.at(layer_index)->at(ship_index).position();
            layer_index_type const  area_layer_index = properties()->find_layer_index(ship_pos(2));
            ASSUMPTION(area_layer_index < properties()->layer_props().size());
            network_layer_props const&  area_layer_props = properties()->layer_props().at(area_layer_index);
            object_index_type  sector_index;
            {
                sector_coordinate_type  x, y, c;
                area_layer_props.dock_sector_coordinates(ship_pos, x, y, c);
                sector_index = area_layer_props.dock_sector_index(x, y, c);
            }
            ASSUMPTION(sector_index < m_ships_in_sectors.at(area_layer_index).size());
            m_ships_in_sectors.at(area_layer_index).at(sector_index).push_back({ layer_index, ship_index });
        }
}


spiker const&  network::get_spiker(layer_index_type const  layer_index, object_index_type const  object_index) const
{
    ASSUMPTION(layer_index < properties()->layer_props().size());
    ASSUMPTION(object_index < m_spikers.at(layer_index)->size());
    return m_spikers.at(layer_index)->at(object_index);
}


dock const&  network::get_dock(layer_index_type const  layer_index, object_index_type const  object_index) const
{
    ASSUMPTION(layer_index < properties()->layer_props().size());
    ASSUMPTION(object_index < m_docks.at(layer_index)->size());
    return m_docks.at(layer_index)->at(object_index);
}


ship const&  network::get_ship(layer_index_type const  layer_index, object_index_type const  object_index) const
{
    ASSUMPTION(layer_index < properties()->layer_props().size());
    ASSUMPTION(object_index < m_ships.at(layer_index)->size());
    return m_ships.at(layer_index)->at(object_index);
}


std::vector<compressed_layer_and_object_indices> const&  network::get_indices_of_ships_in_dock_sector(
        layer_index_type const  layer_index,
        object_index_type const  dock_sector_index
        ) const
{
    ASSUMPTION(layer_index < properties()->layer_props().size());
    ASSUMPTION(dock_sector_index < m_ships_in_sectors.at(layer_index).size());
    return m_ships_in_sectors.at(layer_index).at(dock_sector_index);
}


void  network::update(
        const bool  use_spiking,
        const bool  use_mini_spiking,
        const bool  use_movement_of_ships,
        tracked_network_object_stats* const  stats_of_tracked_object
        )
{
    TMPROF_BLOCK();

    ++m_update_id;

    if (use_movement_of_ships)
        update_movement_of_ships(dynamic_cast<tracked_ship_stats*>(stats_of_tracked_object));

//    if (use_spiking || use_mini_spiking)
//        update_spiking(!use_spiking,stats_of_tracked_spiker,stats_of_tracked_dock,stats_of_tracked_ship);
//    if (use_mini_spiking)
//        update_mini_spiking(stats_of_tracked_spiker,stats_of_tracked_dock,stats_of_tracked_ship);

//    TODO!:
//    std::swap(m_current_spikers, m_next_spikers);
//    m_next_spikers->clear();


}


//void  network::update_spiking(
//        const bool update_only_potential,
//        tracked_spiker_stats* const  stats_of_tracked_spiker,
//        tracked_dock_stats* const  stats_of_tracked_dock,
//        tracked_ship_stats* const  stats_of_tracked_ship
//        )
//{
//    TMPROF_BLOCK();

//}


//void  network::update_mini_spiking(
//        tracked_spiker_stats* const  stats_of_tracked_spiker,
//        tracked_dock_stats* const  stats_of_tracked_dock,
//        tracked_ship_stats* const  stats_of_tracked_ship
//        )
//{
//    TMPROF_BLOCK();

//}


void  network::update_movement_of_ships(tracked_ship_stats* const  stats_of_tracked_ship)
{
    TMPROF_BLOCK();

    for (layer_index_type  layer_index = 0U; layer_index < properties()->layer_props().size(); ++layer_index)
        if (properties()->layer_props().at(layer_index).ship_controller_ptr().operator bool())
            for (object_index_type  ship_index_in_layer = 0UL;
                 ship_index_in_layer < m_ships.at(layer_index)->size();
                 ++ship_index_in_layer
                 )
                update_movement_of_ship(layer_index,ship_index_in_layer,stats_of_tracked_ship);
}


void  network::update_movement_of_ship(
        layer_index_type const  layer_index,
        object_index_type const  ship_index_in_layer,
        tracked_ship_stats*  stats_of_tracked_ship
        )
{
    TMPROF_BLOCK();

    netlab::ship&  ship = m_ships.at(layer_index)->at(ship_index_in_layer);
    compressed_layer_and_object_indices const  ship_loc(layer_index,ship_index_in_layer);

    if (stats_of_tracked_ship != nullptr && ship_loc != stats_of_tracked_ship->indices())
        stats_of_tracked_ship = nullptr;

    object_index_type  spiker_sector_index;
    {
        network_layer_props const&  ship_layer_props = properties()->layer_props().at(layer_index);
        spiker_sector_index = ship_layer_props.spiker_index_from_ship_index(ship_index_in_layer);
    }
    vector3 const&  movement_area_center = m_movement_area_centers.at(layer_index).at(spiker_sector_index);

    layer_index_type const  space_layer_index = properties()->find_layer_index(movement_area_center(2));
    network_layer_props const&  space_layer_props = properties()->layer_props().at(space_layer_index);

    std::vector< std::vector<compressed_layer_and_object_indices> >&  ships_in_sectors = m_ships_in_sectors.at(space_layer_index);

    vector3  dock_sector_center_of_ship;
    bool  dock_sector_belongs_to_the_same_spiker_as_the_ship;
    {
        sector_coordinate_type  x, y, c;
        space_layer_props.dock_sector_coordinates(ship.position(), x,y,c);
        dock_sector_center_of_ship = space_layer_props.dock_sector_centre(x,y,c);

        if (layer_index == space_layer_index)
        {
            object_index_type  spiker_index;
            {
                sector_coordinate_type  spiker_x, spiker_y, spiker_c;
                space_layer_props.spiker_sector_coordinates_from_dock_sector_coordinates(
                    x, y, c,
                    spiker_x, spiker_y, spiker_c
                    );
                spiker_index = space_layer_props.spiker_sector_index(spiker_x, spiker_y, spiker_c);
            }
            dock_sector_belongs_to_the_same_spiker_as_the_ship = spiker_index == spiker_sector_index;
        }
        else
            dock_sector_belongs_to_the_same_spiker_as_the_ship = false;
    }

    vector3  ship_acceleration =
        space_layer_props.ship_controller_ptr()->accelerate_ship_in_environment(ship.velocity(),space_layer_props,*properties());
    {
        ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_into_space_box(
                ship.position(),
                ship.velocity(),
                movement_area_center,
                space_layer_props,
                *properties()
                );

        sector_coordinate_type  x_lo, y_lo, c_lo;
        sector_coordinate_type  x_hi, y_hi, c_hi;
        {
            vector3 const  range_vector(
                    space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_into_dock(),
                    space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_into_dock(),
                    space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_into_dock()
                    );
            space_layer_props.dock_sector_coordinates(ship.position() - range_vector, x_lo, y_lo, c_lo);
            space_layer_props.dock_sector_coordinates(ship.position() + range_vector, x_hi, y_hi, c_hi);
        }
        for (sector_coordinate_type x = x_lo; x <= x_hi; ++x)
            for (sector_coordinate_type y = y_lo; y <= y_hi; ++y)
                for (sector_coordinate_type c = c_lo; c <= c_hi; ++c)
                {
                    vector3 const  sector_centre = space_layer_props.dock_sector_centre(x,y,c);

                    bool  dock_belongs_to_the_same_spiker_as_the_ship;
                    {
                        if (layer_index == space_layer_index)
                        {
                            object_index_type  spiker_index;
                            {
                                sector_coordinate_type  spiker_x, spiker_y, spiker_c;
                                space_layer_props.spiker_sector_coordinates_from_dock_sector_coordinates(
                                            x,y,c,
                                            spiker_x,spiker_y,spiker_c
                                            );
                                spiker_index = space_layer_props.spiker_sector_index(spiker_x,spiker_y,spiker_c);
                            }
                            dock_belongs_to_the_same_spiker_as_the_ship = spiker_index == spiker_sector_index;
                        }
                        else
                            dock_belongs_to_the_same_spiker_as_the_ship = false;
                    }

                    if (dock_belongs_to_the_same_spiker_as_the_ship)
                        ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_from_dock(
                                ship.position(),
                                ship.velocity(),
                                sector_centre,
                                space_layer_props,
                                *properties()
                                );
                    else
                        ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_into_dock(
                                ship.position(),
                                ship.velocity(),
                                sector_centre,
                                space_layer_props,
                                *properties()
                                );

                }

        {
            vector3 const  range_vector(
                space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_from_ship(),
                space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_from_ship(),
                space_layer_props.ship_controller_ptr()->docks_enumerations_distance_for_accelerate_from_ship()
                );
            space_layer_props.dock_sector_coordinates(ship.position() - range_vector, x_lo, y_lo, c_lo);
            space_layer_props.dock_sector_coordinates(ship.position() + range_vector, x_hi, y_hi, c_hi);
        }
        for (sector_coordinate_type x = x_lo; x <= x_hi; ++x)
            for (sector_coordinate_type y = y_lo; y <= y_hi; ++y)
                for (sector_coordinate_type c = c_lo; c <= c_hi; ++c)
                {
                    object_index_type const  sector_index = space_layer_props.dock_sector_index(x,y,c);
                    for (compressed_layer_and_object_indices const  loc : ships_in_sectors.at(sector_index))
                        if (loc != ship_loc)
                        {
                            netlab::ship const&  other_ship = m_ships.at(loc.layer_index())->at(loc.object_index());

                            ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_from_ship(
                                    ship.position(),
                                    ship.velocity(),
                                    other_ship.position(),
                                    other_ship.velocity(),
                                    dock_sector_center_of_ship,
                                    dock_sector_belongs_to_the_same_spiker_as_the_ship,
                                    space_layer_props,
                                    *properties()
                                    );
                        }
                }
    }

    float_32_bit const  dt = properties()->update_time_step_in_seconds();

    vector3  new_velocity = ship.velocity() + dt * ship_acceleration;
    {
        float_32_bit const  new_speed = length(new_velocity);
        if (new_speed < space_layer_props.min_speed_of_ship_in_meters_per_second())
            space_layer_props.ship_controller_ptr()->on_too_slow(
                    new_velocity,
                    ship.position(),
                    new_speed,
                    dock_sector_center_of_ship,
                    space_layer_props,
                    *properties()
                    );
        else if (new_speed > space_layer_props.max_speed_of_ship_in_meters_per_second())
            space_layer_props.ship_controller_ptr()->on_too_fast(
                    new_velocity,
                    ship.position(),
                    new_speed,
                    dock_sector_center_of_ship,
                    space_layer_props,
                    *properties()
                    );
    }

    vector3 const  new_position = ship.position() + dt * new_velocity;

    object_index_type  old_sector_index;
    {
        sector_coordinate_type  x, y, c;
        space_layer_props.dock_sector_coordinates(ship.position(), x, y, c);
        old_sector_index = space_layer_props.dock_sector_index(x,y,c);
    }
    object_index_type  new_sector_index;
    {
        sector_coordinate_type  x, y, c;
        space_layer_props.dock_sector_coordinates(new_position, x, y, c);
        new_sector_index = space_layer_props.dock_sector_index(x,y,c);
    }
    if (new_sector_index != old_sector_index)
    {
        std::vector<compressed_layer_and_object_indices>&  old_sector = ships_in_sectors.at(old_sector_index);
        auto  it = old_sector.begin();
        while (true)
        {
            INVARIANT(it != old_sector.end());
            if (*it == ship_loc)
                break;
            ++it;
        }
        *it = old_sector.back();
        old_sector.pop_back();

        ships_in_sectors.at(new_sector_index).push_back(ship_loc);
    }

    ship.set_position(new_position);
    ship.set_velocity(new_velocity);
}


}
