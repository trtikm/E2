#ifndef NETLAB_SHIP_CONTROLLER_HPP_INCLUDED
#   define NETLAB_SHIP_CONTROLLER_HPP_INCLUDED

#   include <utility/tensor_math.hpp>

namespace netlab {


struct  ship_controller
{
    virtual ~ship_controller() = default;

    /**
     * Returns an acceleration vector preventing the ship from moving
     * far outside from the its space-box (i.e. the area where it should
     * be most of the time). The ship may escape its space-box for a while,
     * but the acceleration should stir it so that it returns back into the
     * space-box soon. If the ship is inside the space-box the function
     * typically returns zero vector.
     */
    virtual vector3  accelerate_into_space_box(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  space_box_center,           //!< Coordinates in meters.
            float_32_bit const  space_box_length_x,     //!< Length is in meters.
            float_32_bit const  space_box_length_y,     //!< Length is in meters.
            float_32_bit const  space_box_length_c      //!< Length is in meters.
            ) const = 0;

    /**
     * Returns an acceleration vector stearing the ship towards the passed dock.
     */
    virtual vector3  accelerate_into_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            float_32_bit const  dock_distance           //!< Distance between docks in meters.
            ) const = 0;

    /**
     * Returns an acceleration vector stearing the ship from another ship.
     */
    virtual vector3  accelerate_from_ship(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  other_ship_position,        //!< Coordinates in meters.
            vector3 const&  other_ship_velocity,        //!< In meters per second.
            vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
            float_32_bit const  dock_distance           //!< Distance between docks in meters.
            ) const = 0;
};


}

#endif
