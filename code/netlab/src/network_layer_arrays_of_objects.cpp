#include <netlab/network_layer_arrays_of_objects.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <iostream>
#include <iomanip>

namespace netlab {


layer_of_spikers::layer_of_spikers(layer_index_type const  layer_index, object_index_type const  num_spikers_in_the_layer)
    : m_layer_index(layer_index)
    , m_last_update_ids(num_spikers_in_the_layer,0ULL)
{
    ASSUMPTION(!m_last_update_ids.empty());
}

void  layer_of_spikers::update_spiking_potential(
        object_index_type const  spiker_index,
        natural_64_bit const  current_update_id,
        network_props const&  props
        )
{
    natural_64_bit& last_update_id = m_last_update_ids.at(spiker_index);
    for ( ; last_update_id < current_update_id; ++last_update_id)
        integrate_spiking_potential(spiker_index,props.update_time_step_in_seconds(),props);
}

std::ostream&  layer_of_spikers::get_info_text(
        object_index_type const  spiker_index,
        std::ostream&  ostr,
        std::string const&  shift
        ) const
{
    ostr << shift << "Last update id: " << last_update_id(spiker_index) << "\n";
    return ostr;
}


layer_of_docks::layer_of_docks(layer_index_type const  layer_index, object_index_type const  num_docks_in_the_layer)
    : m_layer_index(layer_index)
    , m_num_docks_in_the_layer(num_docks_in_the_layer)
{
    ASSUMPTION(m_num_docks_in_the_layer != 0ULL);
}


layer_of_ships::layer_of_ships(layer_index_type const  layer_index, object_index_type const  num_ships_in_the_layer)
    : m_layer_index(layer_index)
    , m_positions(num_ships_in_the_layer,{0.0f,0.0f,0.0f})
    , m_velocities(num_ships_in_the_layer,{0.0f,0.0f,0.0f})
{
    ASSUMPTION(!m_positions.empty());
}

std::ostream&  layer_of_ships::get_info_text(
        object_index_type const  ship_index,
        std::ostream&  ostr,
        std::string const&  shift
        ) const
{
    vector3 const&  p = position(ship_index);
    vector3 const&  v = velocity(ship_index);
    ostr << std::fixed << std::setprecision(3)
         << shift << "Position: [ " << p(0) << "m, " << p(1) << "m, " << p(2) << "m ]\n"
         << shift << "Velocity: [ " << v(0) << "m/s, " << v(1) << "m/s, " << v(2) << "m/s ]\n"
         ;
    return ostr;
}


}
