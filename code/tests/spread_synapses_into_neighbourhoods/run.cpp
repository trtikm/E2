#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <cellconnect/spread_synapses_into_neighbourhoods.hpp>
#include <cellconnect/column_shift_function.hpp>
#include <utility/test.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <algorithm>
#include <tuple>
#include <limits>
#include <memory>


static void build_fill_src_coords_matrix(
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
    std::vector<natural_32_bit>&  matrix
    )
{
    TMPROF_BLOCK();

    ASSUMPTION(static_state_ptr->num_cells_of_cell_kind(static_state_ptr->num_kinds_of_cells() - 1U) == 1U);

    matrix.resize(
        (natural_32_bit)static_state_ptr->num_kinds_of_tissue_cells() *
        (natural_32_bit)static_state_ptr->num_kinds_of_cells()
        );
    INVARIANT(matrix.size() > 0U);
    std::fill(matrix.begin(), matrix.end(), 0U);

    for (cellab::kind_of_cell i = 0U; i < static_state_ptr->num_kinds_of_tissue_cells(); ++i)
    {
        natural_32_bit const row_shift = i * (natural_32_bit)static_state_ptr->num_kinds_of_cells();
        natural_32_bit const last_column_index = static_state_ptr->num_kinds_of_cells() - 1U;

        matrix.at(row_shift + last_column_index) = static_state_ptr->num_synapses_in_territory_of_cell_kind(i) *
            static_state_ptr->num_tissue_cells_of_cell_kind(i);

        natural_32_bit num_skipped = 0U;
        cellab::kind_of_cell j = 0U;
        do
        {
            if (static_state_ptr->num_cells_of_cell_kind(j) <= matrix.at(row_shift + last_column_index))
            {
                ++matrix.at(row_shift + j);
                matrix.at(row_shift + last_column_index) -= static_state_ptr->num_cells_of_cell_kind(j);
                num_skipped = 0U;
            }
            else
                ++num_skipped;

            ++j;
            if (j == last_column_index)
                j = 0U;
        } while (matrix.at(row_shift + last_column_index) > 0U && num_skipped < static_state_ptr->num_kinds_of_cells() - 1U);

        INVARIANT(
            static_state_ptr->num_synapses_in_territory_of_cell_kind(i) * static_state_ptr->num_tissue_cells_of_cell_kind(i) ==
            [](std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
                std::vector<natural_32_bit> const&  matrix, natural_32_bit const row_shift) {
            natural_64_bit  SUM = 0ULL;
            for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
                SUM += (natural_64_bit)matrix.at(row_shift + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
            return SUM;
        }(static_state_ptr, matrix, row_shift)
            );
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
            addon = (natural_32_bit)num_synapses_to_distribute;
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


static std::shared_ptr<cellconnect::column_shift_function const>  build_shift_function(
        natural_32_bit const  num_tissue_cells_along_axis_x,
        natural_32_bit const  num_tissue_cells_along_axis_y,
        natural_16_bit const  largest_template_dim_x,
        natural_16_bit const  largest_template_dim_y,
        natural_32_bit const  scale_of_templates_x,
        natural_32_bit const  scale_of_templates_y,
        natural_16_bit const  template_rep_x,
        natural_16_bit const  template_rep_y
        )
{
    TMPROF_BLOCK();

    natural_16_bit const  num_exists = (largest_template_dim_x * largest_template_dim_y / 2U) / 8U;

    std::vector<cellconnect::shift_template>  shift_templates = {
        // left-top
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_DOWN, num_exists },
              { cellconnect::DIR_RIGHT_DOWN, num_exists },
              { cellconnect::DIR_RIGHT, num_exists },
          }
        },

        // left-middle
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_DOWN, num_exists },
              { cellconnect::DIR_RIGHT_DOWN, num_exists },
              { cellconnect::DIR_RIGHT, num_exists },
              { cellconnect::DIR_RIGHT_UP, num_exists },
              { cellconnect::DIR_UP, num_exists },
          }
        },

        // left-bottom
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_RIGHT, num_exists },
              { cellconnect::DIR_RIGHT_UP, num_exists },
              { cellconnect::DIR_UP, num_exists },
          }
        },

        // middle-top
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT, num_exists },
              { cellconnect::DIR_LEFT_DOWN, num_exists },
              { cellconnect::DIR_DOWN, num_exists },
              { cellconnect::DIR_RIGHT_DOWN, num_exists },
              { cellconnect::DIR_RIGHT, num_exists },
          }
        },

        // middle-middle
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, num_exists },
              { cellconnect::DIR_LEFT, num_exists },
              { cellconnect::DIR_LEFT_DOWN, num_exists },
              { cellconnect::DIR_DOWN, num_exists },
              { cellconnect::DIR_RIGHT_DOWN, num_exists },
              { cellconnect::DIR_RIGHT, num_exists },
              { cellconnect::DIR_RIGHT_UP, num_exists },
              { cellconnect::DIR_UP, num_exists },
          }
        },

        // middle-bottom
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_RIGHT, num_exists },
              { cellconnect::DIR_RIGHT_UP, num_exists },
              { cellconnect::DIR_UP, num_exists },
              { cellconnect::DIR_LEFT_UP, num_exists },
              { cellconnect::DIR_LEFT, num_exists },
          }
        },

        // right-top
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT, num_exists },
              { cellconnect::DIR_LEFT_DOWN, num_exists },
              { cellconnect::DIR_DOWN, num_exists },
          }
        },

        // right-middle
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, num_exists },
              { cellconnect::DIR_LEFT, num_exists },
              { cellconnect::DIR_LEFT_DOWN, num_exists },
              { cellconnect::DIR_DOWN, num_exists },
              { cellconnect::DIR_UP, num_exists },
          }
        },

        // right-bottom
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, num_exists },
              { cellconnect::DIR_LEFT, num_exists },
              { cellconnect::DIR_UP, num_exists },
          }
        }
    };

    std::shared_ptr<cellconnect::column_shift_function const>  shift_fn {
            new cellconnect::column_shift_function {
                    num_tissue_cells_along_axis_x,
                    num_tissue_cells_along_axis_y,
                    shift_templates,
                    scale_of_templates_x,
                    scale_of_templates_y,
                    {
                        3U, 3U,
                        {
                            0, 3, 6,
                            1, 4, 7,
                            2, 5, 8
                        }
                    },
                    { {1U,2U,(natural_16_bit)(template_rep_x - 2U)}, },
                    { {1U,2U,(natural_16_bit)(template_rep_y - 2U)}, }
            }
    };

    return shift_fn;
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
        std::vector<natural_32_bit> const&  matrix,
        std::shared_ptr<cellconnect::column_shift_function const> const  shift_fn
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

    natural_32_bit  num_self_synapses = 0U;
    natural_32_bit  num_self_in_matrix = 0U;
    natural_32_bit  num_skipped_synapses = 0U;

    natural_32_bit  target_x, target_y;
    std::tie(target_x,target_y) = shift_fn->operator()(x,y);
    TEST_SUCCESS(target_x < static_state_ptr->num_cells_along_x_axis());
    TEST_SUCCESS(target_y < static_state_ptr->num_cells_along_y_axis());

    natural_32_bit  progress_counter = 0U;
    for (natural_32_bit  i = 0U; i < diameter_x; ++i)
        for (natural_32_bit  j = 0U; j < diameter_y; ++j)
        {
            natural_32_bit const  shifted_x =
                    cellab::shift_coordinate(
                            target_x,
                            (integer_64_bit)i - (integer_64_bit)center_x,
                            static_state_ptr->num_cells_along_x_axis(),
                            static_state_ptr->is_x_axis_torus_axis()
                            );
            natural_32_bit const  shifted_y =
                    cellab::shift_coordinate(
                            target_y,
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

            if (x == shifted_x && y == shifted_y)
            {
                TEST_SUCCESS(!shift_fn->is_identity_function() || (i == center_x && j == center_y));
                num_self_synapses = num_synapses;
                num_self_in_matrix = matrix.at(j * diameter_x + i);
            }
            else
            {
                TEST_SUCCESS(!shift_fn->is_identity_function() || !(i == center_x && j == center_y));
                TEST_SUCCESS(matrix.at(j * diameter_x + i) == num_synapses);
            }

            if (progress_counter % 100U == 0U)
            {
                TEST_PROGRESS_UPDATE();
                progress_counter = 0U;
            }
        }

    TEST_SUCCESS(!shift_fn->is_identity_function() || num_self_in_matrix + num_skipped_synapses == num_self_synapses);
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector<natural_32_bit> coefs = { 1, 4, 3, 8, 1, 9, 2, 5, 4, 2, 7, 6, 6, 5, 8, 9, 3 };

    natural_16_bit const  largest_template_dim_x = 3U;
    natural_16_bit const  largest_template_dim_y = 3U;

    typedef std::tuple<natural_32_bit,  // num cells x
                       natural_32_bit,  // num cells y
                       natural_16_bit,  // num tissue cell kinds
                       natural_16_bit,  // num sensory cell kinds
                       bool,            // is torus axis x
                       bool,            // is torus axis y
                       natural_32_bit,  // num template repetitions x
                       natural_32_bit>  // num template repetitions y
            tissue_props;
    for (tissue_props props :
         std::vector<tissue_props>{
                tissue_props{ 3U, 3U, 2U, 1U, true, true, 1U, 1U },
                tissue_props{ 10U, 15U, 4U, 3U, false, true, 3U, 5U },
                tissue_props{ 13U, 8U, 5U, 6U, true, false, 4U, 3U },
                tissue_props{ 12U, 12U, 6U, 7U, true, true, 4U, 3U },
                })
    {
        natural_16_bit const  template_rep_x = std::get<6>(props);
        natural_16_bit const  template_rep_y = std::get<7>(props);

        natural_32_bit  cells_x;
        natural_32_bit  scale_of_templates_x;
        cellconnect::compute_tissue_axis_length_and_template_scale(
                    std::get<0>(props),
                    largest_template_dim_x,
                    template_rep_x,
                    cells_x,
                    scale_of_templates_x
                    );

        natural_32_bit  cells_y;
        natural_32_bit  scale_of_templates_y;
        cellconnect::compute_tissue_axis_length_and_template_scale(
                    std::get<1>(props),
                    largest_template_dim_y,
                    template_rep_y,
                    cells_y,
                    scale_of_templates_y
                    );

        natural_16_bit const  tissue_cell_kinds = std::get<2>(props);
        natural_16_bit const  sensory_cell_kinds = std::get<3>(props);

        bool const  is_torus_axis_x = std::get<4>(props);
        bool const  is_torus_axis_y = std::get<5>(props);

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


        typedef std::tuple<natural_32_bit,natural_32_bit> diameters_type;
        for (diameters_type diameters :
             std::vector<diameters_type>{diameters_type{1, 3},diameters_type{7, 5}})
        {
            natural_32_bit const  diameter_x = std::get<0>(diameters);
            if (diameter_x >= static_tissue->num_cells_along_x_axis())
                continue;

            natural_32_bit const  diameter_y = std::get<1>(diameters);
            if (diameter_y >= static_tissue->num_cells_along_y_axis())
                continue;

            for (natural_32_bit num_threads : std::vector<natural_32_bit>{1, 16})
            {
                cellconnect::fill_coords_of_source_cells_of_synapses_in_tissue(
                            dynamic_tissue,
                            matrix_fill_src_coords,
                            num_threads
                            );
                TEST_PROGRESS_UPDATE();

                std::vector< std::shared_ptr<cellconnect::column_shift_function const> >  shift_fns;

                natural_32_bit  shift_fn_kind = 0U;
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

                        shift_fns.push_back(
                                    shift_fn_kind == 0 ?
                                            std::shared_ptr<cellconnect::column_shift_function const>(new cellconnect::column_shift_function()) :
                                            build_shift_function(
                                                    cells_x,
                                                    cells_y,
                                                    largest_template_dim_x,
                                                    largest_template_dim_y,
                                                    scale_of_templates_x,
                                                    scale_of_templates_y,
                                                    template_rep_x,
                                                    template_rep_y
                                                    )
                                    );
                        shift_fn_kind = (shift_fn_kind + 1U) % 2U;

                        TEST_SUCCESS(
                                cellconnect::check_consistency_of_matrix_and_tissue(
                                        dynamic_tissue,
                                        target_kind,
                                        source_kind,
                                        matrix_spread_synapses,
                                        num_threads
                                        )
                                );

                        cellconnect::spread_synapses_into_neighbourhoods(
                                    dynamic_tissue,
                                    target_kind,
                                    source_kind,
                                    diameter_x,
                                    diameter_y,
                                    matrix_spread_synapses,
                                    *shift_fns.back(),
                                    num_threads
                                    );

                        TEST_PROGRESS_UPDATE();
                    }

                natural_32_bit  shift_fn_index = 0U;
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
                                        matrix_spread_synapses,
                                        shift_fns.at(shift_fn_index)
                                        );
                                TEST_PROGRESS_UPDATE();
                            }

                        ++shift_fn_index;
                    }
            }
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
