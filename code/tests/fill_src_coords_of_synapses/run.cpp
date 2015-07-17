#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <utility/test.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <algorithm>


static void build_matrix(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        std::vector<natural_32_bit>& matrix
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(static_state_ptr->num_cells_of_cell_kind(static_state_ptr->num_kinds_of_cells() - 1U) == 1U);

    static std::vector<natural_32_bit> multipliers = {
            1, 8, 5, 3, 2, 4, 0, 9, 6, 1, 7, 3, 4, 2, 6, 5, 8, 0, 9, 7
            };

    matrix.resize(
            (natural_32_bit)static_state_ptr->num_kinds_of_tissue_cells() *
            (natural_32_bit)static_state_ptr->num_kinds_of_cells()
            );
    INVARIANT(matrix.size() > 0U);
    std::fill(matrix.begin(),matrix.end(),0U);

    for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
    {
        natural_64_bit const TiNi =
                (natural_64_bit)static_state_ptr->num_synapses_in_territory_of_cell_kind(i) *
                (natural_64_bit)static_state_ptr->num_tissue_cells_of_cell_kind(i);
        INVARIANT(TiNi > 0ULL);

        natural_32_bit const row_shift = i * (natural_32_bit)static_state_ptr->num_kinds_of_cells();

        natural_64_bit SUM = 0ULL;
        for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
        {
            matrix.at(row_shift + j) =  1U + (natural_32_bit)(multipliers.at((i + j) % multipliers.size()) * TiNi) /
                                             ( (natural_64_bit)static_state_ptr->num_kinds_of_cells() *
                                               (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j) );
            SUM += (natural_64_bit)matrix.at(row_shift + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
        }
        INVARIANT(SUM >= static_state_ptr->num_kinds_of_cells());
        for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
            matrix.at(row_shift + j) = (float_64_bit)matrix.at(row_shift + j) * (float_64_bit)TiNi / (float_64_bit)SUM;
        SUM = 0ULL;
        for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
            SUM += (natural_64_bit)matrix.at(row_shift + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
        for (cellab::kind_of_cell j = 0U; SUM > TiNi && j < static_state_ptr->num_kinds_of_cells(); j = (j+1) % static_state_ptr->num_kinds_of_cells())
            if (matrix.at(row_shift + j) > 0U)
            {
                --matrix.at(row_shift + j);
                SUM -= (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
            }
        INVARIANT(SUM <= TiNi);
        matrix.at(row_shift + static_state_ptr->num_kinds_of_cells() - 1U) += (natural_32_bit)(TiNi - SUM);
        SUM = 0ULL;
        for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
            SUM += (natural_64_bit)matrix.at(row_shift + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
        INVARIANT(SUM == TiNi);

        std::rotate(multipliers.begin(), multipliers.begin() + 1, multipliers.end());
    }
}


static void test_column(
        natural_32_bit const x,
        natural_32_bit const y,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_state_ptr,
        std::vector<natural_32_bit> const& matrix
        )
{
    TMPROF_BLOCK();

    std::vector<natural_64_bit> inferred_matrix(
                (natural_32_bit)static_state_ptr->num_kinds_of_tissue_cells() *
                (natural_32_bit)static_state_ptr->num_kinds_of_cells(),
                0ULL
                );
    INVARIANT(inferred_matrix.size() == matrix.size());

    for (natural_32_bit c = 0U; c < static_state_ptr->num_cells_along_columnar_axis(); ++c)
        for (natural_32_bit s = 0U; s < static_state_ptr->num_synapses_in_territory_of_cell_with_columnar_coord(c); ++s)
        {
            cellab::tissue_coordinates const coords =
                    cellab::get_coordinates_of_source_cell_of_synapse_in_tissue(
                            dynamic_state_ptr,
                            cellab::tissue_coordinates(x,y,c),
                            s
                            );
            TEST_SUCCESS(coords.get_coord_along_x_axis() == x &&
                         coords.get_coord_along_y_axis() == y &&
                         coords.get_coord_along_columnar_axis() < static_state_ptr->num_cells_along_columnar_axis() + static_state_ptr->num_sensory_cells());

            ++inferred_matrix.at(
                    (natural_32_bit)static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(c) *
                    (natural_32_bit)static_state_ptr->num_kinds_of_cells() +
                    (natural_32_bit)static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(coords.get_coord_along_columnar_axis())
                    );
        }

    for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
        for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
            TEST_SUCCESS(
                    (natural_64_bit)matrix.at(i * static_state_ptr->num_kinds_of_cells() + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j)
                        == inferred_matrix.at(i * static_state_ptr->num_kinds_of_cells() + j)
                    );
}


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

                    std::vector<natural_32_bit> matrix;
                    build_matrix(static_tissue,matrix);

                    for (natural_32_bit num_threads = 0U; num_threads <= 16; num_threads += 8)
                    {
                        cellab::tissue_coordinates const error_coords(
                                    static_tissue->num_cells_along_x_axis(),
                                    static_tissue->num_cells_along_y_axis(),
                                    static_tissue->num_cells_along_columnar_axis() + static_tissue->num_sensory_cells()
                                    );
                        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
                            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                            {
                                for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
                                    for (natural_32_bit s = 0U; s < static_tissue->num_synapses_in_territory_of_cell_with_columnar_coord(c); ++s)
                                    {
                                        bits_reference bits_of_coords =
                                                dynamic_tissue->find_bits_of_coords_of_source_cell_of_synapse_in_tissue(x,y,c,s);
                                        cellab::write_tissue_coordinates_to_bits_of_coordinates(error_coords,bits_of_coords);
                                    }
                                TEST_PROGRESS_UPDATE();
                            }

                        cellconnect::fill_coords_of_source_cells_of_synapses_in_tissue(
                                    dynamic_tissue,
                                    matrix,
                                    std::max(1U,num_threads)
                                    );

                        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
                            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                            {
                                test_column(x,y,static_tissue,dynamic_tissue,matrix);
                                TEST_PROGRESS_UPDATE();
                            }
                    }

                    std::rotate(coefs.begin(), coefs.begin() + 1, coefs.end());
                }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
