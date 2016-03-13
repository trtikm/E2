#include <plot/plot.hpp>
#include <utility/msgstream.hpp>
#include <cstdlib>

namespace plot { namespace  detail {


plot_base::plot_base()
    : m_title()
    , m_file_format(FILE_FORMAT::SVG)
    , m_font_name("Liberation serif")
    , m_font_size(16U)
    , m_width(640U)
    , m_height(480U)
    , m_x_axis_label()
    , m_y_axis_label()
    , m_show_grid(true)
    , m_show_legend(true)
{}

std::ostream&  plot_base::operator<<(std::ostream&  ostr) const
{
    if (!title().empty())
        ostr << "set title \"" << title() << "\"\n";

    ostr << "set terminal ";
    switch (file_format())
    {
    case FILE_FORMAT::SVG: ostr << "svg"; break;
    case FILE_FORMAT::PNG: ostr << "png"; break;
    default: UNREACHABLE();
    }
    ostr << " font '" << font_name() << "," << (natural_32_bit)font_size_in_points() << "'"
         << " size " << width_in_pixels() << "," << height_in_pixels() << "\n";

    if (show_grid())
        ostr << "set grid\n";

    if (!x_axis_label().empty())
        ostr << "set xlabel \"" << x_axis_label() << "\"\n";
    if (!y_axis_label().empty())
        ostr << "set ylabel \"" << y_axis_label() << "\"\n";

    return ostr;
}


}}

namespace plot {


void  draw(boost::filesystem::path const&  script_file_pathname,
           boost::filesystem::path const&  output_plot_pathname,
           boost::filesystem::path const&  gnuplot_install_dir)
{
    ASSUMPTION(boost::filesystem::exists(script_file_pathname));
    ASSUMPTION(boost::filesystem::is_regular_file(script_file_pathname));

    boost::filesystem::create_directories(output_plot_pathname.parent_path());
    boost::filesystem::path const  gnuplot =
            (gnuplot_install_dir.empty() ? boost::filesystem::path() :
                                          gnuplot_install_dir / "bin")
            / "gnuplot";
    std::string const  command_line =
            msgstream() << gnuplot.string() << " \""
                        << boost::filesystem::absolute(script_file_pathname).string()
                        << "\" > \""
                        << boost::filesystem::absolute(output_plot_pathname).string()
                        << "\"";
    std::system(command_line.c_str());
}


}
