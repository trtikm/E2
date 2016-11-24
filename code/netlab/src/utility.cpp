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


}
