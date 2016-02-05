#ifndef CELLCONNECT_DUMP_HPP_INCLUDED
#   define CELLCONNECT_DUMP_HPP_INCLUDED

#   include <cellconnect/column_shift_function.hpp>
#   include <cellab/static_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_map>
#   include <string>
#   include <iosfwd>

namespace cellconnect {


std::ostream&  dump_degrees_distribution_to_gnuplot_plot(
        std::ostream&  output_stream,
        std::unordered_map<natural_32_bit,natural_64_bit> const&  distribution_of_degrees,
        std::string const&  title = "",
        std::string const&  output_file_path_name_inside_the_script = "",
        natural_16_bit const  plot_width_in_pixels = 800U,
        natural_16_bit const  plot_height_in_pixels = 600U,
        bool const  show_grid = true,
        float_32_bit const  boxwidth_scale = 0.9f,
        natural_8_bit const  fill_pattern_type = 5,
        std::string const&  fill_colour_name = "black",
        std::string const&  label_for_x_axis = "In-degrees of cells",
        std::string const&  label_for_y_axis = "Counts of cells",
        std::string const&  font_name = "Liberation serif",
        natural_8_bit const  font_size_in_points = 16U
        );

int  generate_svg_file_from_plt_file_using_gnuplot(std::string const&  pathname_of_plt_file);

std::ostream&  dump_html_file_with_embedded_svg(
        std::ostream&  output_stream,
        std::string const&  pathname_of_svg_file,
                //!< Either full disc path or a relative path from the a desired location
                //!< of the dumped HTML file.
        std::string const&  chapter_name = "",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description = "",
                //!< A text to be placed into <p></p> right after the chapter name
        std::string const&  title = ""
                //!< Title of whole HTML file
        );

std::ostream&  dump_html_table_with_links_to_distributions_of_individual_regions(
        std::ostream&  output_stream,
        natural_32_bit const  num_rows,
                //!< The number of rows in the distribution whose HTML table of regions is dumped here.
        natural_32_bit const  num_columns,
                //!< The number of columns in the distribution whose HTML table of regions is dumped here.
        std::string const&  common_pathname_prefix_of_all_distribution_files,
                //!< It is assumed there are num_rows * num_columns HTML files depicting distributuions
                //!< of individual regions. It is further assumed that all these files reside on the disc
                //!< at the same directory and their names (without the HTML extension) end by a unique
                //!< number from the range 0,...,num_rows * num_columns - 1. This agrument represent
                //!< the longest common prefix of all those files. The common prefix must be either the full
                //!< disc path or a relative path from the a desired location of the dumped HTML file.
                //!< Finally, it is assumed that distribution in the region (i,j) is given by the file:
                //!< 'common_pathname_prefix_of_all_distribution_filesN.html', where N=i*num_columns+j.
        std::string const&  chapter_name = "",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description = "",
                //!< A text to be placed into <p></p> right after the chapter name
        std::string const&  title = ""
                //!< Title of whole HTML file
        );

/**
 * 
 */
std::ostream&  dump_matrix_for_setup_of_source_cell_coordinates_in_tissue_columns(
        std::ostream&  output_stream,
                //!< The passed matrix (the 4th parameter) will be dumped into this stream as a HTML file.
        natural_16_bit const  num_tissue_cell_kinds,
                //!< The number of kinds of tissue cell also identifies of the number of rows in the matrix.
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
                //!< The number of all kinds of cell (tissue and sensotry) also identifies the number of columns
                //!< in the matrix.
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
                //!< The matrix which will be dumped into the output stream in the form of HTML table.
        std::string const&  chapter_name = "Matrix used in setup of source cell coordinates of synapses in the tissue.",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description =
                "The number of rows in the table is the number of kinds of tissue cells and the number of columns\n"
                "is the number of kinds of tissue and sensory cells together. A number\n"
                "<i>a</i><sub><i>i</i>,<i>j</i></sub> in the matrix is a count of synapses projected from <em>each</em>\n"
                "cell of the kind <i>j</i> to territories of cells of the kind <i>j</i>.\n",
                //!< A text to be placed into <p></p> right after the chapter name.
        std::string const&  caption = "",
                //!< A text to be placed into <caption></caption> of the table representing the damped matrix.
        std::string const&  title = "Setup matrix of source coordinates in tissue columns."
                //!< Title of whole HTML file.
        );


std::ostream&  dump_spread_synapses_matrix(
        std::ostream&  output_stream,
                //!< The passed matrix (the 4th parameter) will be dumped into this stream as a HTML file.
        natural_32_bit const  num_rows,
                //!< The number of rows in the spread_synapses_matrix. This dimension is aligned with X tissue axis.
        natural_32_bit const  num_columns,
                //!< The number of columns in the spread_synapses_matrix. This dimension is aligned with Y tissue axis.
        std::vector<natural_32_bit> const&  spread_synapses_matrix,
                //!< The matrix which will be dumped into the output stream in the form of a HTML table.
        std::string const&  chapter_name = "Matrix used in initial spreading synapses in the tissue.",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description =
                "The horisontal axis of the matrix is aligned with the X axis\n"
                "of the tissue. The matrix is internally stored in the column-major order.\n",
                //!< A text to be placed into <p></p> right after the chapter name.
        std::string const&  caption = "",
                //!< A text to be placed into <caption></caption> of the table representing the damped matrix.
        std::string const&  title = "Setup matrix of spreading synapses in the tissue."
                //!< Title of whole HTML file.
        );

std::ostream&  dump_html_table_with_links_to_matrices_for_spreading_synapses_amongst_columns(
        std::ostream&  output_stream,
        natural_16_bit const  num_tissue_cell_kinds,
                //!< The number of kinds of tissue cell also identifies of the number of columns in the table.
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
                //!< The number of all kinds of cell (tissue and sensotry) also identifies the number of rows
                //!< in the table.
        std::string const&  common_pathname_prefix_of_all_spreading_matrices,
                //!< It is assumed there are num_tissue_plus_sensory_cell_kinds * num_tissue_cell_kinds HTML
                //!< files depicting spreading matrices. It is further assumed that all these files reside
                //!< on the disc at the same directory and their names (without the HTML extension) end by a unique 
                //!< number from the range 0,...,num_tissue_plus_sensory_cell_kinds * num_tissue_cell_kinds - 1.
                //!< This agrument represents the longest common prefix of all those files. The common prefix must be
                //!< either a full disc path or a relative path from the a desired location of the dumped HTML file.
                //!< Finally, it is assumed that speading matrix for the source and rarget cell kinds (i,j) is stored
                //!< in the file: 'common_pathname_prefix_of_all_spreading_matricesN.html',
                //!< where N=i*num_tissue_cell_kinds+j.
        std::string const&  chapter_name = "Table of spread matrices of synapses amongst tissue columns",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description =
                "The vertical axis of the table identifies source kinds of cells (includes both\n"
                "tissue and sensory cell kinds) and the horisontal axis represent target kinds of\n"
                "cells (tissue cell kinds only).\n",
                //!< A text to be placed into <p></p> right after the chapter name.
        std::string const&  title = "Table of spread matrices of synases."
                //!< Title of whole HTML file.
        );


std::ostream&  dump_column_shift_function(
        std::ostream&  output_stream,
        column_shift_function const&  column_shift_fn,
        std::string const&  chapter_name = "A column shift function.",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description = "",
                //!< A text to be placed into <p></p> right after the chapter name.
        std::string const&  title = "A column shift function."
                //!< Title of whole HTML file.
        );

std::ostream&  dump_html_table_with_links_to_column_shift_functions(
        std::ostream&  output_stream,
        natural_16_bit const  num_tissue_cell_kinds,
                //!< The number of kinds of tissue cell also identifies of the number of columns in the table.
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
                //!< The number of all kinds of cell (tissue and sensotry) also identifies the number of rows
                //!< in the table.
        std::string const&  common_pathname_prefix_of_all_column_shift_functions,
                //!< It is assumed there are num_tissue_plus_sensory_cell_kinds * num_tissue_cell_kinds HTML
                //!< files depicting column shift functions. It is further assumed that all these files reside
                //!< on the disc at the same directory and their names (without the HTML extension) end by a unique 
                //!< number from the range 0,...,num_tissue_plus_sensory_cell_kinds * num_tissue_cell_kinds - 1.
                //!< This agrument represents the longest common prefix of all those files. The common prefix must be
                //!< either a full disc path or a relative path from the a desired location of the dumped HTML file.
                //!< Finally, it is assumed that the column shift function for the source and rarget cell kinds (i,j)
                //!< is stored in the file: 'common_pathname_prefix_of_all_column_shift_functionsN.html',
                //!< where N=i*num_tissue_cell_kinds+j.
        std::string const&  chapter_name = "Table of column shift functions.",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const&  description =
                "The vertical axis of the table identifies source kinds of cells (includes both\n"
                "tissue and sensory cell kinds) and the horisontal axis represent target kinds of\n"
                "cells (tissue cell kinds only).\n",
                //!< A text to be placed into <p></p> right after the chapter name.
        std::string const&  title = "Table of column shift functions."
                //!< Title of whole HTML file.
        );


}

#endif
