#include <netlab/network.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <algorithm>
#include <limits>

namespace netlab { namespace detail { namespace {


vector3 gen_random_velocity(float_32_bit const  magnitude)
{
    while (true)
    {
        natural_32_bit const  tx = get_random_natural_32_bit_in_range(0U, 10000U);
        natural_32_bit const  ty = get_random_natural_32_bit_in_range(0U, 10000U);
        natural_32_bit const  tc = get_random_natural_32_bit_in_range(0U, 10000U);
        vector3 const  u( ((float_32_bit)tx / 10000.0f) - 0.5f ,
                          ((float_32_bit)ty / 10000.0f) - 0.5f ,
                          ((float_32_bit)tc / 10000.0f) - 0.5f );
        float_32_bit const  u_len = length(u);
        if (u_len >= 0.001f)
            return (magnitude / u_len) * u;
    }
}

vector3  update_magnitude_of_velocity(
        vector3 const&  velocity,
        float_32_bit const  desired_magnitude,
        float_32_bit const  min_magnitude,
        float_32_bit const  max_magnitude
        )
{
    float_32_bit const  speed = length(velocity);
    if (speed >= min_magnitude)
    {
        if (speed <= max_magnitude)
            return velocity;
        return (max_magnitude / speed) * velocity;
    }
    return gen_random_velocity(desired_magnitude);
}


}}}

namespace netlab {


network::network(std::shared_ptr<network_props> const  properties, network_objects_factory const&  objects_factory)
    : m_properties(properties)
    , m_spikers()
    , m_docks()
    , m_ships()
    , m_movement_area_centers()
    , m_ships_in_sectors()
    , m_update_id(0UL)
{
    ASSUMPTION(this->properties().operator bool());

    m_spikers.reserve(this->properties()->layer_props().size());
    m_docks.reserve(this->properties()->layer_props().size());
    m_ships.reserve(this->properties()->layer_props().size());
    m_spikers.reserve(this->properties()->layer_props().size());
    m_movement_area_centers.reserve(this->properties()->layer_props().size());
    m_ships_in_sectors.reserve(this->properties()->layer_props().size());

    for (natural_8_bit  layer_index = 0U; layer_index < this->properties()->layer_props().size(); ++layer_index)
    {
        network_layer_props const&  layer_props = this->properties()->layer_props().at(layer_index);

        m_spikers.push_back( objects_factory.create_array_of_spikers(layer_index,layer_props.num_spikers()) );
        m_docks.push_back( objects_factory.create_array_of_docks(layer_index,layer_props.num_docks()) );
        m_ships.push_back( objects_factory.create_array_of_ships(layer_index,layer_props.num_ships()) );

        m_movement_area_centers.at(layer_index).resize(layer_props.num_spikers(),vector3(0.0f,0.0f,0.0f));

        std::vector<network_object_id>  empty_dock_sector;
        empty_dock_sector.reserve(3UL + layer_props.num_ships() / layer_props.num_docks());
        m_ships_in_sectors.at(layer_index).resize(layer_props.num_docks(),empty_dock_sector);
    }
}


void  network::update(
        const bool  use_spiking,
        const bool  use_mini_spiking,
        const bool  use_movement_of_ships,
        tracked_spiker_stats* const  stats_of_tracked_spiker,
        tracked_dock_stats* const  stats_of_tracked_dock,
        tracked_ship_stats* const  stats_of_tracked_ship
        )
{
    TMPROF_BLOCK();

    ++m_update_id;

    if (use_movement_of_ships)
        update_movement_of_ships();

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


void  network::update_movement_of_ships()
{
    TMPROF_BLOCK();

    for (natural_8_bit  layer_index = 0U; layer_index < properties()->layer_props().size(); ++layer_index)
        if (properties()->layer_props().at(layer_index).ship_controller_ptr().operator bool())
            for (natural_64_bit  ship_index_in_layer = 0UL;
                 ship_index_in_layer < m_ships.at(layer_index)->size();
                 ++ship_index_in_layer
                 )
                update_movement_of_ship(layer_index,ship_index_in_layer);
}


void  network::update_movement_of_ship(natural_8_bit const  layer_index, natural_64_bit const  ship_index_in_layer)
{
    TMPROF_BLOCK();

    netlab::ship&  ship = m_ships.at(layer_index)->at(ship_index_in_layer);
    network_object_id const  ship_id(object_kind_ship(),layer_index,ship_index_in_layer);

    natural_64_bit  spiker_sector_index;
    {
        network_layer_props const&  ship_layer_props = properties()->layer_props().at(layer_index);
        natural_32_bit  x, y, c;
        ship_layer_props.spiker_sector_coordinates(ship.position(),x,y,c);
        spiker_sector_index = ship_layer_props.spiker_sector_index(x,y,c);
    }
    vector3 const&  movement_area_center = m_movement_area_centers.at(layer_index).at(spiker_sector_index);

    natural_8_bit const  space_layer_index = properties()->find_layer_index(movement_area_center(2));
    network_layer_props const&  space_layer_props = properties()->layer_props().at(space_layer_index);

    natural_32_bit  sector_x, sector_y, sector_c;
    space_layer_props.dock_sector_coordinates(ship.position(), sector_x, sector_y, sector_c);

    std::vector< std::vector<network_object_id> >&  ships_in_sectors = m_ships_in_sectors.at(space_layer_index);

    vector3  ship_acceleration = vector3_zero();
    {
        ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_into_space_box(
                ship.position(),
                ship.velocity(),
                movement_area_center,
                space_layer_props.size_of_ship_movement_area_along_x_axis_in_meters(),
                space_layer_props.size_of_ship_movement_area_along_y_axis_in_meters(),
                space_layer_props.size_of_ship_movement_area_along_c_axis_in_meters()
                );

        vector3 const  sector_centre = space_layer_props.dock_sector_centre(sector_x,sector_y,sector_c);

        natural_32_bit  x_lo, y_lo, c_lo;
        natural_32_bit  x_hi, y_hi, c_hi;
        {
            vector3 const  range_vector(
                    0.499f * space_layer_props.distance_of_docks_in_meters(),
                    0.499f * space_layer_props.distance_of_docks_in_meters(),
                    0.499f * space_layer_props.distance_of_docks_in_meters()
                    );
            space_layer_props.dock_sector_coordinates(ship.position() - range_vector, x_lo, y_lo, c_lo);
            space_layer_props.dock_sector_coordinates(ship.position() + range_vector, x_hi, y_hi, c_hi);
        }
        for (natural_32_bit x = x_lo; x <= x_hi; ++x)
            for (natural_32_bit y = y_lo; y <= y_hi; ++y)
                for (natural_32_bit c = c_lo; c <= c_hi; ++c)
                {
                    ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_into_dock(
                            ship.position(),
                            ship.velocity(),
                            sector_centre,
                            space_layer_props.distance_of_docks_in_meters()
                            );

                    natural_64_bit const  sector_index = space_layer_props.dock_sector_index(x,y,c);
                    for (network_object_id const  id : ships_in_sectors.at(sector_index))
                        if (id != ship_id)
                        {
                            netlab::ship const&  other_ship = m_ships.at(id.index_of_network_layer())
                                                                     ->at(id.index_into_network_layer());

                            ship_acceleration += space_layer_props.ship_controller_ptr()->accelerate_from_ship(
                                    ship.position(),
                                    ship.velocity(),
                                    other_ship.position(),
                                    other_ship.velocity(),
                                    sector_centre,
                                    space_layer_props.distance_of_docks_in_meters()
                                    );
                        }
                }
    }

    float_32_bit const  dt = 1000.0f * properties()->update_time_step_in_seconds();
    vector3 const  new_velocity =
            detail::update_magnitude_of_velocity(
                ship.velocity() + dt * ship_acceleration,
                space_layer_props.min_speed_of_ship_in_meters_per_second(),
                space_layer_props.min_speed_of_ship_in_meters_per_second(),
                space_layer_props.max_speed_of_ship_in_meters_per_second()
                );
    vector3 const  new_position = ship.position() + dt * new_velocity;

    natural_64_bit const  old_sector_index = space_layer_props.dock_sector_index(sector_x, sector_y, sector_c);

    natural_64_bit  new_sector_index;
    {
        natural_32_bit  x, y, c;
        space_layer_props.dock_sector_coordinates(new_position, x, y, c);
        new_sector_index = space_layer_props.dock_sector_index(x,y,c);
    }
    if (new_sector_index != old_sector_index)
    {
        std::vector<network_object_id>&  old_sector = ships_in_sectors.at(old_sector_index);
        auto  it = old_sector.begin();
        while (true)
        {
            INVARIANT(it != old_sector.end());
            if (*it == ship_id)
                break;
            ++it;
        }
        *it = old_sector.back();
        old_sector.pop_back();

        ships_in_sectors.at(new_sector_index).push_back(ship_id);
    }

    ship.set_position(new_position);
    ship.set_velocity(new_velocity);
}


}
