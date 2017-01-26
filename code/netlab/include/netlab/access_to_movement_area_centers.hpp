#ifndef NETLAB_ACCESS_TO_MOVEMENT_AREA_CENTERS_HPP_INCLUDED
#   define NETLAB_ACCESS_TO_MOVEMENT_AREA_CENTERS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/assumptions.hpp>
#   include <vector>

namespace netlab {


struct  access_to_movement_area_centers
{
    access_to_movement_area_centers(std::vector< std::vector<vector3> >* const  movement_area_centers_ptr)
        : m_movement_area_centers_ptr(movement_area_centers_ptr)
    {
        ASSUMPTION(m_movement_area_centers_ptr != nullptr);
    }

    vector3&  area_center(layer_index_type const  layer_index, object_index_type const  spiker_index)
    {
        return get_area_center(m_movement_area_centers_ptr,layer_index,spiker_index);
    }

    vector3 const&  area_center(layer_index_type const  layer_index, object_index_type const  spiker_index) const
    {
        return get_area_center(m_movement_area_centers_ptr, layer_index, spiker_index);
    }

private:
    static vector3&  get_area_center(
            std::vector< std::vector<vector3> >* const  movement_area_centers_ptr,
            layer_index_type const  layer_index,
            object_index_type const  spiker_index
            )
    {
        ASSUMPTION(layer_index < movement_area_centers_ptr->size());
        ASSUMPTION(spiker_index < movement_area_centers_ptr->at(layer_index).size());
        return movement_area_centers_ptr->at(layer_index).at(spiker_index);
    }

    std::vector< std::vector<vector3> >*  m_movement_area_centers_ptr;
};


}

#endif
