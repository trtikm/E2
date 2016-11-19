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
    : netlab::ship_controller(docks_enumerations_distance_for_accelerate_into_dock,docks_enumerations_distance_for_accelerate_from_ship)
    , m_acceleration_to_dock(acceleration_to_dock)
    , m_num_time_steps_to_stop_ship(num_time_steps_to_stop_ship)
    , m_max_acceleration_from_other_ship(max_acceleration_from_other_ship)
    , m_max_avoidance_distance_from_other_ship(max_avoidance_distance_from_other_ship)
{}


vector3  ship_controller_flat_space::accelerate_into_dock(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  dock_position,              //!< Coordinates in meters.
        netlab::network_layer_props const&  layer_props,
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

    vector3 const  doc_ship_positions_delta = dock_position - ship_position;
    float_32_bit const  distance_to_dock = length(doc_ship_positions_delta);
    vector3 const  accel_dir = (distance_to_dock < 0.0001f) ? vector3_zero() : (1.0f / distance_to_dock) * doc_ship_positions_delta;
    if (distance_to_dock < props.max_connection_distance_in_meters())
    {
        float_32_bit const  scale = distance_to_dock / props.max_connection_distance_in_meters();
        float_32_bit const  desired_speed = scale * layer_props.max_speed_of_ship_in_meters_per_second();
        vector3 const  velocity_delta = desired_speed * accel_dir - ship_velocity;
        float_32_bit const  time_delta =
            scale * static_cast<float_32_bit>(num_time_steps_to_stop_ship()) * props.update_time_step_in_seconds() + 0.001f;
        return (1.0f / time_delta) * velocity_delta;

    }
    return acceleration_to_dock() * accel_dir;
}


vector3  ship_controller_flat_space::accelerate_from_ship(
        vector3 const&  ship_position,              //!< Coordinates in meters.
        vector3 const&  ship_velocity,              //!< In meters per second.
        vector3 const&  other_ship_position,        //!< Coordinates in meters.
        vector3 const&  other_ship_velocity,        //!< In meters per second.
        vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
        bool const  both_ship_and_dock_belongs_to_same_spiker,
        netlab::network_layer_props const&  layer_props,
        netlab::network_props const&  props
        ) const
{
    TMPROF_BLOCK();

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
        float_32_bit const  squared_max_connection_distance = props.max_connection_distance_in_meters() *
            props.max_connection_distance_in_meters();

        if (length_squared(nearest_dock_position - other_ship_position) <= squared_max_connection_distance)
            accel_mag = acceleration_to_dock();
        else
        {
            if (length_squared(nearest_dock_position - ship_position) <= squared_max_connection_distance)
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
