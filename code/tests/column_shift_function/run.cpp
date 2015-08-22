#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/column_shift_function.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>
#include <functional>
#include <vector>
#include <string>
#include <sstream>


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector< std::pair<natural_32_bit,natural_32_bit> > const  tissue_dimensions = {
        { 100U, 100U },
        { 500U, 500U },
        { 100U, 500U },
        { 50U, 200U },
        { 225U, 210U },
        { 75U, 25U },
        { 150U, 125U },
    };

    std::vector<natural_16_bit> const  central_dimension = {
        20U, 10U, 9U, 8U, 7U, 6U , 5U, 4U, 3U, 2U
    };

    std::vector<natural_8_bit> const  central_repetitions = {
        10U, 9U, 8U, 7U, 6U , 5U
    };

    for (auto const&  tissue_dim : tissue_dimensions)
        for (auto const  central_dim_x : central_dimension)
            for (auto const  central_dim_y : central_dimension)
                for (auto const  central_rep_x : central_repetitions)
                    for (auto const  central_rep_y : central_repetitions)
                    {
                        natural_32_bit  scale_of_all_templates_x;
                        natural_16_bit  dimension_of_the_front_template_x;
                        natural_8_bit  num_repetitions_of_the_front_template_x;
                        natural_16_bit  dimension_of_the_tail_template_x;
                        natural_8_bit  num_repetitions_of_the_tail_template_x;
                        if (!cellconnect::compute_dimestions_repetitions_and_scales_for_templates_along_one_axis_of_tissue(
                                    tissue_dim.first,
                                    central_dim_x,
                                    central_rep_x,
                                    scale_of_all_templates_x,
                                    dimension_of_the_front_template_x,
                                    num_repetitions_of_the_front_template_x,
                                    dimension_of_the_tail_template_x,
                                    num_repetitions_of_the_tail_template_x
                                    ))
                        {
                            LOG(info,"");
                            continue;
                        }
                        TEST_SUCCESS(tissue_dim.first == scale_of_all_templates_x * (
                                         (natural_32_bit)central_dim_x * (natural_32_bit)central_rep_x +
                                         (natural_32_bit)dimension_of_the_front_template_x * (natural_32_bit)num_repetitions_of_the_front_template_x +
                                         (natural_32_bit)dimension_of_the_tail_template_x * (natural_32_bit)num_repetitions_of_the_tail_template_x)
                                     );

                        natural_32_bit  scale_of_all_templates_y;
                        natural_16_bit  dimension_of_the_front_template_y;
                        natural_8_bit  num_repetitions_of_the_front_template_y;
                        natural_16_bit  dimension_of_the_tail_template_y;
                        natural_8_bit  num_repetitions_of_the_tail_template_y;
                        if (!cellconnect::compute_dimestions_repetitions_and_scales_for_templates_along_one_axis_of_tissue(
                                    tissue_dim.second,
                                    central_dim_y,
                                    central_rep_y,
                                    scale_of_all_templates_y,
                                    dimension_of_the_front_template_y,
                                    num_repetitions_of_the_front_template_y,
                                    dimension_of_the_tail_template_y,
                                    num_repetitions_of_the_tail_template_y
                                    ))
                        {
                            LOG(info,"");
                            continue;
                        }
                        TEST_SUCCESS(tissue_dim.second == scale_of_all_templates_y * (
                                         (natural_32_bit)central_dim_y * (natural_32_bit)central_rep_y +
                                         (natural_32_bit)dimension_of_the_front_template_y * (natural_32_bit)num_repetitions_of_the_front_template_y +
                                         (natural_32_bit)dimension_of_the_tail_template_y * (natural_32_bit)num_repetitions_of_the_tail_template_y)
                                     );
                    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
