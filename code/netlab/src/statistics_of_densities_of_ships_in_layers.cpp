#include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#include <utility/assumptions.hpp>

namespace netlab {


statistics_of_densities_of_ships_in_layers::statistics_of_densities_of_ships_in_layers(
        natural_64_bit const num_layers
        )
    : m_ideal_densities(num_layers,0.0f)
    , m_minimal_densities(num_layers, 0.0f)
    , m_maximal_densities(num_layers, 0.0f)
    , m_average_densities(num_layers, 0.0f)
    , m_distribution_of_spikers_by_densities_of_ships(
            num_layers,
            []() -> distribution_of_spikers_by_density_of_ships {
                distribution_of_spikers_by_density_of_ships  result;
                for (std::size_t  i = 0ULL; i != result.size(); ++i)
                    result.at(i) = 0ULL;
                }())
{
    ASSUMPTION(m_ideal_densities.size() != 0ULL);
    ASSUMPTION(m_ideal_densities.size() <= max_number_of_layers());
}


statistics_of_densities_of_ships_in_layers::statistics_of_densities_of_ships_in_layers(
        std::vector<float_32_bit> const&  ideal_densities,
        std::vector<float_32_bit> const&  minimal_densities,
        std::vector<float_32_bit> const&  maximal_densities,
        std::vector<float_32_bit> const&  average_densities,
        std::vector<distribution_of_spikers_by_density_of_ships> const&  distribution_of_spikers_by_densities_of_ships
        )
    : m_ideal_densities(ideal_densities)
    , m_minimal_densities(minimal_densities)
    , m_maximal_densities(maximal_densities)
    , m_average_densities(average_densities)
    , m_distribution_of_spikers_by_densities_of_ships(distribution_of_spikers_by_densities_of_ships)
{
    ASSUMPTION(m_ideal_densities.size() != 0ULL);
    ASSUMPTION(m_ideal_densities.size() <= max_number_of_layers());
    ASSUMPTION(m_ideal_densities.size() == m_minimal_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_maximal_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_average_densities.size());
    ASSUMPTION(m_ideal_densities.size() == m_distribution_of_spikers_by_densities_of_ships.size());
    ASSUMPTION(
            [](std::vector<float_32_bit> const&  ideal,
               std::vector<float_32_bit> const&  minimal,
               std::vector<float_32_bit> const&  maximal,
               std::vector<float_32_bit> const&  average ) -> bool {
                for (std::size_t  i = 0ULL; i != ideal.size(); ++i)
                    if (minimal.at(i) < 0.0f || average.at(i) < minimal.at(i) || maximal.at(i) < average.at(i))
                        return false;
                return true;
            }(m_ideal_densities,m_minimal_densities,m_maximal_densities,m_average_densities)
            );
}


}
