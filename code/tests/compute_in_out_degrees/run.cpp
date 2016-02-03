#include "./program_info.hpp"
#include "./program_options.hpp"
#include <cellconnect/check_for_network_properties.hpp>
#include <cellconnect/column_shift_function.hpp>
#include <cellconnect/fill_coords_of_source_cells_of_synapses_in_tissue.hpp>
#include <cellconnect/fill_delimiters_between_territorial_lists.hpp>
#include <cellconnect/spread_synapses_into_neighbourhoods.hpp>
#include <cellconnect/dump.hpp>
#include <cellab/dynamic_state_of_neural_tissue.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <cellab/dump.hpp>
#include <utility/msgstream.hpp>
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
#include <fstream>


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
                                                   static_state_ptr->num_tissue_cells_of_cell_kind(i) ;

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
        }
        while (matrix.at(row_shift + last_column_index) > 0U && num_skipped < static_state_ptr->num_kinds_of_cells() - 1U);

        INVARIANT(
            static_state_ptr->num_synapses_in_territory_of_cell_kind(i) * static_state_ptr->num_tissue_cells_of_cell_kind(i) ==
            [](std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_state_ptr,
               std::vector<natural_32_bit> const&  matrix, natural_32_bit const row_shift) {
                    natural_64_bit  SUM = 0ULL;
                    for (cellab::kind_of_cell j = 0U; j < static_state_ptr->num_kinds_of_cells(); ++j)
                        SUM += (natural_64_bit)matrix.at(row_shift + j) * (natural_64_bit)static_state_ptr->num_cells_of_cell_kind(j);
                    return SUM;
                    }(static_state_ptr,matrix,row_shift)
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


void  generate_svg_html(std::string const&  outdir, std::string const&  filename_base)
{
    cellconnect::generate_svg_file_from_plt_file_using_gnuplot(msgstream() << outdir << filename_base << ".plt");
    std::ofstream  ofs(msgstream() << outdir << filename_base << ".html",std::ostream::out);
    cellconnect::dump_html_file_with_embedded_svg(ofs,msgstream() << "./" << filename_base << ".svg");
}

void  generate_table_of_regions_of_distributions(
        std::string const&  outdir,
        std::string const&  filename_base,
        std::string const&  filename_base_of_links,
        std::string const&  title_base,
        natural_32_bit const  num_rows,
        natural_32_bit const  num_columns
        )
{
    std::ofstream  ofs(msgstream() << outdir << filename_base << ".html",std::ostream::out);
    cellconnect::dump_html_table_with_links_to_distributions_of_individual_regions(
                ofs,
                num_rows,
                num_columns,
                msgstream() << "./" << filename_base_of_links,
                msgstream() << title_base << " in regions of tissue",
                "",
                "Table of regions of distribution of degrees"
                );
}

void dump_distribution_matrix(
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  distribution,
        natural_32_bit const  test_counter,
        natural_32_bit const  num_columns_in_distribution_matrix,
        std::string const&  filename_base,
        std::string const&  title_base,
        std::string const&  matrix_file_name_base
        )
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << test_counter << "/";
    boost::filesystem::create_directories(outdir);

    for (natural_64_bit  j = 0ULL; j < distribution.size(); ++j)
    {
        std::string const  plt_pathname = msgstream() << outdir << filename_base << j << ".plt";
        {
            std::ofstream  ofs(plt_pathname,std::ostream::out);
            cellconnect::dump_degrees_distribution_to_gnuplot_plot(
                        ofs,
                        distribution.at(j),
                        msgstream() << title_base << " ("
                                    << (j / num_columns_in_distribution_matrix) << ","
                                    << (j % num_columns_in_distribution_matrix) << ").",
                        msgstream() << outdir << filename_base << j << ".svg"
                        );
        }
        generate_svg_html(outdir,msgstream() << filename_base << j);
    }
    generate_table_of_regions_of_distributions(
                outdir,
                matrix_file_name_base,
                filename_base,
                title_base,
                (natural_32_bit)(distribution.size() / num_columns_in_distribution_matrix),
                num_columns_in_distribution_matrix
                );
}

void dump_distribution_matrix(
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  distribution,
        natural_32_bit const  test_counter,
        cellab::kind_of_cell const  cell_kind,
        natural_32_bit const  num_columns_in_distribution_matrix,
        bool const  as_in_degrees
        )
{
    std::string const  outtype = as_in_degrees ? "in" : "out";
    dump_distribution_matrix(
                distribution,
                test_counter,
                num_columns_in_distribution_matrix,
                msgstream() << outtype << "_degrees_of_kind_" << cell_kind << "_in_region_",
                msgstream() << "Distribution of " << outtype << "-degrees of cells of kind " << cell_kind << " in region",
                msgstream() << outtype << "_degrees_of_kind_" << cell_kind << "_table_of_regions"
                );
}

void dump_distribution_matrix(
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  distribution,
        natural_32_bit const  test_counter,
        natural_32_bit const  num_columns_in_distribution_matrix,
        bool const  as_in_degrees
        )
{
    std::string const  outtype = as_in_degrees ? "in" : "out";
    dump_distribution_matrix(
                distribution,
                test_counter,
                num_columns_in_distribution_matrix,
                msgstream() << outtype << "_degrees_of_all_kinds_in_region_",
                msgstream() << "Distribution of " << outtype << "-degrees of tissue cells of all kinds in region",
                msgstream() << outtype << "_degrees_of_all_kinds_table_of_regions"
                );
}

void dump_distribution(
        std::unordered_map<natural_32_bit,natural_64_bit> const&  distribution,
        natural_32_bit const  test_counter,
        bool const  as_in_degrees
        )
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << test_counter << "/";
    boost::filesystem::create_directories(outdir);

    std::string const  outtype = as_in_degrees ? "in" : "out";
    std::string const  filename_base = msgstream() << outtype << "_degrees_of_all_kinds";
    {
        std::ofstream  ofs(msgstream() << outdir << filename_base << ".plt",std::ostream::out);
        cellconnect::dump_degrees_distribution_to_gnuplot_plot(
                    ofs,
                    distribution,
                    msgstream() << "Distribution of " << outtype << "-degrees of tissue cells of all kinds.",
                    msgstream() << outdir << filename_base << ".svg"
                    );
    }
    generate_svg_html(outdir,filename_base);
}

void  dump_src_coords_setup_matrix(
    natural_32_bit const  tissue_index,
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_tissue_ptr,
    std::vector<natural_32_bit> const&  matrix_fill_src_coords
    )
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << tissue_index << "/";
    boost::filesystem::create_directories(outdir);

    {
        std::ofstream  ofs(msgstream() << outdir << "matrix_fill_src_coords_in_columns.html", std::ostream::out);
        cellconnect::dump_matrix_for_setup_of_source_cell_coordinates_in_tissue_columns(
                ofs,
                static_tissue_ptr->num_kinds_of_tissue_cells(),
                static_tissue_ptr->num_kinds_of_cells(),
                matrix_fill_src_coords
                );
    }
}

void  dump_spread_synapses_matrix(
        natural_32_bit const  tissue_index,
        cellab::kind_of_cell const  source_kind,
        cellab::kind_of_cell const  target_kind,
        natural_32_bit const  num_rows,
        natural_32_bit const  num_columns,
        std::vector<natural_32_bit> const&  spread_synapses_matrix
        )
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << tissue_index << "/";
    boost::filesystem::create_directories(outdir);

    {
        std::ofstream  ofs(msgstream() << outdir << "spread_synapses_matrix_"
                                       << source_kind << "_" << target_kind << ".html", std::ostream::out);
        cellconnect::dump_spread_synapses_matrix(
                ofs,
                num_rows,
                num_columns,
                spread_synapses_matrix
                );
    }
}

void generate_table_of_spread_synapses_matrices(
    natural_32_bit const  tissue_index,
    std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_tissue_ptr,
    natural_32_bit const  diameter_x,
    natural_32_bit const  diameter_y
    )
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << tissue_index << "/";
    boost::filesystem::create_directories(outdir);

    std::ofstream  ofs(msgstream() << outdir << "/table_of_spread_synapses_matrices.html", std::ostream::out);

    //for (cellab::kind_of_cell source_kind = 0U; source_kind < static_tissue->num_kinds_of_cells(); ++source_kind)
    //    for (cellab::kind_of_cell target_kind = 0U; target_kind < static_tissue->num_kinds_of_tissue_cells(); ++target_kind)
    //    {
    //    }
}

void  generate_root_html_file_for_tissue(
        natural_32_bit const  tissue_index,
        std::shared_ptr<cellab::static_state_of_neural_tissue const> const  static_tissue_ptr)
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/tissue_" << tissue_index << "/";
    boost::filesystem::create_directories(outdir);

    {
        std::ofstream  ofs(msgstream() << outdir << "/static_tissue_props.html",std::ostream::out);
        cellab::dump_in_html(ofs,static_tissue_ptr);
    }
    std::ofstream  ofs(msgstream() << outdir << "/index.html",std::ostream::out);
    ofs << "<!DOCTYPE html>\n"
           "<html>\n"
           "<head>\n"
           "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
           "    <title>Table of regions of distribution of degrees</title>\n"
           "    <style type=\"text/css\">\n"
           "        body {\n"
           "            background-color: white;\n"
           "            color: black;\n"
           "            margin-left: auto;\n"
           "            margin-right: auto;\n"
           "        }\n"
           "        h1, h2, h3, h4, h5, h6, table { font-family:\"Liberation serif\"; }\n"
           "        p, table {\n"
           "            font-size:12pt;\n"
           "            margin-left: auto;\n"
           "            margin-right: auto;\n"
           "            text-align: justify\n"
           "        }\n"
           "        th, td {\n"
           "            font-family:\"Liberation mono\", monospace;\n"
           "            font-size:10pt;\n"
           "            padding: 3pt;\n"
           "        }\n"
           "   </style>\n"
           "</head>\n"
           "<body>\n"
           "<h2>TISSUE " << tissue_index << ": Plots of in/out-degree distributions of tissue cells</h2>\n"
           "<ul>\n"
           "<li><a href=\"./static_tissue_props.html\">Properties of the static state of the tissue.</a></li>\n"
           "</ul>\n"
           "<ul>\n"
           "<li><a href=\"./matrix_fill_src_coords_in_columns.html\">"
                        "The matrix used in setup of source cell coordinates of synapses.</a></li>\n"
           "<li><a href=\"./table_of_spread_synapses_matrices.html\">"
                        "A table of links to matrices used in initial spreading of synapses in the tissue.</a></li>\n"
           "</ul>\n"
           "<ul>\n"
           "<li><a href=\"./in_degrees_of_all_kinds.html\">In-degrees of tissue cells of all kinds.</a></li>\n"
           "<li><a href=\"./out_degrees_of_all_kinds.html\">Out-degrees of tissue cells of all kinds.</a></li>\n"
           "</ul>\n"
           "<ul>\n"
           "<li><a href=\"./in_degrees_of_all_kinds_table_of_regions.html\">"
                        "In-degrees of tissue cells of all kinds in regions.</a></li>\n"
           "<li><a href=\"./out_degrees_of_all_kinds_table_of_regions.html\">"
                        "Out-degrees of tissue cells of all kinds in regions.</a></li>\n"
           "</ul>\n"
           ;
    for (cellab::kind_of_cell  i = 0U; i < static_tissue_ptr->num_kinds_of_tissue_cells(); ++i)
        ofs << "<p></p>\n<ul>\n"
               "<li><a href=\"./in_degrees_of_kind_" << i << "_table_of_regions.html\">"
                            "In-degrees of cells of kind " << i << " in regions</a></li>\n"
               "<li><a href=\"./out_degrees_of_kind_" << i << "_table_of_regions.html\">"
                            "Out-degrees of cells of kind " << i << " in regions</a></li>\n"
               "</ul>\n"
               ;
    ofs << "</body>\n"
           "</html>\n"
           ;
}

void  generate_root_html_file(natural_32_bit const  num_tissues)
{
    std::string const  outdir = msgstream() << "./output/" << get_program_name() << "/";
    boost::filesystem::create_directories(outdir);

    std::ofstream  ofs(msgstream() << outdir << "/index.html",std::ostream::out);
    ofs << "<!DOCTYPE html>\n"
           "<html>\n"
           "<body>\n"
           "<h2>Plots of in/out-degree distributions of tissue cells</h2>\n"
           "<ul>\n"
           ;
    for (natural_32_bit  i = 0U; i < num_tissues; ++i)
        ofs << "<li><a href=\"./tissue_" << i << "/index.html\">tissue " << i << "</a></li>\n";
    ofs << "</ul>\n"
           "</body>\n"
           "</html>\n"
           ;
}


void run()
{
    TMPROF_BLOCK();

    TEST_PROGRESS_SHOW();

    std::vector<natural_32_bit> coefs = { 1, 4, 3, 8, 1, 9, 2, 5, 4, 2, 7, 6, 6, 5, 8, 9, 3 };

    natural_32_bit  test_counter = 0U;

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
                tissue_props{ 10U, 10U, 3U, 3U, true, true, 3U, 3U, 4U, 4U, 3U, 3U, true, 3U, 3U, 8U },
                //tissue_props{ 50U, 50U, 6U, 6U, false, false, 5U, 5U, 10U, 10U, 5U, 5U, true, 1U, 1U, 8U },
                //tissue_props{ 75U, 50U, 6U, 7U, false, true, 7U, 5U, 10U, 10U, 7U, 5U, false, 5U, 3U, 8U },
                //tissue_props{ 50U, 100U, 8U, 10U, true, false, 5U, 10U, 10U, 10U, 5U, 9U, true, 5U, 10U, 16U },
                //tissue_props{ 100U, 75U, 10U, 15U, true, true, 10U, 7U, 10U, 10U, 9U, 7U, false, 20U, 8U, 32U },
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

        natural_16_bit const synapses_to_muscles_kinds = 2U * sensory_cell_kinds;
        std::vector<natural_32_bit>  num_synapses_to_muscles_of_synapse_kind;
        for (natural_16_bit i = 0U; i < synapses_to_muscles_kinds; ++i)
        {
            natural_32_bit const coef = coefs.at((i + 2U) % coefs.size()) +
                                        coefs.at((i + 4U) % coefs.size());
            num_synapses_to_muscles_of_synapse_kind.push_back(2U + coef);
        }

        std::rotate(coefs.begin(), coefs.begin() + 1, coefs.end());

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
                dump_spread_synapses_matrix(
                        test_counter,
                        source_kind,
                        target_kind,
                        diameter_x,
                        diameter_y,
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

        generate_table_of_spread_synapses_matrices(
                test_counter,
                static_tissue,
                diameter_x,
                diameter_y
                );

        std::unordered_map<natural_32_bit,natural_64_bit> correct_summary_distribution;
        natural_64_bit  num_cells_without_input = 0ULL;
        natural_64_bit  num_connected_synapses = 0ULL;
        natural_64_bit  num_not_connected_synapses = 0ULL;
        natural_64_bit  num_connected_input_synapses = 0ULL;

        for (natural_32_bit x = 0U; x < static_tissue->num_cells_along_x_axis(); ++x)
            for (natural_32_bit y = 0U; y < static_tissue->num_cells_along_y_axis(); ++y)
                for (natural_32_bit c = 0U; c < static_tissue->num_cells_along_columnar_axis(); ++c)
                {
                    natural_32_bit const num_synapses =
                            static_tissue->num_synapses_in_territory_of_cell_kind(
                                    static_tissue->compute_kind_of_cell_from_its_position_along_columnar_axis(c)
                                    );

                    natural_32_bit  force_signal_delivery_counter = 0U;
                    natural_32_bit  num_connected_to_current_cell = 0U;
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

                        if (force_signal_delivery_counter == 0U)
                            territorial_state = cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY;
                        force_signal_delivery_counter = (force_signal_delivery_counter + 1U) % 2U;

                        bits_reference const  ref_to_territorial_state =
                                dynamic_tissue->find_bits_of_territorial_state_of_synapse_in_tissue(x,y,c,s);

                        value_to_bits(territorial_state,ref_to_territorial_state);

                        if (territorial_state == cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY)
                        {
                            ++num_connected_to_current_cell;
                            ++num_connected_synapses;

                            cellab::tissue_coordinates const  source_coords =
                                    cellab::get_coordinates_of_source_cell_of_synapse_in_tissue(
                                            dynamic_tissue,
                                            cellab::tissue_coordinates{x,y,c},
                                            s
                                            );
                            if (source_coords.get_coord_along_columnar_axis() >= static_tissue->num_cells_along_columnar_axis())
                                ++num_connected_input_synapses;

                        }
                        else
                            ++num_not_connected_synapses;

                    }

                    if (num_connected_to_current_cell == 0U)
                        ++num_cells_without_input;

                    std::unordered_map<natural_32_bit,natural_64_bit>::iterator const  it =
                            correct_summary_distribution.find(num_connected_to_current_cell);
                    if (it == correct_summary_distribution.end())
                        correct_summary_distribution.insert({num_connected_to_current_cell,1ULL});
                    else
                        ++it->second;

                    TEST_PROGRESS_UPDATE();
                }

        for (natural_32_bit  i = 0U; i < static_tissue->num_synapses_to_muscles(); ++i)
        {
            bits_reference  bits_of_coords =
                    dynamic_tissue->find_bits_of_coords_of_source_cell_of_synapse_to_muscle(i);
            natural_8_bit const  num_bits = dynamic_tissue->num_bits_per_source_cell_coordinate();
            value_to_bits(
                    get_random_natural_32_bit_in_range(0U, static_tissue->num_cells_along_x_axis() - 1U),
                    bits_of_coords, 0U, num_bits);
            value_to_bits(
                    get_random_natural_32_bit_in_range(0U, static_tissue->num_cells_along_y_axis() - 1U),
                    bits_of_coords, num_bits, num_bits);
            value_to_bits(
                    get_random_natural_32_bit_in_range(0U, static_tissue->num_cells_along_columnar_axis() - 1U),
                    bits_of_coords, num_bits + num_bits, num_bits);
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

        std::unordered_map<natural_32_bit,natural_64_bit> summary_distribution;
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  summary_distribution_matrix;
        for (cellab::kind_of_cell  i = 0U; i < static_tissue->num_kinds_of_tissue_cells(); ++i)
        {
            std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  distribution_of_in_degrees;
            cellconnect::compute_in_degrees_of_tissue_cells_of_given_kind(
                        dynamic_tissue,
                        i,
                        true,
                        false,
                        num_rows_in_distribution_matrix,
                        num_columns_in_distribution_matrix,
                        num_threads,
                        distribution_of_in_degrees,
                        cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
                        );

            dump_distribution_matrix(
                        distribution_of_in_degrees,
                        test_counter,
                        i,
                        num_columns_in_distribution_matrix,
                        true
                        );

            cellconnect::add_degree_distributions(distribution_of_in_degrees,summary_distribution_matrix);

            TEST_PROGRESS_UPDATE();
        }

        dump_distribution_matrix(
                    summary_distribution_matrix,
                    test_counter,
                    num_columns_in_distribution_matrix,
                    true
                    );

        cellconnect::add_degree_distributions(summary_distribution_matrix,summary_distribution);

        TEST_SUCCESS(
            (summary_distribution.count(0U) == 0ULL && num_cells_without_input == 0ULL) ||
            summary_distribution.find(0U)->second == num_cells_without_input
            );
        TEST_SUCCESS(correct_summary_distribution.size() == summary_distribution.size());
        natural_64_bit  num_cells = 0ULL;
        natural_64_bit  num_in_synapses = 0ULL;
        for (auto const& elem : correct_summary_distribution)
        {
            auto const  it = summary_distribution.find(elem.first);
            TEST_SUCCESS(elem == *it);
            num_cells += it->second;
            num_in_synapses += (natural_64_bit)it->first * it->second;

        }
        TEST_SUCCESS(num_cells == cellab::num_tissue_cells_in_tissue(static_tissue));
        TEST_SUCCESS(num_in_synapses == num_connected_synapses);
        TEST_SUCCESS(num_connected_synapses + num_not_connected_synapses ==
                     cellab::num_synapses_in_all_columns(static_tissue));

        dump_distribution(
                    summary_distribution,
                    test_counter,
                    true
                    );

        summary_distribution.clear();
        summary_distribution_matrix.clear();
        for (cellab::kind_of_cell  i = 0U; i < static_tissue->num_kinds_of_tissue_cells(); ++i)
        {
            std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >  distribution_of_out_degrees;
            cellconnect::compute_out_degrees_of_tissue_cells_of_given_kind(
                        dynamic_tissue,
                        i,
                        false,
                        num_rows_in_distribution_matrix,
                        num_columns_in_distribution_matrix,
                        num_threads,
                        distribution_of_out_degrees,
                        cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
                        );

            dump_distribution_matrix(
                        distribution_of_out_degrees,
                        test_counter,
                        i,
                        num_columns_in_distribution_matrix,
                        false
                        );

            cellconnect::add_degree_distributions(distribution_of_out_degrees,summary_distribution_matrix);

            TEST_PROGRESS_UPDATE();
        }

        dump_distribution_matrix(
                    summary_distribution_matrix,
                    test_counter,
                    num_columns_in_distribution_matrix,
                    false
                    );

        cellconnect::add_degree_distributions(summary_distribution_matrix,summary_distribution);

        num_cells = 0ULL;
        natural_64_bit  num_out_synapses = 0ULL;
        for (auto const& elem : summary_distribution)
        {
            auto const  it = summary_distribution.find(elem.first);
            num_cells += it->second;
            num_out_synapses += (natural_64_bit)it->first * it->second;

        }
        TEST_SUCCESS(num_cells == cellab::num_tissue_cells_in_tissue(static_tissue));
        TEST_SUCCESS(num_out_synapses == num_connected_synapses - num_connected_input_synapses +
                                         static_tissue->num_synapses_to_muscles());
        TEST_SUCCESS(num_connected_synapses + num_not_connected_synapses ==
                     cellab::num_synapses_in_all_columns(static_tissue));

        dump_distribution(
                    summary_distribution,
                    test_counter,
                    false
                    );

        dump_src_coords_setup_matrix(test_counter,static_tissue,matrix_fill_src_coords);
        generate_root_html_file_for_tissue(test_counter,static_tissue);

        ++test_counter;
    }

    generate_root_html_file(test_counter);

    TEST_PROGRESS_HIDE();

    TEST_PRINT_STATISTICS();
}
