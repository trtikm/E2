#ifndef NETLAB_NETWORK_OBJECTS_FACTORY_HPP_INCLUDED
#   define NETLAB_NETWORK_OBJECTS_FACTORY_HPP_INCLUDED

#   include <netlab/network_objects.hpp>
#   include <netlab/network_indices.hpp>
#   include <utility/array_of_derived.hpp>
#   include <memory>

namespace netlab {


struct network_objects_factory
{
    virtual ~network_objects_factory() {}

    virtual std::unique_ptr< array_of_derived<spiker> >  create_array_of_spikers(
            layer_index_type const  layer_index,
            natural_64_bit const  num_spikers
            ) const;

    virtual std::unique_ptr< array_of_derived<dock> >  create_array_of_docks(
            layer_index_type const  layer_index,
            natural_64_bit const  num_docks
            ) const;

    virtual std::unique_ptr< array_of_derived<ship> >  create_array_of_ships(
            layer_index_type const  layer_index,
            natural_64_bit const  num_ships
            ) const;
};


}

#endif
