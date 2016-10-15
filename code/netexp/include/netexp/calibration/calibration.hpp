#ifndef NETEXP_CALIBRATION_CALIBRATION_HPP_INCLUDED
#   define NETEXP_CALIBRATION_CALIBRATION_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <netlab/tracked_object_stats.hpp>
#   include <memory>

namespace netexp { namespace calibration {


struct  tracked_spiker_stats : public netlab::tracked_spiker_stats
{
};


struct  tracked_dock_stats : public netlab::tracked_dock_stats
{
};


struct  tracked_ship_stats : public netlab::tracked_ship_stats
{
};


std::shared_ptr<netlab::network>  create_network();

std::shared_ptr<netlab::tracked_spiker_stats>  create_tracked_spiker_stats();
std::shared_ptr<netlab::tracked_dock_stats>  create_tracked_dock_stats();
std::shared_ptr<netlab::tracked_ship_stats>  create_tracked_ship_stats();


}}

#endif
