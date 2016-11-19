#ifndef NETEXP_SHIP_CONTROLLER_FLAT_SPACE_HPP_INCLUDED
#   define NETEXP_SHIP_CONTROLLER_FLAT_SPACE_HPP_INCLUDED

#   include <netlab/ship_controller.hpp>

namespace netexp {


struct  ship_controller_flat_space : public netlab::ship_controller
{
    ship_controller_flat_space(
            float_32_bit const  docks_enumerations_distance_for_accelerate_into_dock,
            float_32_bit const  docks_enumerations_distance_for_accelerate_from_ship,
            float_32_bit const  acceleration_to_dock,
            natural_32_bit const  num_time_steps_to_stop_ship,
            float_32_bit const  max_acceleration_from_other_ship,
            float_32_bit const max_avoidance_distance_from_other_ship
            );


    vector3  accelerate_into_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            netlab::network_layer_props const&  layer_props,
            netlab::network_props const&  props
            ) const;


    vector3  accelerate_from_ship(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  other_ship_position,        //!< Coordinates in meters.
            vector3 const&  other_ship_velocity,        //!< In meters per second.
            vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
            bool const  both_ship_and_dock_belongs_to_same_spiker,
            netlab::network_layer_props const&  layer_props,
            netlab::network_props const&  props
            ) const;


    float_32_bit  acceleration_to_dock() const noexcept { return m_acceleration_to_dock; }
    natural_32_bit  num_time_steps_to_stop_ship() const noexcept { return m_num_time_steps_to_stop_ship; }
    float_32_bit  max_acceleration_from_other_ship() const noexcept { return m_max_acceleration_from_other_ship; }
    float_32_bit  max_avoidance_distance_from_other_ship() const noexcept { return m_max_avoidance_distance_from_other_ship; }

private:
    float_32_bit  m_acceleration_to_dock;
    natural_32_bit  m_num_time_steps_to_stop_ship;
    float_32_bit  m_max_acceleration_from_other_ship;
    float_32_bit  m_max_avoidance_distance_from_other_ship;
};


}

#endif
