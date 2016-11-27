#ifndef NETLAB_NETWORK_LAYER_PROPS_HPP_INCLUDED
#   define NETLAB_NETWORK_LAYER_PROPS_HPP_INCLUDED

#   include <netlab/ship_controller.hpp>
#   include <netlab/network_indices.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <memory>

namespace netlab {


using  sector_coordinate_type = natural_32_bit;


/**
 *
 *
 *
 *
 */
struct  network_layer_props
{
    network_layer_props(
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
            );

    ~network_layer_props() {}

    natural_32_bit  num_spikers_along_x_axis() const noexcept { return m_num_spikers_along_x_axis; }
    natural_32_bit  num_spikers_along_y_axis() const noexcept { return m_num_spikers_along_y_axis; }
    natural_32_bit  num_spikers_along_c_axis() const noexcept { return m_num_spikers_along_c_axis; }

    natural_64_bit  num_spikers_in_xy_plane() const noexcept { return m_num_spikers_in_xy_plane; }
    natural_64_bit  num_spikers() const noexcept { return m_num_spikers; }

    natural_32_bit  num_docks_along_x_axis_per_spiker() const noexcept { return m_num_docks_along_x_axis_per_spiker; }
    natural_32_bit  num_docks_along_y_axis_per_spiker() const noexcept { return m_num_docks_along_y_axis_per_spiker; }
    natural_32_bit  num_docks_along_c_axis_per_spiker() const noexcept { return m_num_docks_along_c_axis_per_spiker; }
    natural_64_bit  num_docks_per_spiker() const noexcept { return (natural_64_bit)num_docks_along_x_axis_per_spiker() *
                                                                   (natural_64_bit)num_docks_along_y_axis_per_spiker() *
                                                                   (natural_64_bit)num_docks_along_c_axis_per_spiker() ; }

    natural_32_bit  num_docks_along_x_axis() const noexcept { return m_num_docks_along_x_axis; }
    natural_32_bit  num_docks_along_y_axis() const noexcept { return m_num_docks_along_y_axis; }
    natural_32_bit  num_docks_along_c_axis() const noexcept { return m_num_docks_along_c_axis; }

    natural_64_bit  num_docks_in_xy_plane() const noexcept { return m_num_docks_in_xy_plane; }
    natural_64_bit  num_docks() const noexcept { return m_num_docks; }

    natural_32_bit  num_ships_per_spiker() const noexcept { return m_num_ships_per_spiker; }

    natural_64_bit  num_ships() const noexcept { return m_num_ships; }

    float_32_bit  distance_of_docks_in_meters() const noexcept { return m_distance_of_docks_in_meters; }

    float_32_bit  distance_of_spikers_along_x_axis_in_meters() const noexcept { return m_distance_of_spikers_along_x_axis_in_meters; }
    float_32_bit  distance_of_spikers_along_y_axis_in_meters() const noexcept { return m_distance_of_spikers_along_y_axis_in_meters; }
    float_32_bit  distance_of_spikers_along_c_axis_in_meters() const noexcept { return m_distance_of_spikers_along_c_axis_in_meters; }

    vector3 const&  low_corner_of_docks() const noexcept { return m_low_corner_of_docks; }
    vector3 const&  high_corner_of_docks() const noexcept { return m_high_corner_of_docks; }

    vector3 const&  low_corner_of_spikers() const noexcept { return m_low_corner_of_spikers; }
    vector3 const&  high_corner_of_spikers() const noexcept { return m_high_corner_of_spikers; }

    vector3 const&  low_corner_of_ships() const noexcept { return m_low_corner_of_ships; }
    vector3 const&  high_corner_of_ships() const noexcept { return m_high_corner_of_ships; }

    float_32_bit  size_of_ship_movement_area_along_x_axis_in_meters(layer_index_type const  layer_index) const;
    float_32_bit  size_of_ship_movement_area_along_y_axis_in_meters(layer_index_type const  layer_index) const;
    float_32_bit  size_of_ship_movement_area_along_c_axis_in_meters(layer_index_type const  layer_index) const;
    vector3 const&  size_of_ship_movement_area_in_meters(layer_index_type const  layer_index) const;
    std::vector<vector3> const&  sizes_of_ship_movement_areas_in_meters() const noexcept { return m_size_of_ship_movement_area_in_meters; }

    float_32_bit  min_speed_of_ship_in_meters_per_second(layer_index_type const  layer_index) const;
    float_32_bit  max_speed_of_ship_in_meters_per_second(layer_index_type const  layer_index) const;
    vector2 const&  speed_limits_of_ship_in_meters_per_second(layer_index_type const  layer_index) const;
    std::vector<vector2> const&  speed_limits_of_ship_in_meters_per_second() const noexcept { return m_speed_limits_of_ship_in_meters_per_second; }

    bool  are_spikers_excitatory() const noexcept { return m_are_spikers_excitatory; }

    std::shared_ptr<ship_controller const>  ship_controller_ptr() const noexcept { return m_ship_controller_ptr; }

    void  dock_sector_coordinates(
            vector3 const&  pos,
            sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
            ) const;
    void  dock_sector_coordinates(
            object_index_type  index_into_layer,
            sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
            ) const;
    object_index_type  dock_sector_index(
            sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
            ) const;
    vector3  dock_sector_centre(
            sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
            ) const;

    void  spiker_sector_coordinates(
            vector3 const&  pos,
            sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
            ) const;
    void  spiker_sector_coordinates(
            object_index_type index_into_layer,
            sector_coordinate_type& x, sector_coordinate_type& y, sector_coordinate_type& c
            ) const;
    object_index_type  spiker_sector_index(
            sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
            ) const;
    vector3  spiker_sector_centre(
            sector_coordinate_type const  x, sector_coordinate_type const  y, sector_coordinate_type const  c
            ) const;

    object_index_type  spiker_index_from_ship_index(object_index_type const  ship_index) const;
    object_index_type  ships_begin_index_of_spiker(object_index_type const  spiker_index) const;

    void  spiker_sector_coordinates_from_dock_sector_coordinates(
            sector_coordinate_type const&  dock_x, sector_coordinate_type const&  dock_y, sector_coordinate_type const&  dock_c,
            sector_coordinate_type&  x, sector_coordinate_type&  y, sector_coordinate_type&  c
            ) const;

    void  dock_low_sector_coordinates_from_spiker_sector_coordinates(
            sector_coordinate_type const&  x, sector_coordinate_type const&  y, sector_coordinate_type const&  c,
            sector_coordinate_type&  dock_x, sector_coordinate_type&  dock_y, sector_coordinate_type&  dock_c
            ) const;


private:
    natural_32_bit  m_num_spikers_along_x_axis;
    natural_32_bit  m_num_spikers_along_y_axis;
    natural_32_bit  m_num_spikers_along_c_axis;

    natural_64_bit  m_num_spikers_in_xy_plane;
    natural_64_bit  m_num_spikers;

    natural_32_bit  m_num_docks_along_x_axis_per_spiker;
    natural_32_bit  m_num_docks_along_y_axis_per_spiker;
    natural_32_bit  m_num_docks_along_c_axis_per_spiker;

    natural_32_bit  m_num_docks_along_x_axis;
    natural_32_bit  m_num_docks_along_y_axis;
    natural_32_bit  m_num_docks_along_c_axis;

    natural_64_bit  m_num_docks_in_xy_plane;
    natural_64_bit  m_num_docks;

    natural_32_bit  m_num_ships_per_spiker;

    natural_64_bit  m_num_ships;

    float_32_bit  m_distance_of_docks_in_meters;

    float_32_bit  m_distance_of_spikers_along_x_axis_in_meters;
    float_32_bit  m_distance_of_spikers_along_y_axis_in_meters;
    float_32_bit  m_distance_of_spikers_along_c_axis_in_meters;

    vector3  m_low_corner_of_docks;
    vector3  m_high_corner_of_docks;

    vector3  m_low_corner_of_spikers;
    vector3  m_high_corner_of_spikers;

    vector3  m_low_corner_of_ships;
    vector3  m_high_corner_of_ships;

    std::vector<vector3>  m_size_of_ship_movement_area_in_meters;

    std::vector<vector2>  m_speed_limits_of_ship_in_meters_per_second;

    bool  m_are_spikers_excitatory;

    std::shared_ptr<ship_controller const> const  m_ship_controller_ptr;
};


}

#endif
