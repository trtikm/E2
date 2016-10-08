#ifndef NETLAB_NETWORK_LAYER_PROPS_HPP_INCLUDED
#   define NETLAB_NETWORK_LAYER_PROPS_HPP_INCLUDED

#   include <netlab/ship_controller.hpp>
#   include <utility/tensor_math.hpp>
#   include <memory>

namespace netlab {


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

            float_32_bit const  size_of_ship_movement_area_along_x_axis_in_meters,
            float_32_bit const  size_of_ship_movement_area_along_y_axis_in_meters,
            float_32_bit const  size_of_ship_movement_area_along_c_axis_in_meters,

            float_32_bit const  min_speed_of_ship_in_meters_per_second,
            float_32_bit const  max_speed_of_ship_in_meters_per_second,

            bool const  are_spikers_excitatory,

            natural_8_bit const  num_bytes_per_spiker_for_parameters_pack,
            natural_8_bit const  num_bytes_per_dock_for_parameters_pack,
            natural_8_bit const  num_bytes_per_ship_for_parameters_pack,

            std::shared_ptr<ship_controller const> const  ship_controller_ptr
            );

    ~network_layer_props() {}

    natural_32_bit  num_spikers_along_x_axis() const noexcept { return m_num_spikers_along_x_axis; }
    natural_32_bit  num_spikers_along_y_axis() const noexcept { return m_num_spikers_along_y_axis; }
    natural_32_bit  num_spikers_along_c_axis() const noexcept { return m_num_spikers_along_c_axis; }

    natural_64_bit  num_spikers() const noexcept { return m_num_spikers; }

    natural_32_bit  num_docks_along_x_axis_per_spiker() const noexcept { return m_num_docks_along_x_axis_per_spiker; }
    natural_32_bit  num_docks_along_y_axis_per_spiker() const noexcept { return m_num_docks_along_y_axis_per_spiker; }
    natural_32_bit  num_docks_along_c_axis_per_spiker() const noexcept { return m_num_docks_along_c_axis_per_spiker; }

    natural_32_bit  num_docks_along_x_axis() const noexcept { return m_num_docks_along_x_axis; }
    natural_32_bit  num_docks_along_y_axis() const noexcept { return m_num_docks_along_y_axis; }
    natural_32_bit  num_docks_along_c_axis() const noexcept { return m_num_docks_along_c_axis; }

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

    float_32_bit  size_of_ship_movement_area_along_x_axis_in_meters() const noexcept { return m_size_of_ship_movement_area_along_x_axis_in_meters; }
    float_32_bit  size_of_ship_movement_area_along_y_axis_in_meters() const noexcept { return m_size_of_ship_movement_area_along_y_axis_in_meters; }
    float_32_bit  size_of_ship_movement_area_along_c_axis_in_meters() const noexcept { return m_size_of_ship_movement_area_along_c_axis_in_meters; }

    float_32_bit  min_speed_of_ship_in_meters_per_second() const noexcept { return m_min_speed_of_ship_in_meters_per_second; }
    float_32_bit  max_speed_of_ship_in_meters_per_second() const noexcept { return m_max_speed_of_ship_in_meters_per_second; }

    bool  are_spikers_excitatory() const noexcept { return m_are_spikers_excitatory; }

    natural_8_bit  num_bytes_per_spiker_for_parameters_pack() const noexcept { return m_num_bytes_per_spiker_for_parameters_pack; }
    natural_8_bit  num_bytes_per_dock_for_parameters_pack() const noexcept { return m_num_bytes_per_dock_for_parameters_pack; }
    natural_8_bit  num_bytes_per_ship_for_parameters_pack() const noexcept { return m_num_bytes_per_ship_for_parameters_pack; }

    std::shared_ptr<ship_controller const>  ship_controller_ptr() const noexcept { return m_ship_controller_ptr; }

    void  dock_sector_coordinates(vector3 const&  pos, natural_32_bit&  x, natural_32_bit&  y, natural_32_bit&  c) const;
    natural_64_bit  dock_sector_index(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const;
    vector3  dock_sector_centre(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const;

    void  spiker_sector_coordinates(vector3 const&  pos, natural_32_bit&  x, natural_32_bit&  y, natural_32_bit&  c) const;
    natural_64_bit  spiker_sector_index(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const;
    vector3  spiker_sector_centre(natural_32_bit const  x, natural_32_bit const  y, natural_32_bit const  c) const;

private:
    natural_32_bit  m_num_spikers_along_x_axis;
    natural_32_bit  m_num_spikers_along_y_axis;
    natural_32_bit  m_num_spikers_along_c_axis;

    natural_64_bit  m_num_spikers;

    natural_32_bit  m_num_docks_along_x_axis_per_spiker;
    natural_32_bit  m_num_docks_along_y_axis_per_spiker;
    natural_32_bit  m_num_docks_along_c_axis_per_spiker;

    natural_32_bit  m_num_docks_along_x_axis;
    natural_32_bit  m_num_docks_along_y_axis;
    natural_32_bit  m_num_docks_along_c_axis;

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

    float_32_bit  m_size_of_ship_movement_area_along_x_axis_in_meters;
    float_32_bit  m_size_of_ship_movement_area_along_y_axis_in_meters;
    float_32_bit  m_size_of_ship_movement_area_along_c_axis_in_meters;

    float_32_bit  m_min_speed_of_ship_in_meters_per_second;
    float_32_bit  m_max_speed_of_ship_in_meters_per_second;

    bool  m_are_spikers_excitatory;

    natural_8_bit  m_num_bytes_per_spiker_for_parameters_pack;
    natural_8_bit  m_num_bytes_per_dock_for_parameters_pack;
    natural_8_bit  m_num_bytes_per_ship_for_parameters_pack;

    std::shared_ptr<ship_controller const> const  m_ship_controller_ptr;
};


}

#endif
