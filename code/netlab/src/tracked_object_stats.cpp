#include <netlab/tracked_object_stats.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <iostream>

namespace netlab {


std::ostream&  tracked_network_object_stats::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Layer index: " << (natural_64_bit)indices().layer_index() << "\n"
         << shift << "Object index: " << (natural_64_bit)indices().object_index() << "\n"
         ;
    return ostr;
}


std::ostream&  tracked_spiker_stats::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Type: spiker\n";
    return tracked_network_object_stats::get_info_text(ostr,shift);
}


std::ostream&  tracked_dock_stats::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Type: dock\n";
    return tracked_network_object_stats::get_info_text(ostr,shift);
}


std::ostream&  tracked_ship_stats::get_info_text(std::ostream&  ostr, std::string const&  shift) const
{
    ostr << shift << "Type: ship\n";
    return tracked_network_object_stats::get_info_text(ostr,shift);
}


}
