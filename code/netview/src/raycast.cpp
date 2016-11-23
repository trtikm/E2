#include <netview/raycast.hpp>
#include <netview/enumerate.hpp>
#include <angeo/collide.hpp>
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

    float_32_bit  param = 1.0f;
    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        netlab::compressed_layer_and_object_indices  indices(0U,0ULL);
        float_32_bit const  t =
                find_first_spiker_on_line(
                        layer_props.at(layer_index),
                        layer_index,
                        begin_point,
                        end_point,
                        radius_of_spiker,
                        indices
                        );
        if (t < param)
        {
            param = t;
            output_spiker_indices = indices;
        }
    }
    return param;
}


float_32_bit  find_first_spiker_on_line(
        netlab::network_layer_props const&  layer_props,
        netlab::layer_index_type const  layer_index,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_spiker,
        netlab::compressed_layer_and_object_indices&  output_spiker_indices
        )
{
    TMPROF_BLOCK();

    float_32_bit  param = 1.0f;
    enumerate_sectors_intersecting_line(
                begin_point,
                end_point,
                layer_props.low_corner_of_ships(),
                layer_props.high_corner_of_ships(),
                layer_props.distance_of_spikers_along_x_axis_in_meters(),
                layer_props.distance_of_spikers_along_y_axis_in_meters(),
                layer_props.distance_of_spikers_along_c_axis_in_meters(),
                std::bind(
                    [](netlab::network_layer_props const&  layer_props,
                       netlab::layer_index_type const  layer_index,
                       float_32_bit const  radius,
                       vector3 const&   line_begin,
                       vector3 const&   line_end,
                       float_32_bit&  param,
                       netlab::compressed_layer_and_object_indices&  indices,
                       netlab::sector_coordinate_type const  x,
                       netlab::sector_coordinate_type const  y,
                       netlab::sector_coordinate_type const  c
                       ) -> bool
                    {
                        INVARIANT(x < layer_props.num_spikers_along_x_axis() &&
                                  y < layer_props.num_spikers_along_y_axis() &&
                                  c < layer_props.num_spikers_along_c_axis() );
                        vector3 const  sector_centre = layer_props.spiker_sector_centre(x,y,c);
                        vector3  closest_point;
                        float_32_bit const  t =
                                angeo::closest_point_on_line_to_point(line_begin,line_end,sector_centre,&closest_point);
                        if (t < param && length_squared(sector_centre - closest_point) <= radius * radius)
                        {
                            param = t;
                            indices = netlab::compressed_layer_and_object_indices(
                                            layer_index,
                                            layer_props.spiker_sector_index(x,y,c)
                                            );
                            return false;
                        }
                        return true;
                    },
                    std::cref(layer_props),
                    layer_index,
                    radius_of_spiker,
                    std::cref(begin_point),
                    std::cref(end_point),
                    std::ref(param),
                    std::ref(output_spiker_indices),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    )
                );
    return param;
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

    float_32_bit  param = 1.0f;
    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        netlab::compressed_layer_and_object_indices  indices(0U,0ULL);
        float_32_bit const  t =
                find_first_dock_on_line(
                        layer_props.at(layer_index),
                        layer_index,
                        begin_point,
                        end_point,
                        radius_of_dock,
                        indices
                        );
        if (t < param)
        {
            param = t;
            output_dock_indices = indices;
        }
    }
    return param;
}


float_32_bit  find_first_dock_on_line(
        netlab::network_layer_props const&  layer_props,
        netlab::layer_index_type const  layer_index,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_dock,
        netlab::compressed_layer_and_object_indices&  output_dock_indices
        )
{
    TMPROF_BLOCK();

    float_32_bit  param = 1.0f;
    enumerate_sectors_intersecting_line(
                begin_point,
                end_point,
                layer_props.low_corner_of_ships(),
                layer_props.high_corner_of_ships(),
                layer_props.distance_of_docks_in_meters(),
                layer_props.distance_of_docks_in_meters(),
                layer_props.distance_of_docks_in_meters(),
                std::bind(
                    [](netlab::network_layer_props const&  layer_props,
                       netlab::layer_index_type const  layer_index,
                       float_32_bit const  radius,
                       vector3 const&   line_begin,
                       vector3 const&   line_end,
                       float_32_bit&  param,
                       netlab::compressed_layer_and_object_indices&  indices,
                       netlab::sector_coordinate_type const  x,
                       netlab::sector_coordinate_type const  y,
                       netlab::sector_coordinate_type const  c
                       ) -> bool
                    {
                        INVARIANT(x < layer_props.num_docks_along_x_axis() &&
                                  y < layer_props.num_docks_along_y_axis() &&
                                  c < layer_props.num_docks_along_c_axis() );
                        vector3 const  sector_centre = layer_props.dock_sector_centre(x,y,c);
                        vector3  closest_point;
                        float_32_bit const  t =
                                angeo::closest_point_on_line_to_point(line_begin,line_end,sector_centre,&closest_point);
                        if (t < param && length_squared(sector_centre - closest_point) <= radius * radius)
                        {
                            param = t;
                            indices = netlab::compressed_layer_and_object_indices(
                                            layer_index,
                                            layer_props.dock_sector_index(x,y,c)
                                            );
                            return false;
                        }
                        return true;
                    },
                    std::cref(layer_props),
                    layer_index,
                    radius_of_dock,
                    std::cref(begin_point),
                    std::cref(end_point),
                    std::ref(param),
                    std::ref(output_dock_indices),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    )
                );
    return param;
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

    auto const&  layer_props = network.properties()->layer_props();
    float_32_bit  param = 1.0f;
    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        netlab::compressed_layer_and_object_indices  indices(0U,0ULL);
        float_32_bit const  t =
                find_first_ship_on_line(
                        network,
                        layer_props.at(layer_index),
                        layer_index,
                        begin_point,
                        end_point,
                        radius_of_ship,
                        indices
                        );
        if (t < param)
        {
            param = t;
            output_ship_indices = indices;
        }
    }
    return param;
}


float_32_bit  find_first_ship_on_line(
        netlab::network const&  network,
        netlab::network_layer_props const&  layer_props,
        netlab::layer_index_type const  layer_index,
        vector3 const&   begin_point,
        vector3 const&   end_point,
        float_32_bit const  radius_of_ship,
        netlab::compressed_layer_and_object_indices&  output_ship_indices
        )
{
    TMPROF_BLOCK();

    float_32_bit  param = 1.0f;
    enumerate_sectors_intersecting_line(
                begin_point,
                end_point,
                layer_props.low_corner_of_ships(),
                layer_props.high_corner_of_ships(),
                layer_props.distance_of_docks_in_meters(),
                layer_props.distance_of_docks_in_meters(),
                layer_props.distance_of_docks_in_meters(),
                std::bind(
                    [](netlab::network const&  network,
                       netlab::network_layer_props const&  layer_props,
                       netlab::layer_index_type const  layer_index,
                       float_32_bit const  radius,
                       vector3 const&   line_begin,
                       vector3 const&   line_end,
                       float_32_bit&  param,
                       netlab::compressed_layer_and_object_indices&  indices,
                       netlab::sector_coordinate_type const  x,
                       netlab::sector_coordinate_type const  y,
                       netlab::sector_coordinate_type const  c
                       ) -> bool
                    {
                        INVARIANT(x < layer_props.num_docks_along_x_axis() &&
                                  y < layer_props.num_docks_along_y_axis() &&
                                  c < layer_props.num_docks_along_c_axis() );
                        vector3 const  sector_centre = layer_props.dock_sector_centre(x,y,c);
                        vector3 const  shift(0.5f * layer_props.distance_of_docks_in_meters(),
                                             0.5f * layer_props.distance_of_docks_in_meters(),
                                             0.5f * layer_props.distance_of_docks_in_meters());
                        if (param < 1.0f && !angeo::collision_point_and_bbox(line_begin + param * (line_end - line_begin),
                                                                             sector_centre - shift,sector_centre + shift))
                            return false;
                        netlab::object_index_type const  sector_index = layer_props.dock_sector_index(x,y,c);
                        for (auto const&  layer_and_object_indices :
                             network.get_indices_of_ships_in_dock_sector(layer_index,sector_index))
                        {
                            vector3 const&  ship_pos = network.get_ship(layer_and_object_indices.layer_index(),
                                                                        layer_and_object_indices.object_index()).position();
                            vector3  closest_point;
                            float_32_bit const  t =
                                    angeo::closest_point_on_line_to_point(line_begin,line_end,ship_pos,&closest_point);
                            if (t < param && length_squared(ship_pos - closest_point) <= radius * radius)
                            {
                                param = t;
                                indices = layer_and_object_indices;
                            }
                        }
                        return true;
                    },
                    std::cref(network),
                    std::cref(layer_props),
                    layer_index,
                    radius_of_ship,
                    std::cref(begin_point),
                    std::cref(end_point),
                    std::ref(param),
                    std::ref(output_ship_indices),
                    std::placeholders::_1,
                    std::placeholders::_2,
                    std::placeholders::_3
                    )
                );
    return param;
}


}
