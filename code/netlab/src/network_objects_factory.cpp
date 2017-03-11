#include <netlab/network_objects_factory.hpp>

namespace netlab {


std::unique_ptr< array_of_derived<spiker> >  network_objects_factory::create_array_of_spikers(
        layer_index_type const  layer_index,
        natural_64_bit const  num_spikers
        ) const
{
    (void)layer_index;
    return make_array_of_derived<spiker,spiker>(num_spikers);
}


std::unique_ptr< array_of_derived<dock> >  network_objects_factory::create_array_of_docks(
        layer_index_type const  layer_index,
        natural_64_bit const  num_docks
        ) const
{
    (void)layer_index;
    (void)num_docks;
    return make_array_of_derived<dock,dock>(1UL);
}

std::unique_ptr< array_of_derived<ship> >  network_objects_factory::create_array_of_ships(
        layer_index_type const  layer_index,
        natural_64_bit const  num_ships
        ) const
{
    (void)layer_index;
    return make_array_of_derived<ship,ship>(num_ships);
}


}
