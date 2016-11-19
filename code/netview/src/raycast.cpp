#include <netview/raycast.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace netview {


float_32_bit  find_first_spiker_on_line(
        std::vector<netlab::network_layer_props> const&  layer_props,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_spiker,
        netlab::compressed_layer_and_object_indices&  output_spiker_indices
        )
{
    TMPROF_BLOCK();

    return 1.0f;
}


float_32_bit  find_first_dock_on_line(
        std::vector<netlab::network_layer_props> const&  layer_props,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_dock,
        netlab::compressed_layer_and_object_indices&  output_dock_indices
        )
{
    TMPROF_BLOCK();

    return 1.0f;
}


float_32_bit  find_first_ship_on_line(
        netlab::network const&  network,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_ship,
        netlab::compressed_layer_and_object_indices&  output_ship_indices
        )
{
    TMPROF_BLOCK();

    return 1.0f;
}


}
