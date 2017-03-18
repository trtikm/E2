#ifndef NETLAB_NETWORK_LAYER_ARRAYS_OF_OBJECTS_HPP_INCLUDED
#   define NETLAB_NETWORK_LAYER_ARRAYS_OF_OBJECTS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <netlab/network_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <string>
#   include <iosfwd>

namespace netlab {


/**
 * It models spikers (neurons) of a single kind appearing in a certain layer of the neural tissue.
 * The layer is implementedas 1D array. Each spiker is thus associated with a unique index in the array.
 */
struct  layer_of_spikers
{
    layer_of_spikers(layer_index_type const  layer_index, object_index_type const  num_spikers_in_the_layer);
    virtual ~layer_of_spikers() {}

    object_index_type size() const { return m_last_update_ids.size(); }

    virtual natural_64_bit  num_bytes_per_spiker() const { return 0UL; }
    static natural_64_bit  num_extra_bytes_per_spiker() { return sizeof(natural_64_bit) + sizeof(vector3); }

    layer_index_type  layer_index() const { return m_layer_index; }

    natural_64_bit  last_update_id(object_index_type const  spiker_index) const { return m_last_update_ids.at(spiker_index); }

    const vector3 &get_movement_area_center(object_index_type const  spiker_index) const
    { return m_movement_area_center.at(spiker_index); }

    vector3 &get_movement_area_center(object_index_type const  spiker_index)
    { return m_movement_area_center.at(spiker_index); }

    /**
     * The function is automatically called by the network in order to update the spiking potential
     * function of the spiker to the current time of the network. This is implemented by calling the method
     * @integrate_spiking_potential until the member @m_last_update_id does not match the current
     * update id of the network, passed to the function as the parameter @current_update_id.
     *
     * @param current_update_id  The id of the current update step of the network.
     */
    void  update_spiking_potential(
            object_index_type const  spiker_index,
            natural_64_bit const  current_update_id,
            network_props const&  props
            );


    virtual float_32_bit  get_potential(object_index_type const  spiker_index) const { return 0.0f; }

    /**
     * It defines an integrator of the potential function of the spiker. It is supposed to integrate the
     * function from the current potential of the spiker for the passed time interval.
     *
     * @param time_delta_in_seconds  It is a time interval the spiking potential function of the spiker
     *                               should be integrated for.
     */
    virtual void  integrate_spiking_potential(
            object_index_type const  spiker_index,
            float_32_bit const  time_delta_in_seconds,
            network_props const&  props
            )
    {}

    /**
     * It performs an update of the potential function of the spiker according to an instant
     * change of the potential (impulse) arrived from some dock of the spiker.
     *
     * @param potential_delta  An instant change of the potential (impulse) arrived from some dock
     *                         of the spiker. Note that the argument has the opposite sign for the potential
     *                         change from excitatory and inhibitory spikers.
     * @return If the integration of spiker's potential function from the updated potential (by the passed 
     *         potential change) would necessarily lead to genetation of a spike (without arrival of any
     *         further postsynaptic potential), then the function returns true. Otherwise it returns false.
     */
    virtual bool  on_arrival_of_postsynaptic_potential(
            object_index_type const  spiker_index,
            float_32_bit const  potential_delta,
            network_props const&  props
            )
    { return false; }

    virtual std::ostream&  get_info_text(
            object_index_type const  spiker_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const;

private:
    layer_of_spikers(layer_of_spikers const&) = delete;
    layer_of_spikers& operator=(layer_of_spikers const&) = delete;

    layer_index_type  m_layer_index;

    /**
     * The network is updated in discrete steps. Each step has its unique id.
     * This member saves for each spiker the id of the network update in which the spiking potential
     * function function of the spiker was integrated for the last (i.e. the last update
     * id where the network called the method @integrate_spiking_potential).
     */
    std::vector<natural_64_bit>  m_last_update_ids;

    /**
     * Each spiker is associated with a space region in the neural tissue where ships (axon terminals)
     * of the spiker can make connections to docks (synaptic boutons) of other spikers. This region is always
     * comprised in some layer of the tissue, it further always has a box shape with the center stored in this
     * vector. The size of the box is the same for all spikers in this layer (it is stored in "network_layer_props"
     * of this layer). 
     */
    std::vector<vector3>  m_movement_area_center;
};


/**
 * It models docks (synaptic boutons) on dendrites of spikers in one layer of the neural tissue. (Note: a docks is thus a
 * place on a dendrite where ships (axon terminals) of other spikers may connect to). This layer of docks is implemented
 * as 1D array. Each dock (bouton) is thus associated with a unique index in the array. Docks are grouped according to their
 * spikers. It means that first are stored all docks of the first spiker, then all docks of the second one, and so on.
 * Each spiker in the layer has exactly the same count of docks. Also all docks of all spikers in the layer are of the
 * same kind.
 */
struct  layer_of_docks
{
    layer_of_docks(layer_index_type const  layer_index, object_index_type const  num_docks_in_the_layer);
    virtual ~layer_of_docks() {}

    object_index_type size() const { return m_num_docks_in_the_layer; }

    virtual natural_64_bit  num_bytes_per_dock() const { return 0UL; }
    static natural_64_bit  num_extra_bytes_per_dock() { return 0UL; }

    layer_index_type  layer_index() const { return m_layer_index; }

    virtual std::ostream&  get_info_text(
            object_index_type const  dock_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const
    { return ostr; }

    /**
     * It is called by the network whenewer a ship connected to this dock releases the
     * neuro-transmitter into the synaptic cleft between this dock and the ship. The function is supposed
     * to update its connection weight with the connected ship and to compute (return) a change of potential
     * which will arrive from the dock to the spiker (owning the dock).
     *
     * @param potential_delta   It is the change of potential caused by release of the neuro-transmitter
     *                          from the connected ship into the synaptic cleft between this dock and the ship.
     *                          It is a value returned from the function "ship::on_arrival_of_presynaptic_potential"
     *                          invoked on the ship connected to this dock. Note that the argument has the opposite
     *                          sign for a potential change from excitatory and inhibitory spikers.
     * @return It is supposed to compute (return) a potential change which would arrive from this dock to the soma
     *         of the spiker owning this dock.
     */
    virtual float_32_bit  on_arrival_of_postsynaptic_potential(
            object_index_type const  dock_index,
            float_32_bit const  potential_delta,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            layer_index_type const  layer_index_of_spiker_owning_the_connected_ship,
            network_props const&  props
            )
    { return potential_delta; }

    /**
     * First it computes what ammout of the passed potential of the spiker (owning this dock) arrives
     * from the soma of the spiker to this dock. Then it uses the computed potential for an update of
     * the connetion weight of the dock with the connected ship. Next, based on the computed potential
     * it computes a change of potential transferable to the connected ship through the synaptic claft.
     * This potential change is returned.
     *
     * @return A change of potential transferable to the connected ship through the synaptic claft. The
     *         change is always non negative, no matter whether the spiker is excitatoty or inhibitory.
     */
    virtual float_32_bit  on_arrival_of_presynaptic_potential(
            object_index_type const  dock_index,
            float_32_bit const  potential_of_the_spiker_owning_the_connected_ship,
            layer_index_type const  layer_index_of_spiker_owning_the_connected_ship,
            network_props const&  props
            )
    { return props.spiking_potential_magnitude(); }

    /**
     * The function computes (returns) what portion of the mini spiking potential will arrive from this dock to
     * the spiker (owning the dock).
     *
     * @return It returns that portion of "network_props::mini_spiking_potential_magnitude()" which would arrive
     *         from this dock to the soma of the spiker owning this dock. The return value has the opposite
     *         sign depending on whether the mini spike comes from ship of excitatory or inhibitory spiker (see
     *         the value of the parameter "is_it_mini_spike_from_excitatory_spiker").
     */
    virtual float_32_bit  on_arrival_of_mini_spiking_potential(
            object_index_type const  dock_index,
            bool const  is_it_mini_spike_from_excitatory_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            network_props const&  props
            ) const
    { return props.mini_spiking_potential_magnitude() * (is_it_mini_spike_from_excitatory_spiker ? 1.0f : -1.0f); }

    /**
     * It computes (and returns) a spiker's potential at the position of this dock from the passed spiker's
     * potential at its soma (center).
     * @param potential_of_spiker  Spiker's potential at the soma, i.e. the current potential stored in the spiker.
     */
    virtual float_32_bit  compute_potential_of_spiker_at_dock(
            object_index_type const  dock_index,
            float_32_bit const  potential_of_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            network_props const&  props
            ) const
    { return potential_of_spiker; }

private:
    layer_of_docks(layer_of_docks const&) = delete;
    layer_of_docks& operator=(layer_of_docks const&) = delete;

    layer_index_type  m_layer_index;
    object_index_type  m_num_docks_in_the_layer;
};


/**
 * It models ships (axon terminals) of spikers in one layer of the neural tissue. (Note: a ship is thus a 
 * "connector" to a dock (synaptic bouton) on a dendrite of another spiker. This layer of ships is implemented
 * as 1D array. Each ship is thus associated with a unique index in the array. Ships are grouped according to their
 * spikers. It means that first are stored all ships of the first spiker, then all ships of the second one, and so on.
 * Each spiker in the layer has exactly the same count of ships. Also all ships of all spikers in the layer are of the
 * same kind.
 */
struct  layer_of_ships
{
    layer_of_ships(layer_index_type const  layer_index, object_index_type const  num_ships_in_the_layer);
    virtual ~layer_of_ships() {}

    object_index_type size() const { return m_positions.size(); }

    virtual natural_64_bit  num_bytes_per_ship() const { return 0UL; }
    static natural_64_bit  num_extra_bytes_per_ship() { return 2ULL * sizeof(vector3); }

    layer_index_type  layer_index() const { return m_layer_index; }

    vector3 const&  position(object_index_type const  ship_index) const { return m_positions.at(ship_index); }
    void  set_position(object_index_type const  ship_index, vector3 const&  pos) { m_positions.at(ship_index) = pos; }
    vector3&  get_position_nonconst_reference(object_index_type const  ship_index) { return m_positions.at(ship_index); }

    vector3 const&  velocity(object_index_type const  ship_index) const { return m_velocities.at(ship_index); }
    void  set_velocity(object_index_type const  ship_index, vector3 const&  v) { m_velocities.at(ship_index) = v; }
    vector3&  get_velocity_nonconst_reference(object_index_type const  ship_index) { return m_velocities.at(ship_index); }

    virtual std::ostream&  get_info_text(
            object_index_type const  ship_index,
            std::ostream&  ostr,
            std::string const&  shift = ""
            ) const;

    /**
     * It updates the connetion weight of the ship with the dock it is connected to. Then it computes
     * a change of potential on the connected dock caused by release of the neuro transmitter
     * into the synaptic claft. This potential change is returned.
     *
     * @param potential_of_the_other_spiker_at_connected_dock  It is value computed by the function
     *              "dock::compute_potential_of_spiker_at_dock" called on the dock this ship
     *              is connected to.
     * @return A change of potential on the connected dock caused by the release of the neuro transmitter
     *         into the synaptic claft.
     */
    virtual float_32_bit  on_arrival_of_presynaptic_potential(
            object_index_type const  ship_index,
            float_32_bit const  potential_of_the_other_spiker_at_connected_dock,
            layer_index_type const  area_layer_index,
            network_props const&  props
            )
    { return props.spiking_potential_magnitude(); }

    /**
     * It updates the connetion weight of the ship with the dock it is connected to.
     *
     * @param potential_of_the_spiker_owning_the_connected_dock  It is a value computed by the function
     *              "dock::on_arrival_of_presynaptic_potential" called on the dock this ship
     *              is connected to.
     */
    virtual void  on_arrival_of_postsynaptic_potential(
            object_index_type const  ship_index,
            float_32_bit const  potential_of_the_spiker_owning_the_connected_dock,
            layer_index_type const  area_layer_index,
            network_props const&  props
            )
    {}

private:
    layer_of_ships(layer_of_ships const&) = delete;
    layer_of_ships& operator=(layer_of_ships const&) = delete;

    layer_index_type  m_layer_index;
    std::vector<vector3>  m_positions;
    std::vector<vector3>  m_velocities;
};


}

#endif
