#ifndef NETLAB_STATISTICS_OF_DENSITIES_OF_SHIPS_IN_LAYERS_HPP_INCLUDED
#   define NETLAB_STATISTICS_OF_DENSITIES_OF_SHIPS_IN_LAYERS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <netlab/network_props.hpp>
#   include <netlab/network_layer_props.hpp>
#   include <netlab/extra_data_for_spikers.hpp>
#   include <netlab/access_to_movement_area_centers.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <array>

namespace netlab {


using  distribution_of_spikers_by_density_of_ships = std::array<natural_64_bit, 10ULL>;


struct  statistics_of_densities_of_ships_in_layers
{
    statistics_of_densities_of_ships_in_layers(
            std::vector<float_32_bit>&  ideal_densities,
            std::vector<float_32_bit>&  minimal_densities,
            std::vector<float_32_bit>&  maximal_densities,
            std::vector<float_32_bit>&  average_densities,
            std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers_by_densities_of_ships
            );

    std::vector<float_32_bit> const&  ideal_densities() const noexcept { return m_ideal_densities; }
    std::vector<float_32_bit> const&  minimal_densities() const noexcept { return m_minimal_densities; }
    std::vector<float_32_bit> const&  maximal_densities() const noexcept { return m_maximal_densities; }
    std::vector<float_32_bit> const&  average_densities() const noexcept { return m_average_densities; }
    std::vector<distribution_of_spikers_by_density_of_ships> const&  distribution_of_spikers_by_densities_of_ships() const noexcept
    { return m_distribution_of_spikers_by_densities_of_ships; }

private:
    std::vector<float_32_bit>  m_ideal_densities;
    std::vector<float_32_bit>  m_minimal_densities;
    std::vector<float_32_bit>  m_maximal_densities;
    std::vector<float_32_bit>  m_average_densities;
    std::vector<distribution_of_spikers_by_density_of_ships>  m_distribution_of_spikers_by_densities_of_ships;
};


float_32_bit  compute_ideal_density_of_ships_in_layer(
        natural_32_bit const  num_docks_per_spiker_in_layer,
        natural_32_bit const  num_ships_per_spiker_in_layer
        );

void  compute_ideal_densities_of_ships_in_layers(
        network_props const&  props,
        std::vector<float_32_bit>&  ideal_densities
        );

void  initialise_densities_of_ships_per_spiker_in_layer(
        natural_64_bit const  num_spikers_in_layer,
        extra_data_for_spikers_in_one_layer&  densities_of_ships_per_spiker
        );

void  initialise_densities_of_ships_per_spiker_in_layers(
        network_props const&  props,
        extra_data_for_spikers_in_layers&  densities_of_ships_per_spiker
        );

void  compute_densities_of_ships_per_spiker_in_layers(
        network_props const&  props,
        access_to_movement_area_centers const&  movement_area_centers,
        extra_data_for_spikers_in_layers&  densities_of_ships_per_spiker
        );

void  compute_statistics_of_density_of_ships_in_layers(
        network_props const&  props,
        extra_data_for_spikers_in_layers const&  densities_of_ships_per_spiker,
        std::vector<float_32_bit> const&  ideal_densities,
        std::vector<float_32_bit>&  minimal_densities,
        std::vector<float_32_bit>&  maximal_densities,
        std::vector<float_32_bit>&  average_densities,
        std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers
        );


}

#endif
