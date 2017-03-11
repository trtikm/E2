#include <netlab/ship_controller.hpp>
#include <netlab/network_props.hpp>
#include <netlab/network_layer_props.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


ship_controller::ship_controller(
        float_32_bit const  docks_enumerations_distance_for_accelerate_into_dock,
        float_32_bit const  docks_enumerations_distance_for_accelerate_from_ship
        )
    : m_docks_enumerations_distance_for_accelerate_into_dock(docks_enumerations_distance_for_accelerate_into_dock)
    , m_docks_enumerations_distance_for_accelerate_from_ship(docks_enumerations_distance_for_accelerate_from_ship)
{
    ASSUMPTION(m_docks_enumerations_distance_for_accelerate_into_dock >= 0.0f);
    ASSUMPTION(m_docks_enumerations_distance_for_accelerate_from_ship >= 0.0f);
}


vector3  ship_controller::accelerate_into_space_box(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  space_box_center,           //!< Coordinates in meters.
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props
            ) const
{
    TMPROF_BLOCK();

    network_layer_props const&  layer_props = props.layer_props().at(home_layer_index);

    vector3 const  delta = 2.0f * (ship_position - space_box_center);
    float_32_bit const  two_times_max_speed = 2.0f * layer_props.max_speed_of_ship_in_meters_per_second(area_layer_index);

    float_32_bit const  ax =
        delta(0) >  layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index) ? -two_times_max_speed :
        delta(0) < -layer_props.size_of_ship_movement_area_along_x_axis_in_meters(area_layer_index) ? two_times_max_speed :
        0.0f;

    float_32_bit const  ay =
        delta(1) >  layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index) ? -two_times_max_speed :
        delta(1) < -layer_props.size_of_ship_movement_area_along_y_axis_in_meters(area_layer_index) ? two_times_max_speed :
        0.0f;

    float_32_bit const  ac =
        delta(2) >  layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index) ? -two_times_max_speed :
        delta(2) < -layer_props.size_of_ship_movement_area_along_c_axis_in_meters(area_layer_index) ? two_times_max_speed :
        0.0f;

    return{ ax, ay, ac };
}


void  ship_controller::on_too_slow(
        vector3&  ship_velocity,                    //!< In meters per second.
        vector3 const&  ship_position,              //!< Coordinates in meters.
        float_32_bit const  speed,                  //!< In meters per second. Equals to lenght of 'ship_velocity'.
        vector3 const&  nearest_dock_position,
        layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
        layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
        network_props const&  props
        ) const
{
    if (!are_ship_and_dock_connected(ship_position,nearest_dock_position,props.max_connection_distance_in_meters()))
        angeo::get_random_vector_of_magnitude(
                0.5f * (props.layer_props().at(home_layer_index).min_speed_of_ship_in_meters_per_second(area_layer_index) +
                        props.layer_props().at(home_layer_index).max_speed_of_ship_in_meters_per_second(area_layer_index)),
                default_random_generator(),
                ship_velocity
                );
}


void  ship_controller::on_too_fast(
        vector3&  ship_velocity,                    //!< In meters per second.
        vector3 const&  ship_position,              //!< Coordinates in meters.
        float_32_bit const  speed,                  //!< In meters per second. Equals to lenght of 'ship_velocity'.
        vector3 const&  nearest_dock_position,
        layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
        layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
        network_props const&  props
        ) const
{
    if (speed > 1e-3f)
        ship_velocity *= props.layer_props().at(home_layer_index).max_speed_of_ship_in_meters_per_second(area_layer_index) / speed;
}


bool  ship_controller::is_ship_docked(
        vector3 const&  ship_position,
        vector3 const&  ship_velocity,
        layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
        network_props const&  props
        ) const
{
    network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

    sector_coordinate_type  x,y,c;
    area_layer_props.dock_sector_coordinates(ship_position,x,y,c);
    vector3 const  nearest_dock_position = area_layer_props.dock_sector_centre(x,y,c);

    float_32_bit const  docking_radius = props.max_connection_distance_in_meters() / 3.0f;
    float_32_bit const  docking_radius_squared = docking_radius * docking_radius;
    float_32_bit const  max_docking_speed = docking_radius / 1.0f;
    float_32_bit const  max_docking_speed_squared = max_docking_speed * max_docking_speed;

    if (length_squared(nearest_dock_position - ship_position) <= docking_radius_squared &&
        length_squared(ship_velocity) <= max_docking_speed_squared)
        return true;

    return false;
}


}
