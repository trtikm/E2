#include <netlab/network_layer_props.hpp>
#include <utility/checked_number_operations.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


network_layer_props::network_layer_props(
        natural_32_bit const  num_spikers_along_x_axis,
        natural_32_bit const  num_spikers_along_y_axis,
        natural_32_bit const  num_spikers_along_c_axis,

        natural_32_bit const  num_docks_along_x_axis_per_spiker,
        natural_32_bit const  num_docks_along_y_axis_per_spiker,
        natural_32_bit const  num_docks_along_c_axis_per_spiker,

        natural_32_bit const  num_ships_per_spiker,

        float_32_bit const  distance_of_docks_in_meters,

        vector3 const&  low_corner_of_docks,

        std::vector<vector3> const&  size_of_ship_movement_area_in_meters,

        std::vector<vector2> const&  speed_limits_of_ship_in_meters_per_second,

        bool const  are_spikers_excitatory,

        std::shared_ptr<ship_controller const> const  ship_controller_ptr
        )
    : m_num_spikers_along_x_axis(num_spikers_along_x_axis)
    , m_num_spikers_along_y_axis(num_spikers_along_y_axis)
    , m_num_spikers_along_c_axis(num_spikers_along_c_axis)

    , m_num_spikers_in_xy_plane( checked_mul_64_bit(m_num_spikers_along_x_axis, m_num_spikers_along_y_axis) )
    , m_num_spikers( checked_mul_64_bit(m_num_spikers_along_c_axis,m_num_spikers_in_xy_plane) )

    , m_num_docks_along_x_axis_per_spiker(num_docks_along_x_axis_per_spiker)
    , m_num_docks_along_y_axis_per_spiker(num_docks_along_y_axis_per_spiker)
    , m_num_docks_along_c_axis_per_spiker(num_docks_along_c_axis_per_spiker)

    , m_num_docks_along_x_axis(checked_mul_32_bit(m_num_spikers_along_x_axis,m_num_docks_along_x_axis_per_spiker))
    , m_num_docks_along_y_axis(checked_mul_32_bit(m_num_spikers_along_y_axis,m_num_docks_along_y_axis_per_spiker))
    , m_num_docks_along_c_axis(checked_mul_32_bit(m_num_spikers_along_c_axis,m_num_docks_along_c_axis_per_spiker))

    , m_num_docks_in_xy_plane( checked_mul_64_bit(m_num_docks_along_x_axis, m_num_docks_along_y_axis) )
    , m_num_docks( checked_mul_64_bit(m_num_docks_along_c_axis,m_num_docks_in_xy_plane) )

    , m_num_ships_per_spiker(num_ships_per_spiker)

    , m_num_ships(checked_mul_64_bit(m_num_ships_per_spiker, m_num_spikers))

    , m_distance_of_docks_in_meters(distance_of_docks_in_meters)

    , m_distance_of_spikers_along_x_axis_in_meters(
          static_cast<float_32_bit>(m_num_docks_along_x_axis_per_spiker) * m_distance_of_docks_in_meters
          )
    , m_distance_of_spikers_along_y_axis_in_meters(
          static_cast<float_32_bit>(m_num_docks_along_y_axis_per_spiker) * m_distance_of_docks_in_meters
          )
    , m_distance_of_spikers_along_c_axis_in_meters(
          static_cast<float_32_bit>(m_num_docks_along_c_axis_per_spiker) * m_distance_of_docks_in_meters
          )

    , m_low_corner_of_docks(low_corner_of_docks)
    , m_high_corner_of_docks(
          m_low_corner_of_docks + m_distance_of_docks_in_meters *
          vector3(static_cast<float_32_bit>(m_num_docks_along_x_axis - 1U),
                  static_cast<float_32_bit>(m_num_docks_along_y_axis - 1U),
                  static_cast<float_32_bit>(m_num_docks_along_c_axis - 1U))
          )

    , m_low_corner_of_spikers(
          m_low_corner_of_docks + 0.5f *
          vector3(static_cast<float_32_bit>(m_num_docks_along_x_axis_per_spiker - 1U) * m_distance_of_docks_in_meters,
                  static_cast<float_32_bit>(m_num_docks_along_y_axis_per_spiker - 1U) * m_distance_of_docks_in_meters,
                  static_cast<float_32_bit>(m_num_docks_along_c_axis_per_spiker - 1U) * m_distance_of_docks_in_meters)
          )
    , m_high_corner_of_spikers(
          m_high_corner_of_docks - 0.5f *
          vector3(static_cast<float_32_bit>(m_num_docks_along_x_axis_per_spiker - 1U) * m_distance_of_docks_in_meters,
                  static_cast<float_32_bit>(m_num_docks_along_y_axis_per_spiker - 1U) * m_distance_of_docks_in_meters,
                  static_cast<float_32_bit>(m_num_docks_along_c_axis_per_spiker - 1U) * m_distance_of_docks_in_meters)
          )

    , m_low_corner_of_ships(
          m_low_corner_of_docks - 0.5f * vector3(m_distance_of_docks_in_meters,
                                                 m_distance_of_docks_in_meters,
                                                 m_distance_of_docks_in_meters)
          )
    , m_high_corner_of_ships(
          m_high_corner_of_docks + 0.5f * vector3(m_distance_of_docks_in_meters,
                                                  m_distance_of_docks_in_meters,
                                                  m_distance_of_docks_in_meters)
          )

    , m_size_of_ship_movement_area_in_meters(size_of_ship_movement_area_in_meters)

    , m_speed_limits_of_ship_in_meters_per_second(speed_limits_of_ship_in_meters_per_second)

    , m_are_spikers_excitatory(are_spikers_excitatory)

    , m_ship_controller_ptr(ship_controller_ptr)
{
    ASSUMPTION(m_num_spikers_along_x_axis > 0U);
    ASSUMPTION(m_num_spikers_along_y_axis > 0U);
    ASSUMPTION(m_num_spikers_along_c_axis > 0U);

    ASSUMPTION(m_num_spikers > 0UL);
    ASSUMPTION(m_num_spikers <= max_number_of_objects_in_layer());

    ASSUMPTION(m_num_docks_along_x_axis_per_spiker > 0U);
    ASSUMPTION(m_num_docks_along_y_axis_per_spiker > 0U);
    ASSUMPTION(m_num_docks_along_c_axis_per_spiker > 0U);

    ASSUMPTION((m_num_docks_along_x_axis_per_spiker & 1U) != 0U ||
               (m_num_docks_along_y_axis_per_spiker & 1U) != 0U ||
               (m_num_docks_along_c_axis_per_spiker & 1U) != 0U );

    ASSUMPTION(m_num_docks > 0UL);
    ASSUMPTION(m_num_docks <= max_number_of_objects_in_layer());

    ASSUMPTION(m_num_ships_per_spiker > 0UL);

    ASSUMPTION(m_num_ships > 0UL);
    ASSUMPTION(m_num_ships <= max_number_of_objects_in_layer());

    ASSUMPTION(m_distance_of_docks_in_meters > 0.0f);

    ASSUMPTION(m_distance_of_spikers_along_x_axis_in_meters >= m_distance_of_docks_in_meters);
    ASSUMPTION(m_distance_of_spikers_along_y_axis_in_meters >= m_distance_of_docks_in_meters);
    ASSUMPTION(m_distance_of_spikers_along_c_axis_in_meters >= m_distance_of_docks_in_meters);

    ASSUMPTION(m_low_corner_of_docks(0) <= m_high_corner_of_docks(0));
    ASSUMPTION(m_low_corner_of_docks(1) <= m_high_corner_of_docks(1));
    ASSUMPTION(m_low_corner_of_docks(2) <= m_high_corner_of_docks(2));

    ASSUMPTION(m_low_corner_of_spikers(0) <= m_high_corner_of_spikers(0));
    ASSUMPTION(m_low_corner_of_spikers(1) <= m_high_corner_of_spikers(1));
    ASSUMPTION(m_low_corner_of_spikers(2) <= m_high_corner_of_spikers(2));
}


float_32_bit  network_layer_props::size_of_ship_movement_area_along_x_axis_in_meters(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_size_of_ship_movement_area_in_meters.size());
    return m_size_of_ship_movement_area_in_meters.at(layer_index)(0);
}

float_32_bit  network_layer_props::size_of_ship_movement_area_along_y_axis_in_meters(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_size_of_ship_movement_area_in_meters.size());
    return m_size_of_ship_movement_area_in_meters.at(layer_index)(1);
}

float_32_bit  network_layer_props::size_of_ship_movement_area_along_c_axis_in_meters(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_size_of_ship_movement_area_in_meters.size());
    return m_size_of_ship_movement_area_in_meters.at(layer_index)(2);
}

vector3 const&  network_layer_props::size_of_ship_movement_area_in_meters(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_size_of_ship_movement_area_in_meters.size());
    return m_size_of_ship_movement_area_in_meters.at(layer_index);
}


float_32_bit  network_layer_props::min_speed_of_ship_in_meters_per_second(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_speed_limits_of_ship_in_meters_per_second.size());
    return m_speed_limits_of_ship_in_meters_per_second.at(layer_index)(0);
}

float_32_bit  network_layer_props::max_speed_of_ship_in_meters_per_second(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_speed_limits_of_ship_in_meters_per_second.size());
    return m_speed_limits_of_ship_in_meters_per_second.at(layer_index)(1);
}

vector2 const&  network_layer_props::speed_limits_of_ship_in_meters_per_second(layer_index_type const  layer_index) const
{
    ASSUMPTION(layer_index < m_speed_limits_of_ship_in_meters_per_second.size());
    return m_speed_limits_of_ship_in_meters_per_second.at(layer_index);
}


void  network_layer_props::dock_sector_coordinates(
        vector3 const&  pos,
        sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
        ) const
{
    vector3 const  D(distance_of_docks_in_meters(),distance_of_docks_in_meters(),distance_of_docks_in_meters());
    vector3 const  u = (pos - (low_corner_of_docks() - 0.5f * D)).array() / D.array();
    x = (u(0) <= 0.0) ? 0UL : (u(0) >= num_docks_along_x_axis()) ? num_docks_along_x_axis() - 1UL : static_cast<natural_64_bit>(u(0));
    y = (u(1) <= 0.0) ? 0UL : (u(1) >= num_docks_along_y_axis()) ? num_docks_along_y_axis() - 1UL : static_cast<natural_64_bit>(u(1));
    c = (u(2) <= 0.0) ? 0UL : (u(2) >= num_docks_along_c_axis()) ? num_docks_along_c_axis() - 1UL : static_cast<natural_64_bit>(u(2));
}


void  network_layer_props::dock_sector_coordinates(
        object_index_type  index_into_layer,
        sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
        ) const
{
    c = static_cast<sector_coordinate_type>(index_into_layer / num_docks_in_xy_plane());
    index_into_layer = index_into_layer % num_docks_in_xy_plane();
    y = static_cast<sector_coordinate_type>(index_into_layer / static_cast<object_index_type>(num_docks_along_x_axis()));
    x = static_cast<sector_coordinate_type>(index_into_layer % static_cast<object_index_type>(num_docks_along_x_axis()));
}


object_index_type  network_layer_props::dock_sector_index(
        sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
        ) const
{
    return  static_cast<object_index_type>(c) * static_cast<object_index_type>(num_docks_in_xy_plane())
            + static_cast<object_index_type>(y) * static_cast<object_index_type>(num_docks_along_x_axis())
            + static_cast<object_index_type>(x)
            ;
}


vector3  network_layer_props::dock_sector_centre(
        sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
        ) const
{
    return low_corner_of_docks() + distance_of_docks_in_meters() * vector3( static_cast<float_32_bit>(x),
                                                                            static_cast<float_32_bit>(y),
                                                                            static_cast<float_32_bit>(c) );
}


void  network_layer_props::spiker_sector_coordinates(
        vector3 const&  pos,
        sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
        ) const
{
    vector3 const  D(distance_of_spikers_along_x_axis_in_meters(),
                     distance_of_spikers_along_y_axis_in_meters(),
                     distance_of_spikers_along_c_axis_in_meters());
    vector3 const  u = (pos - (low_corner_of_spikers() - 0.5f * D)).array() / D.array();
    x = (u(0) <= 0.0) ? 0U :
                        (u(0) >= num_spikers_along_x_axis()) ? static_cast<sector_coordinate_type>(num_spikers_along_x_axis() - 1UL) :
                                                               static_cast<sector_coordinate_type>(u(0));
    y = (u(1) <= 0.0) ? 0U :
                        (u(1) >= num_spikers_along_y_axis()) ? static_cast<sector_coordinate_type>(num_spikers_along_y_axis() - 1UL) :
                                                               static_cast<sector_coordinate_type>(u(1));
    c = (u(2) <= 0.0) ? 0U :
                        (u(2) >= num_spikers_along_c_axis()) ? static_cast<sector_coordinate_type>(num_spikers_along_c_axis() - 1UL) :
                                                               static_cast<sector_coordinate_type>(u(2));
}


void  network_layer_props::spiker_sector_coordinates(
        object_index_type  index_into_layer,
        sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
        ) const
{
    c = static_cast<sector_coordinate_type>(index_into_layer / num_spikers_in_xy_plane());
    index_into_layer = index_into_layer % num_spikers_in_xy_plane();
    y = static_cast<sector_coordinate_type>(index_into_layer / static_cast<object_index_type>(num_spikers_along_x_axis()));
    x = static_cast<sector_coordinate_type>(index_into_layer % static_cast<object_index_type>(num_spikers_along_x_axis()));
}


object_index_type  network_layer_props::spiker_sector_index(
        sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
        ) const
{
    return  static_cast<object_index_type>(c) * static_cast<object_index_type>(num_spikers_in_xy_plane())
            + static_cast<object_index_type>(y) * static_cast<object_index_type>(num_spikers_along_x_axis())
            + static_cast<object_index_type>(x)
            ;
}


vector3  network_layer_props::spiker_sector_centre(
        sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
        ) const
{
    return low_corner_of_spikers() + vector3( distance_of_spikers_along_x_axis_in_meters() * static_cast<float_32_bit>(x),
                                              distance_of_spikers_along_y_axis_in_meters() * static_cast<float_32_bit>(y),
                                              distance_of_spikers_along_c_axis_in_meters() * static_cast<float_32_bit>(c) );
}


object_index_type  network_layer_props::spiker_index_from_ship_index(object_index_type const  ship_index) const
{
    return ship_index / num_ships_per_spiker();
}


object_index_type  network_layer_props::ships_begin_index_of_spiker(object_index_type const  spiker_index) const
{
    return spiker_index * num_ships_per_spiker();
}


void  network_layer_props::spiker_sector_coordinates_from_dock_sector_coordinates(
        sector_coordinate_type const&  dock_x, sector_coordinate_type const&  dock_y, sector_coordinate_type const&  dock_c,
        sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
        ) const
{
    x = dock_x / num_docks_along_x_axis_per_spiker();
    y = dock_y / num_docks_along_y_axis_per_spiker();
    c = dock_c / num_docks_along_c_axis_per_spiker();
}


}
