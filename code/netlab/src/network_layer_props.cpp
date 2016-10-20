#include <netlab/network_layer_props.hpp>
#include <netlab/network_object_id.hpp>
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

        float_32_bit const  size_of_ship_movement_area_along_x_axis_in_meters,
        float_32_bit const  size_of_ship_movement_area_along_y_axis_in_meters,
        float_32_bit const  size_of_ship_movement_area_along_c_axis_in_meters,

        float_32_bit const  min_speed_of_ship_in_meters_per_second,
        float_32_bit const  max_speed_of_ship_in_meters_per_second,

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

    , m_size_of_ship_movement_area_along_x_axis_in_meters(size_of_ship_movement_area_along_x_axis_in_meters)
    , m_size_of_ship_movement_area_along_y_axis_in_meters(size_of_ship_movement_area_along_y_axis_in_meters)
    , m_size_of_ship_movement_area_along_c_axis_in_meters(size_of_ship_movement_area_along_c_axis_in_meters)

    , m_min_speed_of_ship_in_meters_per_second(min_speed_of_ship_in_meters_per_second)
    , m_max_speed_of_ship_in_meters_per_second(max_speed_of_ship_in_meters_per_second)

    , m_are_spikers_excitatory(are_spikers_excitatory)

    , m_ship_controller_ptr(ship_controller_ptr)
{
    ASSUMPTION(m_num_spikers_along_x_axis > 0U);
    ASSUMPTION(m_num_spikers_along_y_axis > 0U);
    ASSUMPTION(m_num_spikers_along_c_axis > 0U);

    ASSUMPTION(m_num_spikers > 0UL);
    ASSUMPTION(m_num_spikers <= max_num_objects_in_a_layer());

    ASSUMPTION(m_num_docks_along_x_axis_per_spiker > 0U);
    ASSUMPTION(m_num_docks_along_y_axis_per_spiker > 0U);
    ASSUMPTION(m_num_docks_along_c_axis_per_spiker > 0U);

    ASSUMPTION((m_num_docks_along_x_axis_per_spiker & 1U) != 0U ||
               (m_num_docks_along_y_axis_per_spiker & 1U) != 0U ||
               (m_num_docks_along_c_axis_per_spiker & 1U) != 0U );

    ASSUMPTION(m_num_docks > 0UL);
    ASSUMPTION(m_num_docks <= max_num_objects_in_a_layer());

    ASSUMPTION(m_num_ships_per_spiker > 0UL);

    ASSUMPTION(m_num_ships > 0UL);
    ASSUMPTION(m_num_ships <= max_num_objects_in_a_layer());

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

    ASSUMPTION(m_size_of_ship_movement_area_along_x_axis_in_meters > 0.0f);
    ASSUMPTION(m_size_of_ship_movement_area_along_x_axis_in_meters < m_high_corner_of_docks(0) - m_low_corner_of_docks(0));

    ASSUMPTION(m_size_of_ship_movement_area_along_y_axis_in_meters > 0.0f);
    ASSUMPTION(m_size_of_ship_movement_area_along_y_axis_in_meters < m_high_corner_of_docks(1) - m_low_corner_of_docks(1));

    ASSUMPTION(m_size_of_ship_movement_area_along_c_axis_in_meters > 0.0f);
    ASSUMPTION(m_size_of_ship_movement_area_along_c_axis_in_meters < m_high_corner_of_docks(2) - m_low_corner_of_docks(2));

    ASSUMPTION(m_min_speed_of_ship_in_meters_per_second >= 0.0f);
    ASSUMPTION(m_max_speed_of_ship_in_meters_per_second >= m_min_speed_of_ship_in_meters_per_second);
}


void  network_layer_props::dock_sector_coordinates(vector3 const&  pos, natural_32_bit&  x, natural_32_bit&  y, natural_32_bit&  c) const
{
    vector3 const  D(distance_of_docks_in_meters(),distance_of_docks_in_meters(),distance_of_docks_in_meters());
    vector3 const  u = (pos - (low_corner_of_docks() - 0.5f * D)).array() / D.array();
    x = (u(0) <= 0.0) ? 0UL : (u(0) >= num_docks_along_x_axis()) ? num_docks_along_x_axis() - 1UL : static_cast<natural_64_bit>(u(0));
    y = (u(1) <= 0.0) ? 0UL : (u(1) >= num_docks_along_y_axis()) ? num_docks_along_y_axis() - 1UL : static_cast<natural_64_bit>(u(1));
    c = (u(2) <= 0.0) ? 0UL : (u(2) >= num_docks_along_c_axis()) ? num_docks_along_c_axis() - 1UL : static_cast<natural_64_bit>(u(2));
}


void  network_layer_props::dock_sector_coordinates(
        natural_64_bit  index_into_layer,
        natural_64_bit&  x, natural_32_bit&  y, natural_32_bit&  c
        ) const
{
    c = static_cast<natural_32_bit>(index_into_layer / num_docks_in_xy_plane());
    index_into_layer = index_into_layer % num_docks_in_xy_plane();
    y = static_cast<natural_32_bit>(index_into_layer / static_cast<natural_64_bit>(num_docks_along_x_axis()));
    x = static_cast<natural_32_bit>(index_into_layer % static_cast<natural_64_bit>(num_docks_along_x_axis()));
}


natural_64_bit  network_layer_props::dock_sector_index(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const
{
    return  static_cast<natural_64_bit>(c) * num_docks_in_xy_plane()
            + static_cast<natural_64_bit>(y) * static_cast<natural_64_bit>(num_docks_along_x_axis())
            + static_cast<natural_64_bit>(x)
            ;
}


vector3  network_layer_props::dock_sector_centre(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const
{
    return low_corner_of_docks() + distance_of_docks_in_meters() * vector3( static_cast<float_32_bit>(x),
                                                                            static_cast<float_32_bit>(y),
                                                                            static_cast<float_32_bit>(c) );
}


void  network_layer_props::spiker_sector_coordinates(vector3 const&  pos, natural_32_bit&  x, natural_32_bit&  y, natural_32_bit&  c) const
{
    vector3 const  D(distance_of_spikers_along_x_axis_in_meters(),
                     distance_of_spikers_along_y_axis_in_meters(),
                     distance_of_spikers_along_c_axis_in_meters());
    vector3 const  u = (pos - (low_corner_of_spikers() - 0.5f * D)).array() / D.array();
    x = (u(0) <= 0.0) ? 0UL : (u(0) >= num_spikers_along_x_axis()) ? num_spikers_along_x_axis() - 1UL : static_cast<natural_64_bit>(u(0));
    y = (u(1) <= 0.0) ? 0UL : (u(1) >= num_spikers_along_y_axis()) ? num_spikers_along_y_axis() - 1UL : static_cast<natural_64_bit>(u(1));
    c = (u(2) <= 0.0) ? 0UL : (u(2) >= num_spikers_along_c_axis()) ? num_spikers_along_c_axis() - 1UL : static_cast<natural_64_bit>(u(2));
}


void  network_layer_props::spiker_sector_coordinates(
        natural_64_bit  index_into_layer,
        natural_64_bit&  x, natural_32_bit&  y, natural_32_bit&  c
        ) const
{
    c = static_cast<natural_32_bit>(index_into_layer / num_spikers_in_xy_plane());
    index_into_layer = index_into_layer % num_spikers_in_xy_plane();
    y = static_cast<natural_32_bit>(index_into_layer / static_cast<natural_64_bit>(num_spikers_along_x_axis()));
    x = static_cast<natural_32_bit>(index_into_layer % static_cast<natural_64_bit>(num_spikers_along_x_axis()));
}


natural_64_bit  network_layer_props::spiker_sector_index(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const
{
    return  static_cast<natural_64_bit>(c) * num_spikers_in_xy_plane()
            + static_cast<natural_64_bit>(y) * static_cast<natural_64_bit>(num_spikers_along_x_axis())
            + static_cast<natural_64_bit>(x)
            ;
}


vector3  network_layer_props::spiker_sector_centre(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const
{
    return low_corner_of_spikers() + vector3( distance_of_spikers_along_x_axis_in_meters() * static_cast<float_32_bit>(x),
                                              distance_of_spikers_along_y_axis_in_meters() * static_cast<float_32_bit>(y),
                                              distance_of_spikers_along_c_axis_in_meters() * static_cast<float_32_bit>(c) );
}


natural_64_bit  network_layer_props::spiker_index_from_ship_index(natural_64_bit const  ship_index) const
{
    return ship_index / num_ships_per_spiker();
}


natural_64_bit  network_layer_props::ships_begin_index_of_spiker(natural_64_bit const  spiker_index) const
{
    return spiker_index * num_ships_per_spiker();
}


void  network_layer_props::spiker_sector_coordinates_from_dock_sector_coordinates(
        natural_32_bit const&  dock_x, natural_32_bit const&  dock_y, natural_32_bit const&  dock_c,
        natural_32_bit&  x, natural_32_bit&  y, natural_32_bit&  c
        ) const
{
    x = dock_x / num_docks_along_x_axis_per_spiker();
    y = dock_y / num_docks_along_y_axis_per_spiker();
    c = dock_c / num_docks_along_c_axis_per_spiker();
}


}
