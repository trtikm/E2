#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellconnect/fill_delimiters_between_territorial_lists.hpp>
#include <utility/test.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <array>
#include <algorithm>


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector<natural_32_bit> coefs = {
            1, 4, 3, 8, 1, 9, 2, 5, 4, 2, 7, 6, 6, 5, 8, 9, 3
            };

    for (natural_32_bit cells_x = 1U; cells_x <= 11U; cells_x += 5U)
        for (natural_32_bit cells_y = 1U; cells_y <= 11U; cells_y += 5U)
            for (natural_16_bit tissue_cell_kinds = 1U; tissue_cell_kinds <= 11U; tissue_cell_kinds += 5)
                for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds <= 11U; sensory_cell_kinds += 5)
                {
                    natural_16_bit const synapses_to_muscles_kinds = sensory_cell_kinds;

                    natural_16_bit const num_bits_per_cell = 8U;
                    natural_16_bit const num_bits_per_synapse = 8U;
                    natural_16_bit const num_bits_per_signalling = 8U;

                    std::vector<natural_32_bit> num_tissue_cells_of_cell_kind;
                    std::vector<natural_32_bit> num_synapses_in_territory_of_cell_kind;
                    for (natural_16_bit i = 0U; i < tissue_cell_kinds; ++i)
                    {
                        natural_32_bit coef = coefs.at(i % coefs.size());
                        num_tissue_cells_of_cell_kind.push_back(coef + (1U + tissue_cell_kinds / 3U));

                        coef = coefs.at((i + 5U) % coefs.size());
                        num_synapses_in_territory_of_cell_kind.push_back(coef + (1U + tissue_cell_kinds / 2U));
                    }

                    std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
                    for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
                        if (i + 1U == sensory_cell_kinds)
                            num_sensory_cells_of_cell_kind.push_back(1U);
                        else
                        {
                            natural_32_bit const coef = coefs.at(i % coefs.size());
                            num_sensory_cells_of_cell_kind.push_back(coef + (1U + sensory_cell_kinds / 4U));
                        }

                    std::vector<natural_32_bit> const num_synapses_to_muscles_of_synapse_kind(synapses_to_muscles_kinds,10U);

                    std::vector<integer_8_bit> const x_radius_of_signalling_neighbourhood_of_cell(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const y_radius_of_signalling_neighbourhood_of_cell(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const columnar_radius_of_signalling_neighbourhood_of_cell(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const x_radius_of_signalling_neighbourhood_of_synapse(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const y_radius_of_signalling_neighbourhood_of_synapse(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const columnar_radius_of_signalling_neighbourhood_of_synapse(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const x_radius_of_cellular_neighbourhood_of_signalling(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const y_radius_of_cellular_neighbourhood_of_signalling(tissue_cell_kinds,1U);
                    std::vector<integer_8_bit> const columnar_radius_of_cellular_neighbourhood_of_signalling(tissue_cell_kinds,1U);

                    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue =
                            std::shared_ptr<cellab::static_state_of_neural_tissue const>(
                                    new cellab::static_state_of_neural_tissue(
                                        tissue_cell_kinds,
                                        sensory_cell_kinds,
                                        synapses_to_muscles_kinds,
                                        num_bits_per_cell,
                                        num_bits_per_synapse,
                                        num_bits_per_signalling,
                                        cells_x,
                                        cells_y,
                                        num_tissue_cells_of_cell_kind,
                                        num_synapses_in_territory_of_cell_kind,
                                        num_sensory_cells_of_cell_kind,
                                        num_synapses_to_muscles_of_synapse_kind,
                                        true,
                                        true,
                                        true,
                                        x_radius_of_signalling_neighbourhood_of_cell,
                                        y_radius_of_signalling_neighbourhood_of_cell,
                                        columnar_radius_of_signalling_neighbourhood_of_cell,
                                        x_radius_of_signalling_neighbourhood_of_synapse,
                                        y_radius_of_signalling_neighbourhood_of_synapse,
                                        columnar_radius_of_signalling_neighbourhood_of_synapse,
                                        x_radius_of_cellular_neighbourhood_of_signalling,
                                        y_radius_of_cellular_neighbourhood_of_signalling,
                                        columnar_radius_of_cellular_neighbourhood_of_signalling
                                        ));

                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue =
                            std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                                    new cellab::dynamic_state_of_neural_tissue(static_tissue)
                                    );

                    for (natural_32_bit num_threads = 0U; num_threads <= 16; num_threads += 8)
                    {
                        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
                            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                            {
                                for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
                                {
                                    natural_32_bit const  error_count =
                                            static_tissue->num_synapses_in_territory_of_cell_kind(
                                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                                );
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,0U));
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,1U));
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,2U));
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,3U));
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,4U));
                                    value_to_bits(error_count, dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,5U));
                                }
                                TEST_PROGRESS_UPDATE();
                            }

                        cellconnect::fill_delimiters_between_territorial_lists(
                                    dynamic_tissue,
                                    cellconnect::delimiters_fill_kind::SYNAPSES_DISTRIBUTED_REGULARLY,
                                    std::max(1U,num_threads)
                                    );

                        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
                            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                            {
                                for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
                                {
                                    natural_32_bit const  error_count =
                                            static_tissue->num_synapses_in_territory_of_cell_kind(
                                                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                                );
                                    std::array<natural_32_bit,6U> const  delimiters = {
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,0U)),
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,1U)),
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,2U)),
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,3U)),
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,4U)),
                                            bits_to_value<natural_32_bit>(dynamic_tissue->find_bits_of_delimiter_between_territorial_lists(x,y,c,5U))
                                            };
                                    TEST_SUCCESS(
                                                0U <= delimiters.at(0U) &&
                                                delimiters.at(0U) <= delimiters.at(1U) &&
                                                delimiters.at(1U) <= delimiters.at(2U) &&
                                                delimiters.at(2U) <= delimiters.at(3U) &&
                                                delimiters.at(3U) <= delimiters.at(4U) &&
                                                delimiters.at(4U) <= delimiters.at(5U) &&
                                                delimiters.at(5U) < error_count
                                                );
                                }
                                TEST_PROGRESS_UPDATE();
                            }
                    }

                    std::rotate(coefs.begin(), coefs.begin() + 1, coefs.end());
                }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
