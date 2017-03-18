#ifndef NETLAB_ACCESS_TO_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETLAB_ACCESS_TO_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/network_layer_arrays_of_objects.hpp>
#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/assumptions.hpp>
#   include <vector>

namespace netlab {


struct  access_to_movement_area_centers
{
    using layers_of_spikers = std::vector<std::unique_ptr<layer_of_spikers> >;

    access_to_movement_area_centers(layers_of_spikers* const  layers_of_spikers_ptr)
        : m_layers_of_spikers_ptr(layers_of_spikers_ptr)
    {
        ASSUMPTION(m_layers_of_spikers_ptr != nullptr);
    }

    vector3&  area_center(layer_index_type const  layer_index, object_index_type const  spiker_index)
    {
        return get_area_center(m_layers_of_spikers_ptr,layer_index,spiker_index);
    }

    vector3 const&  area_center(layer_index_type const  layer_index, object_index_type const  spiker_index) const
    {
        return get_area_center(m_layers_of_spikers_ptr, layer_index, spiker_index);
    }

private:
    static vector3&  get_area_center(
            layers_of_spikers* const  layers_of_spikers_ptr,
            layer_index_type const  layer_index,
            object_index_type const  spiker_index
            )
    {
        ASSUMPTION(layer_index < layers_of_spikers_ptr->size());
        ASSUMPTION(spiker_index < layers_of_spikers_ptr->at(layer_index)->size());
        return layers_of_spikers_ptr->at(layer_index)->get_movement_area_center(spiker_index);
    }

    layers_of_spikers*  m_layers_of_spikers_ptr;
};


}

#endif
