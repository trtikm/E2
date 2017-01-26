#include <netlab/statistics_of_densities_of_ships_in_layers.hpp>
#include <utility/assumptions.hpp>

namespace netlab {


statistics_of_densities_of_ships_in_layers::statistics_of_densities_of_ships_in_layers(
        std::vector<float_32_bit>&  ideal_densities,
        std::vector<float_32_bit>&  minimal_densities,
        std::vector<float_32_bit>&  maximal_densities,
        std::vector<float_32_bit>&  average_densities,
        std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers_by_densities_of_ships
        )
    : m_ideal_densities()
    , m_minimal_densities()
    , m_maximal_densities()
    , m_average_densities()
    , m_distribution_of_spikers_by_densities_of_ships()
{
    m_ideal_densities.swap(ideal_densities);
    m_minimal_densities.swap(minimal_densities);
    m_maximal_densities.swap(maximal_densities);
    m_average_densities.swap(average_densities);
    m_distribution_of_spikers_by_densities_of_ships.swap(distribution_of_spikers_by_densities_of_ships);

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


float_32_bit  compute_ideal_density_of_ships_in_layer(
        natural_32_bit const  num_docks_per_spiker_in_layer,
        natural_32_bit const  num_ships_per_spiker_in_layer
        )
{
    ASSUMPTION(num_docks_per_spiker_in_layer != 0U);
    return (float_32_bit)num_ships_per_spiker_in_layer / (float_32_bit)num_docks_per_spiker_in_layer;
}


void  compute_ideal_densities_of_ships_in_layers(
        network_props const&  props,
        std::vector<float_32_bit>&  ideal_densities
        )
{
    ideal_densities.reserve(props.layer_props().size());
    for (layer_index_type  i = 0U; i != props.layer_props().size(); ++i)
        ideal_densities.push_back( compute_ideal_density_of_ships_in_layer(props.layer_props().at(i).num_docks_per_spiker(),
                                                                           props.layer_props().at(i).num_ships_per_spiker()) );
}


void  initialise_densities_of_ships_per_spiker_in_layer(
    natural_64_bit const  num_spikers_in_layer,
    extra_data_for_spikers_in_one_layer&  densities_of_ships_per_spiker
    )
{
    densities_of_ships_per_spiker.resize(num_spikers_in_layer,0.0f);
}


void  initialise_densities_of_ships_per_spiker_in_layers(
    network_props const&  props,
    extra_data_for_spikers_in_layers&  densities_of_ships_per_spiker
    )
{
    densities_of_ships_per_spiker.clear();
    densities_of_ships_per_spiker.resize(props.layer_props().size());
    for (layer_index_type i = 0U; i != props.layer_props().size(); ++i)
        initialise_densities_of_ships_per_spiker_in_layer(props.layer_props().at(i).num_spikers(),densities_of_ships_per_spiker.at(i));
}


void  compute_densities_of_ships_per_spiker_in_layers(
    network_props const&  props,
    extra_data_for_spikers_in_layers&  densities_of_ships_per_spiker
    )
{
    ASSUMPTION(densities_of_ships_per_spiker.size() == props.layer_props().size());
    // TODO!
}


void  compute_statistics_of_density_of_ships_in_layers(
    network_props const&  props,
    extra_data_for_spikers_in_layers const&  densities_of_ships_per_spiker,
    std::vector<float_32_bit> const&  ideal_densities,
    std::vector<float_32_bit>&  minimal_densities,
    std::vector<float_32_bit>&  maximal_densities,
    std::vector<float_32_bit>&  average_densities,
    std::vector<distribution_of_spikers_by_density_of_ships>&  distribution_of_spikers
    )
{
    ASSUMPTION(ideal_densities.size() == props.layer_props().size());
    ASSUMPTION(densities_of_ships_per_spiker.size() == props.layer_props().size());

    minimal_densities.clear();
    minimal_densities.resize(props.layer_props().size(),0.0f);

    maximal_densities.clear();
    maximal_densities.resize(props.layer_props().size(),0.0f);

    average_densities.clear();
    average_densities.resize(props.layer_props().size(),0.0f);

    distribution_of_spikers.clear();
    {
        distribution_of_spikers_by_density_of_ships  zeros;
        zeros.fill(0ULL);
        distribution_of_spikers.resize(props.layer_props().size(),zeros);
    }

    // TODO!
}


}
