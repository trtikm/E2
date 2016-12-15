#include <netlab/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


vector3  spiker_sector_centre(
        network_layer_props const&  props,
        object_index_type const  object_index
        )
{
    netlab::sector_coordinate_type  x,y,c;
    props.spiker_sector_coordinates(object_index,x,y,c);
    return props.spiker_sector_centre(x,y,c);
}


vector3  dock_sector_centre(
        network_layer_props const&  props,
        object_index_type const  object_index
        )
{
    netlab::sector_coordinate_type  x,y,c;
    props.dock_sector_coordinates(object_index,x,y,c);
    return props.dock_sector_centre(x,y,c);
}


std::vector<network_layer_props>  make_layer_props(
    std::vector< std::array<natural_32_bit, 3ULL> > const&  num_spikers,
    std::vector< std::array<natural_32_bit, 3ULL> > const&  num_docks_per_spiker,
    std::vector<natural_32_bit> const&  num_ships_per_spiker,
    std::vector<float_32_bit> const&  distance_of_docks,
    std::vector<vector3> const&  low_corner_of_docks,
    std::vector<bool> const&  are_spikers_excitatory,
    std::vector< std::shared_ptr<netlab::ship_controller const> > const&  ship_controllers,
    std::vector< std::vector<vector2> > const&  speed_limits,
    std::vector< std::vector<vector3> > const&  size_of_movement_area
    )
{
    ASSUMPTION(num_spikers.size() != 0U && num_spikers.size() <= max_number_of_layers());
    layer_index_type const  num_layers = static_cast<layer_index_type>(num_spikers.size());

    std::vector<network_layer_props>  output;
    output.reserve(num_layers);
    for (layer_index_type layer_index = 0U; layer_index != num_layers; ++layer_index)
        output.push_back({
                num_spikers.at(layer_index).at(0U),
                num_spikers.at(layer_index).at(1U),
                num_spikers.at(layer_index).at(2U),

                num_docks_per_spiker.at(layer_index).at(0U),
                num_docks_per_spiker.at(layer_index).at(1U),
                num_docks_per_spiker.at(layer_index).at(2U),

                num_ships_per_spiker.at(layer_index),

                distance_of_docks.at(layer_index),

                low_corner_of_docks.at(layer_index),

                size_of_movement_area.at(layer_index),

                speed_limits.at(layer_index),

                are_spikers_excitatory.at(layer_index),

                ship_controllers.at(layer_index)
                });
    return output;
}


}
