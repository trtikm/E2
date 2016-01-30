#ifndef CELLCONNECT_DUMP_HPP_INCLUDED
#   define CELLCONNECT_DUMP_HPP_INCLUDED

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
        std::string const& chapter_name = "",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const& description = "",
                //!< A text to be placed into <p></p> right after the chapter name
        std::string const& title = ""
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
        std::string const& chapter_name = "",
                //!< Name of the only chapter in the file. It will be place into <h2></h2>.
        std::string const& description = "",
                //!< A text to be placed into <p></p> right after the chapter name
        std::string const& title = ""
                //!< Title of whole HTML file
        );


}

#endif
