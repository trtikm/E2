#ifndef NETLAB_SHIP_CONTROLLER_HPP_INCLUDED
#   define NETLAB_SHIP_CONTROLLER_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>

namespace netlab {
struct  network_props;
struct  network_layer_props;
}

namespace netlab {


struct  ship_controller
{
    /**
     * The constructor allows to specify two distances from the processed ship which
     * will tell the network when to call methods bellow for docks and ships. Note that
     * it does not mean that the network will never call those methods for docks and
     * ships beyond those distances. Actually, it commonly happens. The purpose
     * of the two distances is to allow the network to quickly skip all those docks
     * and ships which are obviously too far (e.g. in far space secktors) so that
     * they have to necessarily be beyond those limits.
     */
    ship_controller(
            float_32_bit const  docks_enumerations_distance_for_accelerate_into_dock,
                //!< This distance is used by the network to call the method
                //!< 'accelerate_into_dock' or 'accelerate_from_dock' for all
                //!< dock whose distance to the ship is not greater than the
                //!< passed distance.
                //!< This value cannot be negative.
            float_32_bit const  docks_enumerations_distance_for_accelerate_from_ship
                //!< This distance is used by the network to call the method
                //!< 'accelerate_from_ship' for all ships in all docks whose
                //!< distance to the ship is not greater than the passed distance.
                //!< This value cannot be negative.
            );


    virtual ~ship_controller() {}


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
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
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
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
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
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
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
            bool const  both_ship_and_dock_belongs_to_same_spiker,
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props
            ) const = 0;


    /**
      * Returns an acceleration vector of the ship induced by the environment the ship is moving in.
      *
      * The default implementation defines no effect of the environment to ship's movement.
      */
    virtual vector3  accelerate_ship_in_environment(
            vector3 const&  ship_velocity,              //!< In meters per second.
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props
            ) const
    { return vector3_zero(); }


    /**
      * This method is called when the speed of the ship becomes lower the minimal allowed speed.
      * The method can update the velocity vector of the ship. It is also fine to leave the vector
      * unchanged, because both minimal and maximal speeds for a ship are more related to sending
      * these notifications then strict rules to be preserved.
      *
      * The default implementation leaves the velocity vector of the ship unchanged, if the ship
      * is within a connection radius of some dock. Otherwise a random velocity vector (using the
      * default random generator) of magnitude in the middile between minimal and maximal speed
      * is computed.
      */
    virtual  void  on_too_slow(
            vector3&  ship_velocity,                    //!< In meters per second.
            vector3 const&  ship_position,              //!< Coordinates in meters.
            float_32_bit const  speed,                  //!< In meters per second. Equals to lenght of 'ship_velocity'.
            vector3 const&  nearest_dock_position,
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props
            ) const;


    /**
      * This method is called when the speed of the ship becomes above the maximal allowed speed.
      * The method can update the velocity vector of the ship. It is also fine to leave the vector
      * unchanged, because both minimal and maximal speeds for a ship are more related to sending
      * these notifications than strict rules to be preserved.
      *
      * The default implementation lowers the velocity of the ship to the maximal one.
      */
    virtual  void  on_too_fast(
            vector3&  ship_velocity,                    //!< In meters per second.
            vector3 const&  ship_position,              //!< Coordinates in meters.
            float_32_bit const  speed,                  //!< In meters per second. Equals to lenght of 'ship_velocity'.
            vector3 const&  nearest_dock_position,
            layer_index_type const  home_layer_index,   //!< Index of layer where is the spiker the ship belongs to.
            layer_index_type const  area_layer_index,   //!< Index of layer where is the movement area in which the ship moves.
            network_props const&  props
            ) const;


    float_32_bit  docks_enumerations_distance_for_accelerate_into_dock() const noexcept
    { return m_docks_enumerations_distance_for_accelerate_into_dock; }

    float_32_bit  docks_enumerations_distance_for_accelerate_from_ship() const noexcept
    { return m_docks_enumerations_distance_for_accelerate_from_ship; }

private:
    float_32_bit  m_docks_enumerations_distance_for_accelerate_into_dock;
    float_32_bit  m_docks_enumerations_distance_for_accelerate_from_ship;
};


}

#endif
