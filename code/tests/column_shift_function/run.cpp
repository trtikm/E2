#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/column_shift_function.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/functional/hash.hpp>
#include <vector>
#include <unordered_set>
#include <tuple>
#include <memory>


static std::shared_ptr<cellconnect::column_shift_function const>  build_small_shift_function(
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
                    { {1U,2U,template_rep_x - 2U}, },
                    { {1U,2U,template_rep_y - 2U}, }
            }
    };

    return shift_fn;
}

static std::shared_ptr<cellconnect::column_shift_function const>  build_big_shift_function(
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

    natural_16_bit const  middle1_dim_x = largest_template_dim_x / 2U;
    ASSUMPTION(middle1_dim_x > 0U);
    natural_16_bit const  middle2_dim_x = largest_template_dim_x - middle1_dim_x;
    ASSUMPTION(middle2_dim_x > 0U);

    natural_16_bit const  middle1_dim_y = largest_template_dim_y / 2U;
    ASSUMPTION(middle1_dim_y > 0U);
    natural_16_bit const  middle2_dim_y = largest_template_dim_y - middle1_dim_y;
    ASSUMPTION(middle2_dim_y > 0U);

    natural_16_bit const  large_num_exists = (largest_template_dim_x * largest_template_dim_y / 2U) / 8U;
    natural_16_bit const  small_num_exists = (middle1_dim_x * middle1_dim_y / 2U) / 8U;

    std::vector<cellconnect::shift_template>  shift_templates = {
        // left-top
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_DOWN, large_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, large_num_exists },
          }
        },

        // left-middle1
        { middle1_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, large_num_exists },
          }
        },

        // left-middle2
        { middle2_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_DOWN, large_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // left-bottom
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_RIGHT, large_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, large_num_exists },
          }
        },

        // middle1-top
        { largest_template_dim_x, middle1_dim_y,
          {
              { cellconnect::DIR_LEFT, large_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
          }
        },

        // middle1-middle1
        { middle1_dim_x, middle1_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // middle1-middle2
        { middle2_dim_x, middle1_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // middle1-bottom
        { largest_template_dim_x, middle1_dim_y,
          {
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, large_num_exists },
          }
        },

        // middle2-top
        { largest_template_dim_x, middle2_dim_y,
          {
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, large_num_exists },
          }
        },

        // middle2-middle1
        { middle1_dim_x, middle2_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // middle2-middle2
        { middle2_dim_x, middle2_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT_DOWN, small_num_exists },
              { cellconnect::DIR_RIGHT, small_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // middle2-bottom
        { largest_template_dim_x, middle2_dim_y,
          {
              { cellconnect::DIR_RIGHT, large_num_exists },
              { cellconnect::DIR_RIGHT_UP, small_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
          }
        },

        // right-top
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT, large_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, large_num_exists },
          }
        },

        // right-middle1
        { middle1_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, small_num_exists },
              { cellconnect::DIR_UP, large_num_exists },
          }
        },

        // right-middle2
        { middle2_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, small_num_exists },
              { cellconnect::DIR_LEFT_DOWN, small_num_exists },
              { cellconnect::DIR_DOWN, large_num_exists },
              { cellconnect::DIR_UP, small_num_exists },
          }
        },

        // right-bottom
        { largest_template_dim_x, largest_template_dim_y,
          {
              { cellconnect::DIR_LEFT_UP, small_num_exists },
              { cellconnect::DIR_LEFT, large_num_exists },
              { cellconnect::DIR_UP, large_num_exists },
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
                        4U, 4U,
                        {
                            0, 4, 8,  12,
                            1, 5, 9,  13,
                            2, 6, 10, 14,
                            3, 7, 11, 15
                        }
                    },
                    { {1U,3U,template_rep_x - 2U}, },
                    { {1U,3U,template_rep_y - 2U}, }
            }
    };

    return shift_fn;
}

static void  test_shift_function(std::shared_ptr<cellconnect::column_shift_function const>  const  shift_fn)
{
    TMPROF_BLOCK();

    natural_32_bit  progress_counter = 0U;
    std::unordered_set< std::pair<natural_32_bit,natural_32_bit>, boost::hash< std::pair<natural_32_bit,natural_32_bit> > >  shifts;
    for (natural_32_bit  x = 0U; x < shift_fn->num_cells_along_x_axis(); ++x)
        for (natural_32_bit  y = 0U; y < shift_fn->num_cells_along_y_axis(); ++y)
        {
            std::pair<natural_32_bit,natural_32_bit> const  coords = shift_fn->operator()(x,y);
            TEST_SUCCESS(coords.first < shift_fn->num_cells_along_x_axis());
            TEST_SUCCESS(coords.second < shift_fn->num_cells_along_y_axis());
            TEST_SUCCESS(shifts.count(coords) == 0U);
            shifts.insert(coords);

            if (progress_counter % 100U == 0U)
            {
                TEST_PROGRESS_UPDATE();
                progress_counter = 0U;
            }
        }
    TEST_SUCCESS(shifts.size() == (natural_64_bit)shift_fn->num_cells_along_x_axis() * (natural_64_bit)shift_fn->num_cells_along_y_axis());
}


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

    std::vector<natural_16_bit> const  template_repetitions = {
        10U, 9U, 8U, 7U, 6U, 5U
    };

    for (auto const&  desired_tissue_dim : desired_tissue_dimensions)
        for (auto const  largest_template_dim_x : template_dimensions)
            for (auto const  largest_template_dim_y : template_dimensions)
                for (auto const  template_rep_x : template_repetitions)
                    for (auto const  template_rep_y : template_repetitions)
                    {
                        natural_32_bit  num_tissue_cells_along_axis_x;
                        natural_32_bit  scale_of_templates_x;
                        cellconnect::compute_tissue_axis_length_and_template_scale(
                                    desired_tissue_dim.first,
                                    largest_template_dim_x,
                                    template_rep_x,
                                    num_tissue_cells_along_axis_x,
                                    scale_of_templates_x
                                    );

                        natural_32_bit  num_tissue_cells_along_axis_y;
                        natural_32_bit  scale_of_templates_y;
                        cellconnect::compute_tissue_axis_length_and_template_scale(
                                    desired_tissue_dim.second,
                                    largest_template_dim_y,
                                    template_rep_y,
                                    num_tissue_cells_along_axis_y,
                                    scale_of_templates_y
                                    );

                        test_shift_function(
                                    build_small_shift_function(
                                            num_tissue_cells_along_axis_x,
                                            num_tissue_cells_along_axis_y,
                                            largest_template_dim_x,
                                            largest_template_dim_y,
                                            scale_of_templates_x,
                                            scale_of_templates_y,
                                            template_rep_x,
                                            template_rep_y
                                            )
                                    );

                        TEST_PROGRESS_UPDATE();

                        test_shift_function(
                                    build_big_shift_function(
                                            num_tissue_cells_along_axis_x,
                                            num_tissue_cells_along_axis_y,
                                            largest_template_dim_x,
                                            largest_template_dim_y,
                                            scale_of_templates_x,
                                            scale_of_templates_y,
                                            template_rep_x,
                                            template_rep_y
                                            )
                                    );

                        TEST_PROGRESS_UPDATE();
                    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
