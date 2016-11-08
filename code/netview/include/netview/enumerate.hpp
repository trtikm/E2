#ifndef NETVIEW_ENUMERATE_HPP_INCLUDED
#   define NETVIEW_ENUMERATE_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <tuple>
#   include <functional>

namespace netview {


natural_64_bit  enumerate_spiker_positions(
        std::vector<netlab::network_layer_props> const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );

natural_64_bit  enumerate_spiker_positions(
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );


natural_64_bit  enumerate_dock_positions(
        std::vector<netlab::network_layer_props> const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );

natural_64_bit  enumerate_dock_positions(
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );


natural_64_bit  enumerate_ship_positions(
        netlab::network const&  network,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );

natural_64_bit  enumerate_ship_positions(
        netlab::network const&  network,
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        );

}

#endif
