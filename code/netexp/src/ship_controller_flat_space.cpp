#include <netexp/ship_controller_flat_space.hpp>
#include <netexp/algorithm.hpp>
#include <netlab/network_layer_props.hpp>
#include <netlab/network_props.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netexp {


ship_controller_flat_space::ship_controller_flat_space(
        float_32_bit const  docks_enumerations_distance_for_accelerate_into_dock,
        float_32_bit const  docks_enumerations_distance_for_accelerate_from_ship,
        float_32_bit const  acceleration_to_dock,
        natural_32_bit const  num_time_steps_to_stop_ship,
        float_32_bit const  max_acceleration_from_other_ship,
        float_32_bit const max_avoidance_distance_from_other_ship
        )
    : netlab::ship_controller(docks_enumerations_distance_for_accelerate_into_dock,
                              docks_enumerations_distance_for_accelerate_from_ship)
    , m_acceleration_to_dock(acceleration_to_dock)
    , m_num_time_steps_to_stop_ship(num_time_steps_to_stop_ship)
    , m_max_acceleration_from_other_ship(max_acceleration_from_other_ship)
    , m_max_avoidance_distance_from_other_ship(max_avoidance_distance_from_other_ship)
{
    ASSUMPTION(m_num_time_steps_to_stop_ship >= 1U);
}


vector3  ship_controller_flat_space::accelerate_into_dock(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  dock_position,              //!< Coordinates in meters.
        netlab::layer_index_type const  home_layer_index,//!< Index of layer where is the spiker the ship belongs to.
        netlab::layer_index_type const  area_layer_index,//!< Index of layer where is the movement area in which the ship moves.
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

    vector3 const  dock_ship_positions_delta = dock_position - ship_position;
    float_32_bit const  distance_to_dock = length(dock_ship_positions_delta);
    vector3 const  accel_dir =
            (distance_to_dock < 0.001f) ? vector3_zero() : (1.0f / distance_to_dock) * dock_ship_positions_delta;
    if (distance_to_dock >= props.max_connection_distance_in_meters())
        return acceleration_to_dock() * accel_dir;

    float_32_bit const  desired_speed = distance_to_dock / (num_time_steps_to_stop_ship() * props.update_time_step_in_seconds());

    return (1.0f / props.update_time_step_in_seconds()) * (desired_speed * accel_dir - ship_velocity);
}


vector3  ship_controller_flat_space::accelerate_from_ship(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  other_ship_position,        //!< Coordinates in meters.
        vector3 const&  other_ship_velocity,        //!< In meters per second.
        vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
        bool const  both_ship_and_dock_belongs_to_same_spiker,
        netlab::layer_index_type const  home_layer_index,//!< Index of layer where is the spiker the ship belongs to.
        netlab::layer_index_type const  area_layer_index,//!< Index of layer where is the movement area in which the ship moves.
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

    //vector3 const  ship_positions_delta = ship_position - other_ship_position;
    //float_32_bit const  squared_distance_of_ships = length_squared(ship_positions_delta);

    //if (squared_distance_of_ships > max_avoidance_distance_from_other_ship() * max_avoidance_distance_from_other_ship())
    //    return vector3_zero();

    //float_32_bit const  squared_distance_of_ship_to_dock = length_squared(nearest_dock_position - ship_position);
    //float_32_bit const  squared_distance_of_other_ship_to_dock = length_squared(nearest_dock_position - ship_position);
    //float_32_bit const  min_squared_distance_of_ships_to_dock =
    //        std::min(squared_distance_of_ship_to_dock,squared_distance_of_other_ship_to_dock);
    //float_32_bit const  squared_max_connection_distance =
    //        props.max_connection_distance_in_meters() * props.max_connection_distance_in_meters();

    //if (min_squared_distance_of_ships_to_dock <= squared_max_connection_distance &&
    //    squared_distance_of_ship_to_dock < squared_distance_of_other_ship_to_dock)
    //        return vector3_zero();

    //if (squared_distance_of_ships < 1e-6f)
    //{
    //    vector3  acceleration;
    //    angeo::get_random_vector_of_magnitude(max_acceleration_from_other_ship(), default_random_generator(), acceleration);
    //    return acceleration;
    //}

    //float_32_bit const  distance_of_ships = std::sqrtf(squared_distance_of_ships);
    //float_32_bit const  repulsion_scale =
    //    exponential_increase_from_zero_to_one(1.0f - distance_of_ships / max_avoidance_distance_from_other_ship(), 1.0f, 1.0f);

    //vector3  acceleration = ((repulsion_scale * max_acceleration_from_other_ship()) / distance_of_ships) * ship_positions_delta;

    //if (!both_ship_and_dock_belongs_to_same_spiker && squared_distance_of_other_ship_to_dock <= squared_max_connection_distance)
    //    acceleration += acceleration_to_dock() * normalised(ship_position - nearest_dock_position);

    //return acceleration;

////////////////////////////////////////////////////////////////////////////////////////////////////////////

    vector3 const  ship_positions_delta = ship_position - other_ship_position;
    float_32_bit const  squared_distance_of_ships = length_squared(ship_positions_delta);

    if (squared_distance_of_ships < 1e-6f)
    {
        vector3  acceleration;
        angeo::get_random_vector_of_magnitude(max_acceleration_from_other_ship(), default_random_generator(), acceleration);
        return acceleration;
    }

    float_32_bit  accel_mag;

    if (both_ship_and_dock_belongs_to_same_spiker)
        accel_mag = 0.0f;
    else
    {
        if (netlab::are_ship_and_dock_connected(other_ship_position,nearest_dock_position,props.max_connection_distance_in_meters()))
            accel_mag = acceleration_to_dock();
        else
        {
            if (netlab::are_ship_and_dock_connected(ship_position,nearest_dock_position,props.max_connection_distance_in_meters()))
                return vector3_zero();

            accel_mag = 0.0f;
        }
    }

    if (squared_distance_of_ships < max_avoidance_distance_from_other_ship() * max_avoidance_distance_from_other_ship())
    {
        float_32_bit const  distance_of_ships = std::sqrtf(squared_distance_of_ships);
        float_32_bit const  repulsion_scale =
            exponential_increase_from_zero_to_one(1.0f - distance_of_ships / max_avoidance_distance_from_other_ship(), 1.0f, 1.0f);
        accel_mag += repulsion_scale * max_acceleration_from_other_ship();

        return (accel_mag / distance_of_ships) * ship_positions_delta;
    }

    if (accel_mag > 0.0f)
    {
        float_32_bit const  distance_of_ships = std::sqrtf(squared_distance_of_ships);
        return (accel_mag / distance_of_ships) * ship_positions_delta;
    }

    return vector3_zero();
}


}
