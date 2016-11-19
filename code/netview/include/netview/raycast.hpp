#ifndef NETVIEW_RAYCAST_HPP_INCLUDED
#   define NETVIEW_RAYCAST_HPP_INCLUDED

#   include <netlab/network.hpp>
#   include <netlab/network_indices.hpp>
#   include <netlab/network_layer_props.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace netview {


float_32_bit  find_first_spiker_on_line(
        std::vector<netlab::network_layer_props> const&  layer_props,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_spiker,
        netlab::compressed_layer_and_object_indices&  output_spiker_indices
        );


float_32_bit  find_first_dock_on_line(
        std::vector<netlab::network_layer_props> const&  layer_props,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_dock,
        netlab::compressed_layer_and_object_indices&  output_dock_indices
        );


float_32_bit  find_first_ship_on_line(
        netlab::network const&  network,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_ship,
        netlab::compressed_layer_and_object_indices&  output_ship_indices
        );


}

#endif
