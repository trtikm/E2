#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>



void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_32_bit cells_x = 1U; cells_x <= 51U; cells_x += 10U)
        for (natural_32_bit cells_y = 1U; cells_y <= 51U; cells_y += 10U)
            for (natural_16_bit tissue_cell_kinds = 1U; tissue_cell_kinds <= 25U; ++tissue_cell_kinds)
                for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds <= 50U; ++sensory_cell_kinds)
                {
                    natural_16_bit const synapses_to_muscles_kinds = sensory_cell_kinds;

                    natural_16_bit const num_bits_per_cell = 8U;
                    natural_16_bit const num_bits_per_synapse = 8U;
                    natural_16_bit const num_bits_per_signalling = 8U;

                    std::vector<natural_32_bit> num_tissue_cells_of_cell_kind;
                    std::vector<natural_32_bit> num_synapses_in_territory_of_cell_kind;
                    for (natural_16_bit i = 1U; i <= tissue_cell_kinds; ++i)
                    {
                        num_tissue_cells_of_cell_kind.push_back(i);
                        num_synapses_in_territory_of_cell_kind.push_back(10U+i);
                    }

                    std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
                    for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
                        num_sensory_cells_of_cell_kind.push_back((i%2 == 0) ? 1U : 5000U);

                    std::vector<natural_32_bit> num_synapses_to_muscles_of_synapse_kind;
                    for (natural_16_bit i = 0U; i < synapses_to_muscles_kinds; ++i)
                        num_synapses_to_muscles_of_synapse_kind.push_back((i%2 == 0) ? 1U : 5000U);

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
                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue =
                            std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                                    new cellab::dynamic_state_of_neural_tissue(static_tissue)
                                    );

                    TEST_PROGRESS_UPDATE();
                }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
