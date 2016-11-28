#include <netlab/network_objects.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <iostream>

namespace netlab {


spiker::spiker()
    : m_last_update_id(0UL)
{}

std::ostream&  spiker::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Last update id: " << m_last_update_id << "\n";
    return ostr;
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
