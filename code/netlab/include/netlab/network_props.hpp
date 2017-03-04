#ifndef NETLAB_NETWORK_PROPS_HPP_INCLUDED
#   define NETLAB_NETWORK_PROPS_HPP_INCLUDED

#   include <netlab/network_layer_props.hpp>
#   include <netlab/network_indices.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <limits>

namespace netlab {


/**
 *
 *
 *
 */
struct network_props
{
    network_props(
            std::vector<network_layer_props> const&  layer_props,

            float_32_bit const  update_time_step_in_seconds,

            float_32_bit const  spiking_potential_magnitude,

            float_32_bit const  mini_spiking_potential_magnitude,
            float_32_bit const  average_mini_spiking_period_in_seconds,

            float_32_bit const  max_connection_distance_in_meters,

            natural_32_bit const  num_threads_to_use
            );

    ~network_props() {}

    std::vector<network_layer_props> const&  layer_props() const noexcept { return m_layer_props; }

    natural_64_bit  num_spikers() const { return m_num_spikers; }
    natural_64_bit  num_docks() const { return m_num_docks; }
    natural_64_bit  num_ships() const { return m_num_ships; }

    float_32_bit  update_time_step_in_seconds() const noexcept { return m_update_time_step_in_seconds; }

    float_32_bit  spiking_potential_magnitude() const noexcept { return m_spiking_potential_magnitude; }

    float_32_bit  mini_spiking_potential_magnitude() const noexcept { return m_mini_spiking_potential_magnitude; }
    float_32_bit  average_mini_spiking_period_in_seconds() const noexcept { return m_average_mini_spiking_period_in_seconds; }

    natural_64_bit  num_mini_spikes_to_generate_per_simulation_step() const;

    float_32_bit  max_connection_distance_in_meters() const noexcept { return m_max_connection_distance_in_meters; }

    natural_32_bit  num_threads_to_use() const noexcept { return m_num_threads_to_use; }

    void  set_max_connection_distance_in_meters(float_32_bit const  value) { m_max_connection_distance_in_meters = value; }

    layer_index_type  find_layer_index(float_32_bit const  coord_along_c_axis) const;

//    void  set_update_time_step_in_seconds(float_32_bit const  value) { m_update_time_step_in_seconds = value; }

//    void  set_mini_spiking_potential_magnitude(float_32_bit const  value) { m_mini_spiking_potential_magnitude = value; }
//    void  set_average_mini_spiking_period_in_seconds(float_32_bit const  value) { m_average_mini_spiking_period_in_seconds = value; }

//    void  set_min_speed_of_ship_in_meters_per_second(float_32_bit const  value) { m_min_speed_of_ship_in_meters_per_second = value; }
//    void  set_max_speed_of_ship_in_meters_per_second(float_32_bit const  value) { m_max_speed_of_ship_in_meters_per_second = value; }

private:
    std::vector<network_layer_props>  m_layer_props;

    float_32_bit  m_update_time_step_in_seconds;

    float_32_bit  m_spiking_potential_magnitude;

    float_32_bit  m_mini_spiking_potential_magnitude;
    float_32_bit  m_average_mini_spiking_period_in_seconds;

    float_32_bit  m_max_connection_distance_in_meters;

    natural_32_bit  m_num_threads_to_use;

    std::vector<float_32_bit>  m_max_coods_along_c_axis;

    natural_64_bit  m_num_spikers;
    natural_64_bit  m_num_docks;
    natural_64_bit  m_num_ships;
};


}

#endif
