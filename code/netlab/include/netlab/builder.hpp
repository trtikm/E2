#ifndef NETLAB_BUILDER_HPP_INCLUDED
#   define NETLAB_BUILDER_HPP_INCLUDED

#   include <netlab/uid.hpp>
#   include <netlab/sockets.hpp>
#   include <netlab/unit.hpp>
#   include <netlab/layer.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>

namespace netlab {


struct  network;


struct  builder
{
    struct  layer_info
    {
        natural_16_bit  num_units;
        natural_16_bit  num_sockets_per_unit;
        network_layer  layer;
    };

    builder(network* const  net_);
    builder&  insert_layer_info(layer_info const&  info);
    void  run();

private:

    void  setup_minimal_net();

    network*  net;
    std::vector<layer_info>  layers;
};


}


#endif
