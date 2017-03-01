#include <netlab/network_objects.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <iostream>

namespace netlab {


spiker::spiker()
    : m_last_update_id(0ULL)
{}

std::ostream&  spiker::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Last update id: " << m_last_update_id << "\n";
    return ostr;
}

void  spiker::update_spiking_potential(
        natural_64_bit const  current_update_id,
        float_32_bit const  time_step_per_update_id_in_seconds,
        layer_index_type const  spiker_layer_index,
        network_props const&  props
        )
{
    for ( ; m_last_update_id < current_update_id; ++m_last_update_id)
        integrate_spiking_potential(time_step_per_update_id_in_seconds,spiker_layer_index,props);
}



ship::ship()
    : m_position(0.0f,0.0f,0.0f)
    , m_velocity(0.0f,0.0f,0.0f)
{}

std::ostream&  ship::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Position: [ " << m_position(0) << "m, " << m_position(1) << "m, " << m_position(2) << "m ]\n"
         << shift << "Velocity: [ " << m_velocity(0) << "m/s, " << m_velocity(1) << "m/s, " << m_velocity(2) << "m/s ]\n"
         ;
    return ostr;
}


}
