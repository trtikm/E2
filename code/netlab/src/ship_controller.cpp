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
            vector3 const&  ship_position,
            vector3 const&  ship_velocity,
            vector3 const&  space_box_center,
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const
{
    TMPROF_BLOCK();

    vector3 const  delta = 2.0f * (ship_position - space_box_center);
    float_32_bit const  two_times_max_speed = 2.0f * layer_props.max_speed_of_ship_in_meters_per_second();

    float_32_bit const  ax =
        delta(0) >  layer_props.size_of_ship_movement_area_along_x_axis_in_meters() ? -two_times_max_speed :
        delta(0) < -layer_props.size_of_ship_movement_area_along_x_axis_in_meters() ? two_times_max_speed :
        0.0f;

    float_32_bit const  ay =
        delta(1) >  layer_props.size_of_ship_movement_area_along_y_axis_in_meters() ? -two_times_max_speed :
        delta(1) < -layer_props.size_of_ship_movement_area_along_y_axis_in_meters() ? two_times_max_speed :
        0.0f;

    float_32_bit const  ac =
        delta(2) >  layer_props.size_of_ship_movement_area_along_c_axis_in_meters() ? -two_times_max_speed :
        delta(2) < -layer_props.size_of_ship_movement_area_along_c_axis_in_meters() ? two_times_max_speed :
        0.0f;

    return{ ax, ay, ac };
}


void  ship_controller::on_too_slow(
        vector3&  ship_velocity,                    //!< In meters per second.
        vector3 const&  ship_position,              //!< Coordinates in meters.
        float_32_bit const  speed,                  //!< In meters per second.
        vector3 const&  nearest_dock_position,
        network_layer_props const&  layer_props,
        network_props const&  props
        ) const
{
    vector3 const  dock_ship_delta_vector = nearest_dock_position - ship_position;
    if (length_squared(dock_ship_delta_vector) > props.max_connection_distance_in_meters() * props.max_connection_distance_in_meters())
        angeo::get_random_vector_of_magnitude(
                0.5f * (layer_props.min_speed_of_ship_in_meters_per_second() + layer_props.max_speed_of_ship_in_meters_per_second()),
                default_random_generator(),
                ship_velocity
                );
}


void  ship_controller::on_too_fast(
        vector3&  ship_velocity,                    //!< In meters per second.
        vector3 const&  ship_position,              //!< Coordinates in meters.
        float_32_bit const  speed,                  //!< In meters per second.
        vector3 const&  nearest_dock_position,
        network_layer_props const&  layer_props,
        network_props const&  props
        ) const
{
    ship_velocity *= layer_props.max_speed_of_ship_in_meters_per_second() / speed;
}




}
