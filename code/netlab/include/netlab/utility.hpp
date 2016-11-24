#ifndef NETLAB_UTILITY_HPP_INCLUDED
#   define NETLAB_UTILITY_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>

namespace netlab {


vector3  spiker_sector_centre(
        network_layer_props const&  props,
        object_index_type const  object_index
        );

inline vector3  spiker_sector_centre(
        std::vector<network_layer_props> const&  props,
        compressed_layer_and_object_indices const&  indices
        )
{ return spiker_sector_centre(props.at(indices.layer_index()),indices.object_index());}


inline vector3  shift_from_low_corner_of_spiker_sector_to_center(network_layer_props const&  props)
{
    return vector3{ 0.5f * props.distance_of_spikers_along_x_axis_in_meters(),
                    0.5f * props.distance_of_spikers_along_y_axis_in_meters(),
                    0.5f * props.distance_of_spikers_along_c_axis_in_meters() };
}

inline vector3  shift_from_low_corner_of_spiker_sector_to_center(
        std::vector<network_layer_props> const&  props,
        compressed_layer_and_object_indices const&  indices
        )
{ return shift_from_low_corner_of_spiker_sector_to_center(props.at(indices.layer_index())); }


vector3  dock_sector_centre(
        network_layer_props const&  props,
        object_index_type const  object_index
        );

inline vector3  dock_sector_centre(
        std::vector<network_layer_props> const&  props,
        compressed_layer_and_object_indices const&  indices
        )
{ return dock_sector_centre(props.at(indices.layer_index()),indices.object_index());}


inline vector3  shift_from_low_corner_of_dock_sector_to_center(network_layer_props const&  props)
{
    return vector3{ 0.5f * props.distance_of_docks_in_meters(),
                    0.5f * props.distance_of_docks_in_meters(),
                    0.5f * props.distance_of_docks_in_meters() };
}

inline vector3  shift_from_low_corner_of_dock_sector_to_center(
        std::vector<network_layer_props> const&  props,
        compressed_layer_and_object_indices const&  indices
        )
{ return shift_from_low_corner_of_dock_sector_to_center(props.at(indices.layer_index())); }



}

#endif
