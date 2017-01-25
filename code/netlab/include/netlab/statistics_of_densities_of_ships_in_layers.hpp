#ifndef NETLAB_STATISTICS_OF_DENSITIES_OF_SHIPS_IN_LAYERS_HPP_INCLUDED
#   define NETLAB_STATISTICS_OF_DENSITIES_OF_SHIPS_IN_LAYERS_HPP_INCLUDED

#   include <netlab/network_indices.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <array>

namespace netlab {


struct  statistics_of_densities_of_ships_in_layers
{
    using  distribution_of_spikers_by_density_of_ships = std::array<natural_64_bit, 10ULL>;

    statistics_of_densities_of_ships_in_layers(natural_64_bit const num_layers);

    statistics_of_densities_of_ships_in_layers(
            std::vector<float_32_bit> const&  ideal_densities,
            std::vector<float_32_bit> const&  minimal_densities,
            std::vector<float_32_bit> const&  maximal_densities,
            std::vector<float_32_bit> const&  average_densities,
            std::vector<distribution_of_spikers_by_density_of_ships> const&  distribution_of_spikers_by_densities_of_ships
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


}

#endif
