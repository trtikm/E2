#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/check_for_network_properties.hpp>
#include <cellconnect/column_shift_function.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <cellconnect/fill_delimiters_between_territorial_lists.hpp>
#include <cellconnect/spread_synapses_into_neighbourhoods.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <boost/filesystem.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/test.hpp>
#include <utility/random.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <vector>
#include <algorithm>
#include <tuple>
#include <limits>
#include <memory>
#include <cstdlib>


struct  msgstream
{
    template<typename T>
    msgstream&  operator<<(T const& value) { m_stream << value; return *this; }
    std::string  get() const { return m_stream.str(); }
    operator std::string() const { return get(); }
private:
    std::ostringstream  m_stream;
};


#include <fstream>

static void build_fill_src_coords_matrix(
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr,
        std::vector<natural_32_bit>& matrix
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(static_state_ptr->num_cells_of_cell_kind(static_state_ptr->num_kinds_of_cells() - 1U) == 1U);

    static std::vector<natural_32_bit>  multipliers = {
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
            matrix.at(row_shift + j) = (natural_32_bit)((float_64_bit)matrix.at(row_shift + j) * (float_64_bit)TiNi / (float_64_bit)SUM);
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

    static std::vector<natural_32_bit>  multipliers = {
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

    std::rotate(multipliers.begin(), multipliers.begin() + 1, multipliers.end());

    INVARIANT(
            [](natural_64_bit const  total_num_synapses, std::vector<natural_32_bit> const&  matrix) {
                natural_64_bit  sum = 0ULL;
                for (natural_32_bit count : matrix)
                    sum += count;
                return sum == total_num_synapses;
            }(total_num_synapses,matrix)
            );
}


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
                    { {1U,2U,(natural_16_bit)(template_rep_x - 2U)}, },
                    { {1U,2U,(natural_16_bit)(template_rep_y - 2U)}, }
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

    if ((natural_16_bit)(middle1_dim_x * middle1_dim_y) < 2U)
        return {};

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
                    { {1U,3U,(natural_16_bit)(template_rep_x - 2U)}, },
                    { {1U,3U,(natural_16_bit)(template_rep_y - 2U)}, }
            }
    };

    return shift_fn;
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector<natural_32_bit> coefs = { 1, 4, 3, 8, 1, 9, 2, 5, 4, 2, 7, 6, 6, 5, 8, 9, 3 };

    natural_32_bit  test_counter = 0U;

    boost::filesystem::path const  output_root_dir = (msgstream() << "./output/" << get_program_name()).get();
    std::string const  file_prefix_in_degrees_of_kind = "in_degrees_of_kind_";
    std::string const  file_prefix_out_degrees_of_kind = "out_degrees_of_kind_";
    std::string const  file_prefix_in_degrees_summary = "in_degrees_summary";
    std::string const  file_prefix_out_degrees_summary = "out_degrees_summary";
    std::string const  file_prefix_in_degrees_summary_matrix = "in_degrees_summary_matrix_";
    std::string const  file_prefix_out_degrees_summary_matrix = "out_degrees_summary_matrix_";

    typedef std::tuple<natural_32_bit,  // 00: num cells x
                       natural_32_bit,  // 01: num cells y
                       natural_16_bit,  // 02: num tissue cell kinds
                       natural_16_bit,  // 03: num sensory cell kinds
                       bool,            // 04: is torus axis x?
                       bool,            // 05: is torus axis y?
                       natural_32_bit,  // 06: largest template dimension x
                       natural_32_bit,  // 07: largest template dimension y
                       natural_32_bit,  // 08: num template repetitions x
                       natural_32_bit,  // 09: num template repetitions y
                       natural_32_bit,  // 10: diameter x for column shift function
                       natural_32_bit,  // 11: diameter y for column shift function
                       bool,            // 12: build small shift function?
                       natural_32_bit,  // 13: num rows in in/out-degrees distribution matrix
                       natural_32_bit,  // 14: num columns in in/out-degrees distribution matrix
                       natural_32_bit   // 15: num thread to be used for computation
                       >
            tissue_props;
    for (tissue_props props :
         std::vector<tissue_props>{
                tissue_props{ 10U, 10U, 3U, 3U, false, false, 3U, 3U, 4U, 4U, 3U, 3U, true, 3U, 3U, 8U },
//                tissue_props{ 50U, 50U, 6U, 6U, false, false, 5U, 5U, 10U, 10U, 5U, 5U, true, 1U, 1U, 1U },
//                tissue_props{ 75U, 50U, 6U, 7U, false, true, 7U, 5U, 10U, 10U, 7U, 5U, false, 5U, 3U, 8U },
//                tissue_props{ 50U, 100U, 8U, 10U, true, false, 5U, 10U, 10U, 10U, 5U, 9U, true, 5U, 10U, 16U },
//                tissue_props{ 100U, 75U, 10U, 15U, true, true, 10U, 7U, 10U, 10U, 9U, 7U, false, 20U, 8U, 32U },
                })
    {
        natural_16_bit const  largest_template_dim_x = std::get<6>(props);
        natural_16_bit const  largest_template_dim_y = std::get<7>(props);

        natural_16_bit const  template_rep_x = std::get<8>(props);
        natural_16_bit const  template_rep_y = std::get<9>(props);

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
            natural_32_bit coef = coefs.at((i + 0U) % coefs.size()) +
                                  coefs.at((i + 3U) % coefs.size()) ;
            num_tissue_cells_of_cell_kind.push_back(2U + coef / 2U);

            coef += coefs.at((i + 5U) % coefs.size()) +
                    coefs.at((i + 8U) % coefs.size()) ;
            num_synapses_in_territory_of_cell_kind.push_back(2U + coef);
        }

        std::vector<natural_32_bit> num_sensory_cells_of_cell_kind;
        for (natural_16_bit i = 0U; i < sensory_cell_kinds; ++i)
            if (i + 1U == sensory_cell_kinds)
                num_sensory_cells_of_cell_kind.push_back(1U);
            else
            {
                natural_32_bit const coef = coefs.at((i + 2U) % coefs.size()) +
                                            coefs.at((i + 4U) % coefs.size());
                num_sensory_cells_of_cell_kind.push_back(2U + coef / 2U);
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

        natural_32_bit const  diameter_x = std::get<10>(props);
        if (diameter_x >= static_tissue->num_cells_along_x_axis())
            continue;

        natural_32_bit const  diameter_y = std::get<11>(props);
        if (diameter_y >= static_tissue->num_cells_along_y_axis())
            continue;

        bool const  use_small_shift_function = std::get<12>(props);

        natural_32_bit const  num_threads = std::get<15>(props);

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

                std::shared_ptr<cellconnect::column_shift_function const> const  shift_fn_ptr =
                        (use_small_shift_function ? &build_small_shift_function : &build_big_shift_function)(
                                cells_x,
                                cells_y,
                                largest_template_dim_x,
                                largest_template_dim_y,
                                scale_of_templates_x,
                                scale_of_templates_y,
                                template_rep_x,
                                template_rep_y
                                );


                cellconnect::spread_synapses_into_neighbourhoods(
                            dynamic_tissue,
                            target_kind,
                            source_kind,
                            diameter_x,
                            diameter_y,
                            matrix_spread_synapses,
                            *shift_fn_ptr,
                            num_threads
                            );

                TEST_PROGRESS_UPDATE();
            }

        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
                {
                    natural_32_bit const num_synapses =
                            static_tissue->num_synapses_in_territory_of_cell_kind(
                                    static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                    );
                    for (natural_32_bit s = 0U; s < num_synapses; ++s)
                    {
                        cellab::territorial_state_of_synapse  territorial_state;
                        switch (get_random_natural_32_bit_in_range(0U,6U))
                        {
                        case 0U: territorial_state = cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY; break;
                        case 1U: territorial_state = cellab::MIGRATION_ALONG_POSITIVE_X_AXIS; break;
                        case 2U: territorial_state = cellab::MIGRATION_ALONG_NEGATIVE_X_AXIS; break;
                        case 3U: territorial_state = cellab::MIGRATION_ALONG_POSITIVE_Y_AXIS; break;
                        case 4U: territorial_state = cellab::MIGRATION_ALONG_NEGATIVE_Y_AXIS; break;
                        case 5U: territorial_state = cellab::MIGRATION_ALONG_POSITIVE_COLUMNAR_AXIS; break;
                        case 6U: territorial_state = cellab::MIGRATION_ALONG_NEGATIVE_COLUMNAR_AXIS; break;
                        default:
                            UNREACHABLE();
                        }

                        bits_reference const  ref_to_territorial_state =
                                dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s);

                        value_to_bits(territorial_state,ref_to_territorial_state);
                    }

                    TEST_PROGRESS_UPDATE();
                }

        // We currently do not use delimiters lists in this test. It thus means
        // that we always call 'compute_in_degrees_of_tissue_cells_of_given_kind'
        // with 'ignore_delimiters_lists_and_check_territorial_states_of_all_synapses == true'.
        // That is the reason, why next 5 lines are commented.
//        cellconnect::fill_delimiters_between_territorial_lists(
//                    dynamic_tissue,
//                    cellconnect::delimiters_fill_kind::SYNAPSES_DISTRIBUTED_REGULARLY,
//                    num_threads);
//        TEST_PROGRESS_UPDATE();

        natural_32_bit const  num_rows_in_distribution_matrix = std::get<13>(props);
        natural_32_bit const  num_columns_in_distribution_matrix = std::get<14>(props);

        boost::filesystem::path const  test_root_dir =
                output_root_dir / (msgstream() << "tissue_" << test_counter).get();
        boost::filesystem::create_directories(test_root_dir);

        std::unordered_map<natural_32_bit,natural_64_bit> summary_distribution;
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  summary_distribution_matrix;
        for (cellab::kind_of_cell  i = 0U; i < static_tissue->num_kinds_of_tissue_cells(); ++i)
        {
            std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  distribution_of_in_degrees;
            cellconnect::compute_in_degrees_of_tissue_cells_of_given_kind(
                        dynamic_tissue,
                        i,
                        true,
                        num_rows_in_distribution_matrix,
                        num_columns_in_distribution_matrix,
                        num_threads,
                        distribution_of_in_degrees,
                        cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
                        );

            cellconnect::add_degree_distributions(distribution_of_in_degrees,summary_distribution_matrix);

            for (natural_64_bit  j = 0ULL; j < distribution_of_in_degrees.size(); ++j)
            {
                boost::filesystem::path const  plt_pathname =
                        test_root_dir / (msgstream() << file_prefix_in_degrees_of_kind
                                                     << i << "_" << j << ".plt").get();
                {
                    std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                    cellconnect::degrees_distribution_to_gnuplot_plot(
                                ofs,
                                distribution_of_in_degrees.at(j),
                                msgstream() << "Distribution of in-degrees of cells of kind "
                                            << i << " at region ("
                                            << (j / num_columns_in_distribution_matrix) << ","
                                            << (j % num_columns_in_distribution_matrix) << ").",
                                (test_root_dir / (msgstream() << file_prefix_in_degrees_of_kind
                                                              << i << "_" << j << ".svg").get()).string()
                                );
                }
                std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
                {
                    std::ofstream  ofs(
                            (test_root_dir / (msgstream() << file_prefix_in_degrees_of_kind
                                                          << i << "_" << j << ".html").get()).string(),
                            std::ostream::out
                            );
                    ofs << "<!DOCTYPE html>\n"
                        << "<html>\n"
                        << "<body>\n"
                        << "<img src=\"./" << file_prefix_in_degrees_of_kind << i << "_" << j << ".svg\" alt=\"[./"
                                           << file_prefix_in_degrees_of_kind << i << "_" << j << ".svg]\">\n"
                        << "</body>\n"
                        << "</html>\n"
                        ;
                }
            }

            TEST_PROGRESS_UPDATE();
        }
        cellconnect::add_degree_distributions(summary_distribution_matrix,summary_distribution);

        for (natural_64_bit  j = 0ULL; j < summary_distribution_matrix.size(); ++j)
        {
            boost::filesystem::path const  plt_pathname =
                    test_root_dir / (msgstream() << file_prefix_in_degrees_summary_matrix
                                                 << j << ".plt").get();
            {
                std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                cellconnect::degrees_distribution_to_gnuplot_plot(
                            ofs,
                            summary_distribution_matrix.at(j),
                            msgstream() << "Distribution of in-degrees of cells of all kinds at region ("
                                        << (j / num_columns_in_distribution_matrix) << ","
                                        << (j % num_columns_in_distribution_matrix) << ").",
                            (test_root_dir / (msgstream() << file_prefix_in_degrees_summary_matrix
                                                          << j << ".svg").get()).string()
                            );
            }
            std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
            {
                std::ofstream  ofs(
                        (test_root_dir / (msgstream() << file_prefix_in_degrees_summary_matrix
                                                      << j << ".html").get()).string(),
                        std::ostream::out
                        );
                ofs << "<!DOCTYPE html>\n"
                    << "<html>\n"
                    << "<body>\n"
                    << "<img src=\"./" << file_prefix_in_degrees_summary_matrix << j << ".svg\" alt=\"[./"
                                       << file_prefix_in_degrees_summary_matrix << j << ".svg]\">\n"
                    << "</body>\n"
                    << "</html>\n"
                    ;
            }
        }
        {
            std::ofstream  ofs((test_root_dir / "in_degrees_matrix.html").string(),std::ostream::out);
            ofs << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<body>\n"
                << "<h3>In-degree distributions of cells of all kinds in regions of tissue.</h3>\n"
                << "</body>\n"
                << "</html>\n"
                ;
        }
        {
            boost::filesystem::path const  plt_pathname =
                    test_root_dir / (msgstream() << file_prefix_in_degrees_summary << ".plt").get();
            {
                std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                cellconnect::degrees_distribution_to_gnuplot_plot(
                            ofs,
                            summary_distribution,
                            "Distribution of in-degrees of cells of all kinds.",
                            (test_root_dir / (msgstream() << file_prefix_in_degrees_summary
                                                          << ".svg").get()).string()
                            );
            }
            std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
            {
                std::ofstream  ofs(
                        (test_root_dir / (msgstream() << file_prefix_in_degrees_summary << ".html").get()).string(),
                        std::ostream::out
                        );
                ofs << "<!DOCTYPE html>\n"
                    << "<html>\n"
                    << "<body>\n"
                    << "<img src=\"./" << file_prefix_in_degrees_summary << ".svg\" alt=\"[./"
                                       << file_prefix_in_degrees_summary << ".svg]\">\n"
                    << "</body>\n"
                    << "</html>\n"
                    ;
            }
        }

        summary_distribution.clear();
        summary_distribution_matrix.clear();
        for (cellab::kind_of_cell  i = 0U; i < static_tissue->num_kinds_of_tissue_cells(); ++i)
        {
            std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  distribution_of_out_degrees;
            cellconnect::compute_out_degrees_of_tissue_cells_of_given_kind(
                        dynamic_tissue,
                        i,
                        num_rows_in_distribution_matrix,
                        num_columns_in_distribution_matrix,
                        num_threads,
                        distribution_of_out_degrees,
                        cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
                        );

            cellconnect::add_degree_distributions(distribution_of_out_degrees,summary_distribution_matrix);

            for (natural_64_bit  j = 0ULL; j < distribution_of_out_degrees.size(); ++j)
            {
                boost::filesystem::path const  plt_pathname =
                        test_root_dir / (msgstream() << file_prefix_out_degrees_of_kind
                                                     << i << "_" << j << ".plt").get();
                {
                    std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                    cellconnect::degrees_distribution_to_gnuplot_plot(
                                ofs,
                                distribution_of_out_degrees.at(j),
                                msgstream() << "Distribution of in-degrees of cells of kind "
                                            << i << " at region ("
                                            << (j / num_columns_in_distribution_matrix) << ","
                                            << (j % num_columns_in_distribution_matrix) << ").",
                                (test_root_dir / (msgstream() << file_prefix_out_degrees_of_kind
                                                              << i << "_" << j << ".svg").get()).string()
                                );
                }
                std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
                {
                    std::ofstream  ofs(
                            (test_root_dir / (msgstream() << file_prefix_out_degrees_of_kind
                                                          << i << "_" << j << ".html").get()).string(),
                            std::ostream::out
                            );
                    ofs << "<!DOCTYPE html>\n"
                        << "<html>\n"
                        << "<body>\n"
                        << "<img src=\"./" << file_prefix_out_degrees_of_kind << i << "_" << j << ".svg\" alt=\"[./"
                                           << file_prefix_out_degrees_of_kind << i << "_" << j << ".svg]\">\n"
                        << "</body>\n"
                        << "</html>\n"
                        ;
                }
            }

            TEST_PROGRESS_UPDATE();
        }
        cellconnect::add_degree_distributions(summary_distribution_matrix,summary_distribution);

        for (natural_64_bit  j = 0ULL; j < summary_distribution_matrix.size(); ++j)
        {
            boost::filesystem::path const  plt_pathname =
                    test_root_dir / (msgstream() << file_prefix_out_degrees_summary_matrix
                                                 << j << ".plt").get();
            {
                std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                cellconnect::degrees_distribution_to_gnuplot_plot(
                            ofs,
                            summary_distribution_matrix.at(j),
                            msgstream() << "Distribution of in-degrees of cells of all kinds at region ("
                                        << (j / num_columns_in_distribution_matrix) << ","
                                        << (j % num_columns_in_distribution_matrix) << ").",
                            (test_root_dir / (msgstream() << file_prefix_out_degrees_summary_matrix
                                                          << j << ".svg").get()).string()
                            );
            }
            std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
            {
                std::ofstream  ofs(
                        (test_root_dir / (msgstream() << file_prefix_out_degrees_summary_matrix
                                                      << j << ".html").get()).string(),
                        std::ostream::out
                        );
                ofs << "<!DOCTYPE html>\n"
                    << "<html>\n"
                    << "<body>\n"
                    << "<img src=\"./" << file_prefix_out_degrees_summary_matrix << j << ".svg\" alt=\"[./"
                                       << file_prefix_out_degrees_summary_matrix << j << ".svg]\">\n"
                    << "</body>\n"
                    << "</html>\n"
                    ;
            }
        }
        {
            std::ofstream  ofs((test_root_dir / "out_degrees_matrix.html").string(),std::ostream::out);
            ofs << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<body>\n"
                << "<h3>Out-degree distributions of cells of all kinds in regions of tissue.</h3>\n"
                << "</body>\n"
                << "</html>\n"
                ;
        }
        {
            boost::filesystem::path const  plt_pathname =
                    test_root_dir / (msgstream() << file_prefix_out_degrees_summary << ".plt").get();
            {
                std::ofstream  ofs(plt_pathname.string(),std::ostream::out);
                cellconnect::degrees_distribution_to_gnuplot_plot(
                            ofs,
                            summary_distribution,
                            "Distribution of in-degrees of cells of all kinds.",
                            (test_root_dir / (msgstream() << file_prefix_out_degrees_summary
                                                          << ".svg").get()).string()
                            );
            }
            std::system((msgstream() << "gnuplot " << plt_pathname).get().c_str());
            {
                std::ofstream  ofs(
                        (test_root_dir / (msgstream() << file_prefix_out_degrees_summary << ".html").get()).string(),
                        std::ostream::out
                        );
                ofs << "<!DOCTYPE html>\n"
                    << "<html>\n"
                    << "<body>\n"
                    << "<img src=\"./" << file_prefix_out_degrees_summary << ".svg\" alt=\"[./"
                                       << file_prefix_out_degrees_summary << ".svg]\">\n"
                    << "</body>\n"
                    << "</html>\n"
                    ;
            }
        }

        {
            std::ofstream  ofs((test_root_dir / "index.html").string(),std::ostream::out);
            ofs << "<!DOCTYPE html>\n"
                << "<html>\n"
                << "<body>\n"
                << "<h3>TISSUE " << test_counter << ": Plots of in/out-degree distributions of tissue cells</h3>\n"
                << "<ul>\n"
                << "<li><a href=\"./" << file_prefix_in_degrees_summary << ".html\">"
                                      << "In-degrees of cells of all kinds.</a></li>\n"
                << "<li><a href=\"./" << file_prefix_out_degrees_summary << ".html\">"
                                      << "Out-degrees of cells of all kinds.</a></li>\n"
                << "</ul>\n"
                << "<ul>\n"
                << "<li><a href=\"./in_degrees_matrix.html\">"
                                      << "In-degrees of cells of all kinds in regions of tissue.</a></li>\n"
                << "<li><a href=\"./out_degrees_matrix.html\">"
                                      << "Out-degrees of cells of all kinds in regions of tissue.</a></li>\n"
                << "</ul>\n"
                ;

            for (cellab::kind_of_cell  i = 0U; i < static_tissue->num_kinds_of_tissue_cells(); ++i)
            {
                ofs << "<p></p>\n<ul>\n"
                    << "<li><a href=\"./in_degrees_matrix_" << i << ".html\">"
                            << "In-degrees of cells of kind " << i << "</a></li>\n"
                    << "<li><a href=\"./out_degrees_matrix_" << i << ".html\">"
                            << "Out-degrees of cells of kind " << i << "</a></li>\n"
                    << "</ul>\n"
                    ;
            }

            ofs << "</body>\n"
                << "</html>\n"
                ;
        }

        ++test_counter;
    }

    {
        std::ofstream  ofs((output_root_dir / "index.html").string(),std::ostream::out);
        ofs << "<!DOCTYPE html>\n"
            << "<html>\n"
            << "<body>\n"
            << "<h2>Plots of in/out-degree distributions of tissue cells</h2>\n"
            << "<ul>\n"
            ;

        for (natural_32_bit  i = 0U; i < test_counter; ++i)
            ofs << "<li><a href=\"./tissue_" << i << "/index.html\">tissue " << i << "</a></li>\n";

        ofs << "</ul>\n"
            << "</body>\n"
            << "</html>\n"
            ;
    }

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}