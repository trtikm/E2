#ifndef NETLAB_NETWORK_OBJECTS_HPP_INCLUDED
#   define NETLAB_NETWORK_OBJECTS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <netlab/network_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <string>
#   include <iosfwd>

namespace netlab {


/**
 * It models a neuron.
 */
struct  spiker
{
    spiker();
    virtual ~spiker() {}

    natural_64_bit  last_update_id() const noexcept { return m_last_update_id; }
    //void  set_last_update_id(natural_64_bit const  value) { m_last_update_id = value; }

    /**
     * The function is automatically called by the network in order to update the spiking potential
     * function of the spiker to the current time of the network. This is implemented by calling the method
     * @integrate_spiking_potential until the member @m_last_update_id does not match the current
     * update id of the network, passed to the function as the parameter @current_update_id.
     *
     * @param current_update_id  The id of the current update step of the network.
     */
    void  update_spiking_potential(
            natural_64_bit const  current_update_id,
            layer_index_type const  spiker_layer_index,
            network_props const&  props
            );

    virtual natural_64_bit  size_in_bytes() const { return sizeof(spiker); }

    virtual float_32_bit  get_potential() const { return 0.0f; }

    /**
     * It defines an integrator of the potential function of the spiker. It is supposed to integrate the
     * function from the current potential of the spiker for the passed time interval.
     *
     * @param time_delta_in_seconds  It is a time interval the spiking potential function of the spiker
     *                               should be integrated for.
     */
    virtual void  integrate_spiking_potential(
            float_32_bit const  time_delta_in_seconds,
            layer_index_type const  spiker_layer_index,
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
            float_32_bit const  potential_delta,
            layer_index_type const  spiker_layer_index,
            network_props const&  props
            )
    { return false; }

    virtual std::ostream&  get_info_text(std::ostream&  ostr, std::string const&  shift = "") const;

private:
    /**
     * The network is updated in discrete steps. Each step has its unique id.
     * This member saves id of the network update in which the spiking potential
     * function function of the spiker was integrated for the last (i.e. the last update
     * id where the network called the method @integrate_spiking_potential).
     */
    natural_64_bit  m_last_update_id;
};


/**
 * It models a synaptic bouton on a dendrite of the owner spiker. It thus models a place on the dendrite
 * where a ship may connect to, and to allow the ship there to transmit a spike from the spiker of the ship
 * to the spiker of the dock.
 */
struct  dock
{
    virtual ~dock() {}

    virtual natural_64_bit  size_in_bytes() const { return sizeof(dock); }
    virtual std::ostream&  get_info_text(std::ostream&  ostr, std::string const&  shift = "") const { return ostr; }

    /**
     * It is called by the network whenewer a ship connected to this dock (if there is any) releases the
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
            float_32_bit const  potential_delta,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            layer_index_type const  spiker_layer_index,
            layer_index_type const  layer_index_of_spiker_owning_the_connected_ship,
            network_props const&  props
            )
    { return potential_delta; }

    /**
     * It updates the connetion weight of the dock with the ship it is connected to. Then it computes
     * a change of potential transferable to the connected ship through the synaptic claft. This potential
     * change is returned.
     *
     * @return A change of potential transferable to the connected ship through the synaptic claft. The
     *         change is always non negative, no matter whether the spiker is excitatoty or inhibitory.
     */
    virtual float_32_bit  on_arrival_of_presynaptic_potential(
            float_32_bit const  potential_of_the_spiker_owning_the_connected_ship,
            layer_index_type const  spiker_layer_index,
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
            bool const  is_it_mini_spike_from_excitatory_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            layer_index_type const  spiker_layer_index,
            network_props const&  props
            ) const
    { return props.mini_spiking_potential_magnitude() * (is_it_mini_spike_from_excitatory_spiker ? 1.0f : -1.0f); }

    /**
     * It computes (and returns) a spiker's potential at the position of this dock from the passed spiker's
     * potential at its soma (center).
     * @param potential_of_spiker  Spiker's potential at the soma, i.e. the current potential stored in the spiker.
     */
    virtual float_32_bit  compute_potential_of_spiker_at_dock(
            float_32_bit const  potential_of_spiker,
            vector3 const&  spiker_position,
            vector3 const&  dock_position,
            layer_index_type const  spiker_layer_index,
            network_props const&  props
            ) const
    { return potential_of_spiker; }
};


/**
 * It model an axon terminal of its spiker's axon. It thus models a search of the axon terminal
 * for dendridic boutons (docks) on dendrites of other spikers where the ship can connect to
 * and create a synapse with the other spiker. It is also supposed to model the behaviour of the
 * synapse (i.e. when the ship is connected with a dock).
 */
struct  ship
{
    ship();
    virtual ~ship() {}

    vector3 const&  position() const noexcept { return m_position; }
    void  set_position(vector3 const&  pos) { m_position = pos; }
    vector3&  get_position_nonconst_reference() noexcept { return m_position; }

    vector3 const&  velocity() const noexcept { return m_velocity; }
    void  set_velocity(vector3 const&  v) { m_velocity = v; }
    vector3&  get_velocity_nonconst_reference() noexcept { return m_velocity; }

    virtual natural_64_bit  size_in_bytes() const { return sizeof(ship); }
    virtual std::ostream&  get_info_text(std::ostream&  ostr, std::string const&  shift = "") const;

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
            float_32_bit const  potential_of_the_other_spiker_at_connected_dock,
            layer_index_type const  spiker_layer_index,
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
            float_32_bit const  potential_of_the_spiker_owning_the_connected_dock,
            layer_index_type const  spiker_layer_index,
            layer_index_type const  area_layer_index,
            network_props const&  props
            )
    {}

private:
    vector3  m_position;
    vector3  m_velocity;
};


}

#endif
