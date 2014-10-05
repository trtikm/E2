#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellab/static_state_of_neural_tissue.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <memory>
#include <stdexcept>

static void test_compute_kind_of_cell_from_its_position_along_columnar_axis(
        std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    cellab::kind_of_cell right_kind = 0U;
    natural_32_bit c_break = static_tissue->num_tissue_cells_of_cell_kind(right_kind);
    for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
    {
        if (c == c_break)
        {
            ++right_kind;
            c_break += static_tissue->num_tissue_cells_of_cell_kind(right_kind);
        }
        cellab::kind_of_cell kind =
                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c);

        TEST_SUCCESS(right_kind == kind);
    }
    TEST_SUCCESS(right_kind+1U == static_tissue->lowest_kind_of_sensory_cells());
    TEST_SUCCESS(c_break == static_tissue->num_cells_along_columnar_axis());
    ++right_kind;
    c_break += static_tissue->num_sensory_cells_of_cell_kind(right_kind);
    for (natural_32_bit c = 0U; c < static_tissue->num_sensory_cells(); ++c)
    {
        if (static_tissue->num_cells_along_columnar_axis() + c == c_break)
        {
            ++right_kind;
            c_break += static_tissue->num_sensory_cells_of_cell_kind(right_kind);
        }
        cellab::kind_of_cell kind =
                static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(
                    static_tissue->num_cells_along_columnar_axis() + c);

        TEST_SUCCESS(right_kind == kind);
    }
}

static void test_compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(
        std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    // TODO!
}

static void test_compute_kind_of_sensory_cell_from_its_index(
        std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    // TODO!
}

static void test_compute_kind_of_sensory_cell_and_relative_index_from_its_index(
        std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    // TODO!
}

static void test_compute_index_of_first_sensory_cell_of_kind(
        std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    // TODO!
}

static void test_find_bits_of_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_cell_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_territorial_state_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_coords_of_source_cell_of_synapse_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_signalling(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_delimiter_between_teritorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_sensory_cell(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_find_bits_of_synapse_to_muscle(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    // TODO!
}

static void test_static_state(std::shared_ptr<cellab::static_state_of_neural_tissue> const static_tissue)
{
    test_compute_kind_of_cell_from_its_position_along_columnar_axis(static_tissue);
    test_compute_kind_of_cell_and_relative_columnar_index_from_coordinate_along_columnar_axis(static_tissue);
    test_compute_kind_of_sensory_cell_from_its_index(static_tissue);
    test_compute_kind_of_sensory_cell_and_relative_index_from_its_index(static_tissue);
    test_compute_index_of_first_sensory_cell_of_kind(static_tissue);
}

static void test_dynamic_state(std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const dynamic_tissue)
{
    test_find_bits_of_cell(dynamic_tissue);
    test_find_bits_of_cell_in_tissue(dynamic_tissue);
    test_find_bits_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_territorial_state_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_coords_of_source_cell_of_synapse_in_tissue(dynamic_tissue);
    test_find_bits_of_signalling(dynamic_tissue);
    test_find_bits_of_delimiter_between_teritorial_lists(dynamic_tissue);
    test_find_bits_of_sensory_cell(dynamic_tissue);
    test_find_bits_of_synapse_to_muscle(dynamic_tissue);
}

void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    for (natural_16_bit tissue_cell_kinds = 1U; tissue_cell_kinds < 11U; ++tissue_cell_kinds)
    {
        for (natural_16_bit sensory_cell_kinds = 1U; sensory_cell_kinds < 3U; ++sensory_cell_kinds)
        {
            natural_16_bit const num_bits_per_cell = 1U;
            natural_16_bit const num_bits_per_synapse = 1U;
            natural_16_bit const num_bits_per_signalling = 1U;

            for (natural_32_bit cells_x = 1U; cells_x < 10000U; cells_x += 500U)
            {
                for (natural_32_bit cells_y = 1U; cells_y < 10000U; cells_y += 500U)
                {
                    std::vector<natural_32_bit> num_tissue_cells_of_cell_kind;
                    std::vector<natural_32_bit> num_synapses_in_territory_of_cell_kind;
                    for (natural_16_bit i = 1U; i <= tissue_cell_kinds; ++i)
                    {
                        num_tissue_cells_of_cell_kind.push_back(i);
                        num_synapses_in_territory_of_cell_kind.push_back(10U+i);
                    }

                    std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
                    for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
                        num_sensory_cells_of_cell_kind.push_back((i%2 == 0) ? 1U : 2U);

                    natural_32_bit const num_synapses_to_muscles = 5U;

                    for (natural_8_bit torus_x = 0U; torus_x < 2U; ++torus_x)
                        for (natural_8_bit torus_y = 0U; torus_y < 2U; ++torus_y)
                            for (natural_8_bit torus_c = 0U; torus_c < 2U; ++torus_c)
                            {
                                bool const is_x_axis_torus_axis = torus_x == 1;
                                bool const is_y_axis_torus_axis = torus_y == 1;
                                bool const is_c_axis_torus_axis = torus_c == 1;

                                for (natural_8_bit radius = 1U; radius < 3U; ++radius)
                                {
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_cell;
                                    std::vector<integer_8_bit> x_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> y_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> columnar_radius_of_signalling_neighbourhood_of_synapse;
                                    std::vector<integer_8_bit> x_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> y_radius_of_cellular_neighbourhood_of_signalling;
                                    std::vector<integer_8_bit> columnar_radius_of_cellular_neighbourhood_of_signalling;
                                    for (natural_16_bit i = 0U; i < tissue_cell_kinds; ++i)
                                    {
                                        x_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        y_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        columnar_radius_of_signalling_neighbourhood_of_cell.push_back(radius);
                                        x_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        y_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        columnar_radius_of_signalling_neighbourhood_of_synapse.push_back(radius);
                                        x_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                        y_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                        columnar_radius_of_cellular_neighbourhood_of_signalling.push_back(radius);
                                    }

                                    std::shared_ptr<cellab::static_state_of_neural_tissue> static_tissue;
                                    try
                                    {
                                        static_tissue = std::shared_ptr<cellab::static_state_of_neural_tissue>(
                                                    new cellab::static_state_of_neural_tissue(
                                                        tissue_cell_kinds,
                                                        sensory_cell_kinds,
                                                        num_bits_per_cell,
                                                        num_bits_per_synapse,
                                                        num_bits_per_signalling,
                                                        cells_x,
                                                        cells_y,
                                                        num_tissue_cells_of_cell_kind,
                                                        num_synapses_in_territory_of_cell_kind,
                                                        num_sensory_cells_of_cell_kind,
                                                        num_synapses_to_muscles,
                                                        is_x_axis_torus_axis,
                                                        is_y_axis_torus_axis,
                                                        is_c_axis_torus_axis,
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
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create static state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }
                                    test_static_state(static_tissue);

                                    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_tissue;
                                    try
                                    {
                                        dynamic_tissue = std::shared_ptr<cellab::dynamic_state_of_neural_tissue>(
                                                    new cellab::dynamic_state_of_neural_tissue(static_tissue)
                                                    );
                                    }
                                    catch(...)
                                    {
//                                        LOG(testing,"Cannot create dynamic state of tissue : "
//                                                    "num_kinds_of_tissue_cells = " << tissue_cell_kinds << ",   " <<
//                                                    "num_kinds_of_sensory_cells = " << sensory_cell_kinds << ",   " <<
//                                                    "num_cells_along_x_axis = " << cells_x << ",   " <<
//                                                    "num_cells_along_y_axis = " << cells_y << ",   " <<
//                                                    "radius = " << (natural_32_bit)radius
//                                                    );
                                        continue;
                                    }
                                    test_dynamic_state(dynamic_tissue);
                                }
                            }

                    TEST_PROGRESS_UPDATE();
                }
            }
        }
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
