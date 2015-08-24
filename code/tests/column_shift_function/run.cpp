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
#include <tuple>
#include <sstream>


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector< std::pair<natural_32_bit,natural_32_bit> > const  desired_tissue_dimensions = {
        { 100U, 100U },
        { 500U, 500U },
        { 100U, 500U },
        { 50U, 200U },
        { 225U, 210U },
        { 75U, 25U },
        { 150U, 125U },
    };

    std::vector<natural_16_bit> const  template_dimensions = {
        20U, 10U, 9U, 8U, 7U, 6U, 5U, 4U, 3U, 2U
    };

    std::vector<natural_8_bit> const  template_repetitions = {
        10U, 9U, 8U, 7U, 6U, 5U
    };

    for (auto const&  desired_tissue_dim : desired_tissue_dimensions)
        for (auto const  template_dim_x : template_dimensions)
            for (auto const  template_dim_y : template_dimensions)
                for (auto const  template_rep_x : template_repetitions)
                    for (auto const  template_rep_y : template_repetitions)
                    {
                        natural_32_bit  num_tissue_cells_along_axis_x;
                        natural_32_bit  scale_of_templates_x;
                        cellconnect::compute_tissue_axis_length_and_template_scale(
                                    desired_tissue_dim.first,
                                    template_dim_x,
                                    template_rep_x,
                                    num_tissue_cells_along_axis_x,
                                    scale_of_templates_x
                                    );

                        natural_32_bit  num_tissue_cells_along_axis_y;
                        natural_32_bit  scale_of_templates_y;
                        cellconnect::compute_tissue_axis_length_and_template_scale(
                                    desired_tissue_dim.second,
                                    template_dim_y,
                                    template_rep_y,
                                    num_tissue_cells_along_axis_y,
                                    scale_of_templates_y
                                    );

                        natural_16_bit const  num_exists = (template_dim_x * template_dim_y / 2U) / 8U;

                        std::vector<cellconnect::shift_template>  shift_templates = {
                            // left-top
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT, num_exists },
                              }
                            },

                            // left-middle
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT, num_exists },
                                  { cellconnect::GOTO_RIGHT_UP, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                              }
                            },

                            // left-bottom
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_RIGHT, num_exists },
                                  { cellconnect::GOTO_RIGHT_UP, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                              }
                            },

                            // middle-top
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_LEFT, num_exists },
                                  { cellconnect::GOTO_LEFT_DOWN, num_exists },
                                  { cellconnect::GOTO_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT, num_exists },
                              }
                            },

                            // middle-middle
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_LEFT_UP, num_exists },
                                  { cellconnect::GOTO_LEFT, num_exists },
                                  { cellconnect::GOTO_LEFT_DOWN, num_exists },
                                  { cellconnect::GOTO_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT_DOWN, num_exists },
                                  { cellconnect::GOTO_RIGHT, num_exists },
                                  { cellconnect::GOTO_RIGHT_UP, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                              }
                            },

                            // middle-bottom
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_RIGHT, num_exists },
                                  { cellconnect::GOTO_RIGHT_UP, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                                  { cellconnect::GOTO_LEFT_UP, num_exists },
                                  { cellconnect::GOTO_LEFT, num_exists },
                              }
                            },

                            // right-top
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_LEFT, num_exists },
                                  { cellconnect::GOTO_LEFT_DOWN, num_exists },
                                  { cellconnect::GOTO_DOWN, num_exists },
                              }
                            },

                            // right-middle
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_LEFT_UP, num_exists },
                                  { cellconnect::GOTO_LEFT, num_exists },
                                  { cellconnect::GOTO_LEFT_DOWN, num_exists },
                                  { cellconnect::GOTO_DOWN, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                              }
                            },

                            // right-bottom
                            { template_dim_x, template_dim_y,
                              {
                                  { cellconnect::GOTO_LEFT_UP, num_exists },
                                  { cellconnect::GOTO_LEFT, num_exists },
                                  { cellconnect::GOTO_UP, num_exists },
                              }
                            }
                        };

                        TEST_PROGRESS_UPDATE();
                    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
