#include <cellconnect/dump.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <ostream>
#include <cstdlib>

namespace cellconnect { namespace {



}}

namespace cellconnect {


std::ostream&  dump_degrees_distribution_to_gnuplot_plot(
        std::ostream&  output_stream,
        std::unordered_map<natural_32_bit,natural_64_bit> const&  distribution_of_degrees,
        std::string const&  title,
        std::string const&  output_file_path_name_inside_the_script,
        natural_16_bit const  plot_width_in_pixels,
        natural_16_bit const  plot_height_in_pixels,
        bool const  show_grid,
        float_32_bit const  boxwidth_scale,
        natural_8_bit const  fill_pattern_type,
        std::string const&  fill_colour_name,
        std::string const&  label_for_x_axis,
        std::string const&  label_for_y_axis,
        std::string const&  font_name,
        natural_8_bit const  font_size_in_points
        )
{
    TMPROF_BLOCK();

    if (!title.empty())
        output_stream << "set title \"" << title << "\"\n";

    output_stream << "set terminal svg font '" << font_name << "," << (natural_32_bit)font_size_in_points << "'";
    if (plot_width_in_pixels != 0U && plot_height_in_pixels != 0U)
        output_stream << " size " << plot_width_in_pixels << "," << plot_height_in_pixels;
    output_stream << "\n";

    if (!output_file_path_name_inside_the_script.empty())
        output_stream << "set output '" << output_file_path_name_inside_the_script << "'\n";

    output_stream << "set style fill pattern " << (natural_32_bit)fill_pattern_type << " border\n"
                     "set boxwidth " << boxwidth_scale << " absolute\n"
                     ;

    if (show_grid)
        output_stream << "set grid\n";

    if (!label_for_x_axis.empty())
        output_stream << "set xlabel \"" << label_for_x_axis << "\"\n";
    if (!label_for_y_axis.empty())
        output_stream << "set ylabel \"" << label_for_y_axis << "\"\n";

    output_stream << "plot '-' using 1:2 with boxes notitle lt rgb \"" << fill_colour_name << "\"\n";

    for (auto const& elem : distribution_of_degrees)
        output_stream << "    " << elem.first << "  " << elem.second << "\n";

    output_stream << "    e\n";

    return output_stream;
}

int  generate_svg_file_from_plt_file_using_gnuplot(std::string const&  pathname_of_plt_file)
{
    TMPROF_BLOCK();
    return std::system((msgstream() << "gnuplot " << pathname_of_plt_file).get().c_str());
}

std::ostream&  dump_html_file_with_embedded_svg(
        std::ostream&  output_stream,
        std::string const&  pathname_of_svg_file,
        std::string const& chapter_name,
        std::string const& description,
        std::string const& title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
        "    <style type=\"text/css\">\n"
        "        body {\n"
        "            background-color: white;\n"
        "            color: black;\n"
        "            margin-left: auto;\n"
        "            margin-right: auto;\n"
        "        }\n"
        "        p, h1, h2, h3, h4, h5, h6 { font-family:\"Liberation serif\"; }\n"
        "        p {\n"
        "            font-size:12pt;\n"
        "            margin-left: auto;\n"
        "            margin-right: auto;\n"
        "            text-align: justify\n"
        "        }\n"
        "   </style>\n"
        "</head>\n"
        "<body>\n"
        "<h2>" << chapter_name << "</h2>\n"
        "<p>" << description << "</p>\n"
        "<img src=\"" << pathname_of_svg_file << "\" alt=\"[" << pathname_of_svg_file << "]\">\n"
        "</body>\n"
        "</html>\n"
        ;
    return output_stream;
}

std::ostream&  dump_html_table_with_links_to_distributions_of_individual_regions(
        std::ostream&  output_stream,
        natural_32_bit const  num_rows,
        natural_32_bit const  num_columns,
        std::string const&  common_pathname_prefix_of_all_distribution_files,
        std::string const& chapter_name,
        std::string const& description,
        std::string const& title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        "<h2>" << chapter_name << "</h2>\n"
        "<p>" << description << "</p>\n"
        "<table>\n"
        "<caption>\n"
        "</caption>\n"
        ;

    output_stream <<
        "<tr>\n"
        "<th></th>\n"
        ;
    for (natural_32_bit  i = 0U; i < num_columns; ++i)
        output_stream << "<th>" << i << "</th>\n";
    output_stream << "</tr>\n";

    for (natural_32_bit  i = 0U; i < num_rows; ++i)
    {
        output_stream << "<tr>\n"
            << "<th>" << i << "</th>\n"
            ;
        for (natural_32_bit  j = 0U; j < num_columns; ++j)
            output_stream << "<td>"
                << "<a href=\"" << common_pathname_prefix_of_all_distribution_files
                                << i * num_columns + j << ".html" << "\">*</a>"
                << "</td>\n";
        output_stream << "</tr>\n";
    }

    output_stream <<
        "</table>\n"
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}

std::ostream&  dump_matrix_for_setup_of_source_cell_coordinates_in_tissue_columns(
        std::ostream&  output_stream,
        natural_16_bit const  num_tissue_cell_kinds,
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        std::string const& chapter_name,
        std::string const& description,
        std::string const&  caption,
        std::string const& title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        "<h2>" << chapter_name << "</h2>\n"
        "<p>\n" << description << "</p>\n"
        "<table>\n"
        "<caption>\n" << caption << "</caption>\n"
        ;

    output_stream <<
        "<tr>\n"
        "<th></th>\n"
        ;
    for (cellab::kind_of_cell  i = 0U; i < num_tissue_plus_sensory_cell_kinds; ++i)
        output_stream << "<th>" << i << "</th>\n";
    output_stream << "</tr>\n";

    for (cellab::kind_of_cell i = 0U; i < num_tissue_cell_kinds; ++i)
    {
        output_stream <<
            "<tr>\n"
            << "<th>" << i << "</th>\n"
            ;
        for (cellab::kind_of_cell j = 0U; j < num_tissue_plus_sensory_cell_kinds; ++j)
            output_stream <<
                "<td>"
                << matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds.at(
                            i * num_tissue_plus_sensory_cell_kinds + j
                            )
                << "</td>\n";
        output_stream << "</tr>\n";
    }

    output_stream <<
        "</table>\n"
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}

std::ostream&  dump_spread_synapses_matrix(
        std::ostream&  output_stream,
        natural_32_bit const  num_rows,
        natural_32_bit const  num_columns,
        std::vector<natural_32_bit> const&  spread_synapses_matrix,
        std::string const&  chapter_name,
        std::string const&  description,
        std::string const&  caption,
        std::string const&  title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        "<h2>" << chapter_name << "</h2>\n"
        "<p>\n" << description << "</p>\n"
        "<table>\n"
        "<caption>\n" << caption << "</caption>\n"
        ;

    output_stream <<
        "<tr>\n"
        "<th></th>\n"
        ;
    for (cellab::kind_of_cell i = 0U; i < num_columns; ++i)
        output_stream << "<th>" << i << "</th>\n";
    output_stream << "</tr>\n";

    for (cellab::kind_of_cell i = 0U; i < num_rows; ++i)
    {
        output_stream <<
            "<tr>\n"
            << "<th>" << i << "</th>\n"
            ;
        for (cellab::kind_of_cell j = 0U; j < num_columns; ++j)
            output_stream <<
            "<td>"
            << spread_synapses_matrix.at(j * num_rows + i)
            << "</td>\n";
        output_stream << "</tr>\n";
    }

    output_stream <<
        "</table>\n"
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}

std::ostream&  dump_html_table_with_links_to_matrices_for_spreading_synapses_amongst_columns(
        std::ostream&  output_stream,
        natural_16_bit const  num_tissue_cell_kinds,
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
        std::string const&  common_pathname_prefix_of_all_spreading_matrices,
        std::string const& chapter_name,
        std::string const& description,
        std::string const& title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        "<h2>" << chapter_name << "</h2>\n"
        "<p>" << description << "</p>\n"
        "<table>\n"
        "<caption>\n"
        "</caption>\n"
        ;

    output_stream <<
        "<tr>\n"
        "<th></th>\n"
        ;
    for (natural_32_bit i = 0U; i < num_tissue_cell_kinds; ++i)
        output_stream << "<th>" << i << "</th>\n";
    output_stream << "</tr>\n";

    for (natural_32_bit i = 0U; i < num_tissue_plus_sensory_cell_kinds; ++i)
    {
        output_stream << "<tr>\n"
            << "<th>" << i << "</th>\n"
            ;
        for (natural_32_bit j = 0U; j < num_tissue_cell_kinds; ++j)
            output_stream << "<td>"
                << "<a href=\"" << common_pathname_prefix_of_all_spreading_matrices
                << i * num_tissue_cell_kinds + j << ".html" << "\">*</a>"
                << "</td>\n";
        output_stream << "</tr>\n";
    }

    output_stream <<
        "</table>\n"
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}


std::ostream&  dump_column_shift_function(
        std::ostream&  output_stream,
        column_shift_function const&  column_shift_fn,
        std::string const&  title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        //"<h2>" << chapter_name << "</h2>\n"
        //"<p>\n" << description << "</p>\n"
        ;
    if (column_shift_fn.is_identity_function())
        output_stream << "<p>The column shift function is the identity.</p>\n";
    else
    {
        //output_stream <<
        //    "<table>\n"
        //    "<caption>\n" << caption << "</caption>\n"
        //    ;

        //output_stream <<
        //    "<tr>\n"
        //    "<th></th>\n"
        //    ;
        //for (cellab::kind_of_cell i = 0U; i < num_columns; ++i)
        //    output_stream << "<th>" << i << "</th>\n";
        //output_stream << "</tr>\n";

        //for (cellab::kind_of_cell i = 0U; i < num_rows; ++i)
        //{
        //    output_stream <<
        //        "<tr>\n"
        //        << "<th>" << i << "</th>\n"
        //        ;
        //    for (cellab::kind_of_cell j = 0U; j < num_columns; ++j)
        //        output_stream <<
        //        "<td>"
        //        << spread_synapses_matrix.at(j * num_rows + i)
        //        << "</td>\n";
        //    output_stream << "</tr>\n";
        //}

        //output_stream <<
        //    "</table>\n"
        //    ;
    }
    output_stream <<
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}

std::ostream&  dump_html_table_with_links_to_column_shift_functions(
        std::ostream&  output_stream,
        natural_16_bit const  num_tissue_cell_kinds,
        natural_16_bit const  num_tissue_plus_sensory_cell_kinds,
        std::string const&  common_pathname_prefix_of_all_column_shift_functions,
        std::string const&  chapter_name,
        std::string const&  description,
        std::string const&  title
        )
{
    TMPROF_BLOCK();

    output_stream <<
        "<!DOCTYPE html>\n"
        "<html>\n"
        "<head>\n"
        "    <meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\">\n"
        "    <title>" << title << "</title>\n"
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
        "<h2>" << chapter_name << "</h2>\n"
        "<p>" << description << "</p>\n"
        "<table>\n"
        "<caption>\n"
        "</caption>\n"
        ;

    output_stream <<
        "<tr>\n"
        "<th></th>\n"
        ;
    for (natural_32_bit i = 0U; i < num_tissue_cell_kinds; ++i)
        output_stream << "<th>" << i << "</th>\n";
    output_stream << "</tr>\n";

    for (natural_32_bit i = 0U; i < num_tissue_plus_sensory_cell_kinds; ++i)
    {
        output_stream << "<tr>\n"
            << "<th>" << i << "</th>\n"
            ;
        for (natural_32_bit j = 0U; j < num_tissue_cell_kinds; ++j)
            output_stream << "<td>"
            << "<a href=\"" << common_pathname_prefix_of_all_column_shift_functions
            << i * num_tissue_cell_kinds + j << ".html" << "\">*</a>"
            << "</td>\n";
        output_stream << "</tr>\n";
    }

    output_stream <<
        "</table>\n"
        "</body>\n"
        "</html>\n"
        ;

    return output_stream;
}


}
