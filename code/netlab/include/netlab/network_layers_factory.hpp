#ifndef NETLAB_NETWORK_LAYER_ARRAYS_FACTORY_HPP_INCLUDED
#   define NETLAB_NETWORK_LAYER_ARRAYS_FACTORY_HPP_INCLUDED

#   include <netlab/network_layer_arrays_of_objects.hpp>
#   include <netlab/network_indices.hpp>
#   include <memory>

namespace netlab {


struct network_layers_factory
{
    virtual ~network_layers_factory() {}

    virtual std::unique_ptr<layer_of_spikers>  create_layer_of_spikers(
            layer_index_type const  layer_index,
            object_index_type const  num_spikers
            ) const
    { return std::make_unique<layer_of_spikers>(layer_index,num_spikers); }

    virtual std::unique_ptr<layer_of_docks>  create_layer_of_docks(
            layer_index_type const  layer_index,
            object_index_type const  num_docks
            ) const
    { return std::make_unique<layer_of_docks>(layer_index,num_docks); }

    virtual std::unique_ptr<layer_of_ships>  create_layer_of_ships(
            layer_index_type const  layer_index,
            object_index_type const  num_ships
            ) const
    { return std::make_unique<layer_of_ships>(layer_index,num_ships); }
};


}

#endif
