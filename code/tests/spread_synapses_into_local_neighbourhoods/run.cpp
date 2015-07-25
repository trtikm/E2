#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <cellconnect/spread_synapses_into_local_neighbourhoods.hpp>
#include <utility/test.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <algorithm>
#include <limits>


#include <utility/development.hpp>


static void build_fill_src_coords_matrix(
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


static void  build_spread_synapses_matrix(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        natural_64_bit const  total_num_synapses,
        std::vector<natural_32_bit>&  matrix
        )
{
    TMPROF_BLOCK();

    ASSUMPTION((natural_64_bit)diameter_x * (natural_64_bit)diameter_y < (natural_64_bit)std::numeric_limits<natural_32_bit>::max());
    ASSUMPTION(diameter_x * diameter_y > 2ULL && diameter_x % 2U == 1U && diameter_y % 2U == 1U);
    ASSUMPTION(target_kind < static_state_ptr->num_kinds_of_tissue_cells());
    ASSUMPTION(source_kind < static_state_ptr->num_kinds_of_cells());
    ASSUMPTION(total_num_synapses > 0ULL);

    static std::vector<natural_32_bit> multipliers = {
            1, 8, 5, 3, 2, 4, 0, 9, 6, 1, 7, 3, 4, 2, 6, 5, 8, 0, 9, 7
            };
    matrix.resize(diameter_x * diameter_y);
    std::fill(matrix.begin(),matrix.end(),0U);

    natural_32_bit const  unit_count = (natural_32_bit)(1ULL + total_num_synapses / (10ULL * (natural_64_bit)(diameter_x * diameter_y)));

    natural_64_bit  num_synapses_to_distribute = total_num_synapses;
    for (natural_32_bit  i = (unit_count + target_kind + source_kind) % matrix.size(),
                         j = (unit_count + target_kind + source_kind) % multipliers.size();
         num_synapses_to_distribute > 0ULL;
         i = (i + 1U) % matrix.size(), j = (j + 1U) % multipliers.size())
    {
        natural_32_bit  addon = multipliers.at(j) * unit_count;
        if ((natural_64_bit)addon > num_synapses_to_distribute)
            addon = num_synapses_to_distribute;
        matrix.at(i) += addon;
        num_synapses_to_distribute -= addon;
    }

    INVARIANT(
            [](natural_64_bit const  total_num_synapses, std::vector<natural_32_bit> const&  matrix) {
                natural_64_bit  sum = 0ULL;
                for (natural_32_bit count : matrix)
                    sum += count;
                return sum == total_num_synapses;
            }(total_num_synapses,matrix)
            );
}


static void test_column(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  x,
        natural_32_bit const  y,
        cellab::kind_of_cell const  target_kind,
        cellab::kind_of_cell const  source_kind,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        std::vector<natural_32_bit> const&  matrix
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(x < static_state_ptr->num_cells_along_x_axis());
    ASSUMPTION(y < static_state_ptr->num_cells_along_y_axis());
    ASSUMPTION(target_kind < static_state_ptr->num_kinds_of_tissue_cells());
    ASSUMPTION(source_kind < static_state_ptr->num_kinds_of_cells());
    ASSUMPTION(diameter_x * diameter_y > 2U && diameter_x % 2U == 1U && diameter_y % 2U == 1U);
    ASSUMPTION(matrix.size() == diameter_x * diameter_y);

    natural_32_bit const  center_x = diameter_x / 2U;
    natural_32_bit const  center_y = diameter_y / 2U;
    natural_32_bit const  c0 = static_state_ptr->compute_columnar_coord_of_first_tissue_cell_of_kind(target_kind);

    natural_32_bit  num_center_synapses = 0U;
    natural_32_bit  num_skipped_synapses = 0U;

    for (natural_32_bit  i = 0U; i < diameter_x; ++i)
        for (natural_32_bit  j = 0U; j < diameter_y; ++j)
        {
            natural_32_bit const  shifted_x =
                    cellab::shift_coordinate(
                            x,
                            (integer_64_bit)i - (integer_64_bit)center_x,
                            static_state_ptr->num_cells_along_x_axis(),
                            static_state_ptr->is_x_axis_torus_axis()
                            );
            natural_32_bit const  shifted_y =
                    cellab::shift_coordinate(
                            y,
                            (integer_64_bit)j - (integer_64_bit)center_y,
                            static_state_ptr->num_cells_along_y_axis(),
                            static_state_ptr->is_y_axis_torus_axis()
                            );
            if (shifted_x == static_state_ptr->num_cells_along_x_axis() ||
                shifted_y == static_state_ptr->num_cells_along_y_axis())
            {
                natural_32_bit const  oposite_i = (2ULL * (integer_64_bit)center_x - (integer_64_bit)i) % diameter_x;
                natural_32_bit const  oposite_j = (2ULL * (integer_64_bit)center_y - (integer_64_bit)j) % diameter_y;
                num_skipped_synapses += matrix.at(oposite_j * diameter_x + oposite_i);
                continue;
            }

            natural_32_bit num_synapses = 0U;

            for (natural_32_bit k = 0U; k < static_state_ptr->num_cells_of_cell_kind(target_kind); ++k)
                for (natural_32_bit l = 0U; l < static_state_ptr->num_synapses_in_territory_of_cell_kind(target_kind); ++l)
                {
                    cellab::tissue_coordinates const  coords =
                            cellab::get_coordinates_of_source_cell_of_synapse_in_tissue(
                                        dynamic_state_ptr,
                                        cellab::tissue_coordinates(shifted_x, shifted_y, c0 + k),
                                        l
                                        );
                    if (static_state_ptr->compute_kind_of_cell_from_its_position_along_columnar_axis(coords.get_coord_along_columnar_axis()) == source_kind &&
                            coords.get_coord_along_x_axis() == x &&
                            coords.get_coord_along_y_axis() == y )
                        ++num_synapses;
                }

            if (i == center_x && j == center_y)
                num_center_synapses = num_synapses;
            else
                TEST_SUCCESS(matrix.at(j * diameter_x + i) == num_synapses);
        }

    TEST_SUCCESS(matrix.at(center_y * diameter_x + center_x) + num_skipped_synapses == num_center_synapses);
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector<natural_32_bit> coefs = { 1, 4, 3, 8, 1, 9, 2, 5, 4, 2, 7, 6, 6, 5, 8, 9, 3 };

    for (natural_32_bit cells_x = 1U; cells_x <= 11U; cells_x += 5U)
        for (natural_32_bit cells_y = 2U; cells_y <= 12U; cells_y += 5U)
            for (natural_16_bit tissue_cell_kinds = 2U; tissue_cell_kinds <= 12U; tissue_cell_kinds += 5)
                for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds <= 11U; sensory_cell_kinds += 5)
                    for (bool is_torus_axis_x : std::vector<bool>{ true, false })
                        for (bool is_torus_axis_y : std::vector<bool>{ true, false })
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

                            std::rotate(coefs.begin(), coefs.begin() + 1, coefs.end());

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
                                                is_torus_axis_x,
                                                is_torus_axis_y,
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

                            std::vector<natural_32_bit> matrix_fill_src_coords;
                            build_fill_src_coords_matrix(static_tissue,matrix_fill_src_coords);

                            for (natural_32_bit  diameter_x : std::vector<natural_32_bit>{1, 5})
                            {
                                if (diameter_x >= static_tissue->num_cells_along_x_axis())
                                    continue;

                                for (natural_32_bit  diameter_y : std::vector<natural_32_bit>{3, 7})
                                {
                                    if (diameter_y >= static_tissue->num_cells_along_y_axis())
                                        continue;

                                    for (natural_32_bit num_threads : std::vector<natural_32_bit>{1, 64})
                                    {
                                        cellconnect::fill_coords_of_source_cells_of_synapses_in_tissue(
                                                    dynamic_tissue,
                                                    matrix_fill_src_coords,
                                                    num_threads
                                                    );
                                        TEST_PROGRESS_UPDATE();

                                        for (cellab::kind_of_cell  source_kind = 0U; source_kind < static_tissue->num_kinds_of_cells(); ++source_kind)
                                            for (cellab::kind_of_cell  target_kind = 0U; target_kind < static_tissue->num_kinds_of_tissue_cells(); ++target_kind)
                                            {
                                                natural_64_bit const  total_synapses_count =
                                                        (natural_64_bit)matrix_fill_src_coords.at(target_kind * static_tissue->num_kinds_of_cells() + source_kind) *
                                                        (natural_64_bit)static_tissue->num_cells_of_cell_kind(source_kind)
                                                        ;
                                                if (total_synapses_count == 0ULL)
                                                    continue;

                                                std::vector<natural_32_bit> matrix_spread_synapses;
                                                build_spread_synapses_matrix(
                                                            static_tissue,
                                                            target_kind,
                                                            source_kind,
                                                            diameter_x,
                                                            diameter_y,
                                                            total_synapses_count,
                                                            matrix_spread_synapses
                                                            );

                                                cellconnect::spread_synapses_into_local_neighbourhoods(
                                                            dynamic_tissue,
                                                            target_kind,
                                                            source_kind,
                                                            diameter_x,
                                                            diameter_y,
                                                            matrix_spread_synapses,
                                                            num_threads
                                                            );
                                                TEST_PROGRESS_UPDATE();
                                            }

                                        for (cellab::kind_of_cell  source_kind = 0U; source_kind < static_tissue->num_kinds_of_cells(); ++source_kind)
                                            for (cellab::kind_of_cell  target_kind = 0U; target_kind < static_tissue->num_kinds_of_tissue_cells(); ++target_kind)
                                            {
                                                natural_64_bit const  total_synapses_count =
                                                        (natural_64_bit)matrix_fill_src_coords.at(target_kind * static_tissue->num_kinds_of_cells() + source_kind) *
                                                        (natural_64_bit)static_tissue->num_cells_of_cell_kind(source_kind)
                                                        ;
                                                if (total_synapses_count == 0ULL)
                                                    continue;

                                                std::vector<natural_32_bit> matrix_spread_synapses;
                                                build_spread_synapses_matrix(
                                                            static_tissue,
                                                            target_kind,
                                                            source_kind,
                                                            diameter_x,
                                                            diameter_y,
                                                            total_synapses_count,
                                                            matrix_spread_synapses
                                                            );

                                                for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
                                                    for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                                                    {
                                                        test_column(
                                                                static_tissue,
                                                                dynamic_tissue,
                                                                x,y,
                                                                target_kind,
                                                                source_kind,
                                                                diameter_x,
                                                                diameter_y,
                                                                matrix_spread_synapses
                                                                );
                                                        TEST_PROGRESS_UPDATE();
                                                    }
                                            }
                                    }
                                }
                            }
                }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
