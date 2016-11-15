#ifndef NETLAB_SHIP_CONTROLLER_HPP_INCLUDED
#   define NETLAB_SHIP_CONTROLLER_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace netlab {
struct  network_props;
struct  network_layer_props;
}

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
     *
     * This default implementation applies a constant acceleration (of
     * magnitude proportional to the maximal speed of a ship in the layer)
     * to the ship towards the interrior of the space box if the ship is
     * not inside.
     */
    virtual vector3  accelerate_into_space_box(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  space_box_center,           //!< Coordinates in meters.
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const;

    /**
     * Returns an acceleration vector stearing the ship towards the passed dock.
     * This function is called when the spiker of the dock is NOT the same as the
     * spiker of the ship.
     */
    virtual vector3  accelerate_into_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const = 0;

    /**
     * Returns an acceleration vector stearing the ship away from the passed dock.
     * This function is called when the spiker of the dock is the same as the spiker
     * of the ship.
     *
     * The default implementation ignores the presence of the dock, as if there
     * was no dock at all.
     */
    virtual vector3  accelerate_from_dock(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  dock_position,              //!< Coordinates in meters.
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const
    { return vector3_zero(); }

    /**
     * Returns an acceleration vector stearing the ship from another ship. In other
     * words it must compute an acceleration preventing a collision of the ship with
     * the other one. This operation does not have to be commutative (if we swap
     * data of the ship and the other ship the resulting acceleration may be different).
     * Typically, the distance from the nearest dock to the ship is used as a weighting
     * (scaling) factor for the acceleration (e.g. closer to the dock smaler magnitude
     * of the acceleration).
     */
    virtual vector3  accelerate_from_ship(
            vector3 const&  ship_position,              //!< Coordinates in meters.
            vector3 const&  ship_velocity,              //!< In meters per second.
            vector3 const&  other_ship_position,        //!< Coordinates in meters.
            vector3 const&  other_ship_velocity,        //!< In meters per second.
            vector3 const&  nearest_dock_position,      //!< Coordinates in meters. It is nearest to the ship, not to the other one.
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const = 0;


    /**
      * Returns an acceleration vector of the ship induced by the environment the ship is moving in.
      *
      * The default implementation defines no effect of the environment to ship's movement.
      */
    virtual vector3  accelerate_ship_in_environment(
            vector3 const&  ship_velocity,              //!< In meters per second.
            network_layer_props const&  layer_props,
            network_props const&  props
            ) const
    { return vector3_zero(); }
};


}

#endif
