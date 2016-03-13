#ifndef PLOT_HPP_INCLUDED
#   define PLOT_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <utility/timeprof.hpp>
#   include <utility/assumptions.hpp>
#   include <utility/invariants.hpp>
#   include <boost/filesystem/path.hpp>
#   include <boost/filesystem.hpp>
#   include <boost/algorithm/string.hpp>
#   include <ostream>
#   include <fstream>
#   include <string>
#   include <vector>
#   include <type_traits>

namespace plot {


enum struct FILE_FORMAT : natural_8_bit
{
    SVG,
    PNG,
};


}

namespace plot { namespace  detail {


struct  plot_base
{
    plot_base();
    virtual ~plot_base() {}

    std::string const& title() const noexcept { return m_title; }
    std::string& title() noexcept { return m_title; }

    FILE_FORMAT  file_format() const noexcept { return m_file_format; }
    FILE_FORMAT&  file_format() noexcept { return m_file_format; }

    std::string const&  font_name() const noexcept { return m_font_name; }
    std::string&  font_name() noexcept { return m_font_name; }

    natural_8_bit  font_size_in_points() const noexcept { return m_font_size; }
    natural_8_bit&  font_size_in_points() noexcept { return m_font_size; }

    natural_16_bit  width_in_pixels() const noexcept { return m_width; }
    natural_16_bit&  width_in_pixels() noexcept { return m_width; }

    natural_16_bit  height_in_pixels() const noexcept { return m_height; }
    natural_16_bit&  height_in_pixels() noexcept { return m_height; }

    std::string const& x_axis_label() const noexcept { return m_x_axis_label; }
    std::string& x_axis_label() noexcept { return m_x_axis_label; }

    std::string const& y_axis_label() const noexcept { return m_y_axis_label; }
    std::string& y_axis_label() noexcept { return m_y_axis_label; }

    bool  show_grid() const noexcept { return m_show_grid; }
    bool&  show_grid() noexcept { return m_show_grid; }

    bool  show_legend() const noexcept { return m_show_legend; }
    bool&  show_legend() noexcept { return m_show_legend; }

    std::ostream&  operator<<(std::ostream&  ostr) const;

private:
    std::string  m_title;
    FILE_FORMAT  m_file_format;
    std::string  m_font_name;
    natural_8_bit  m_font_size;
    natural_16_bit  m_width;
    natural_16_bit  m_height;
    std::string  m_x_axis_label;
    std::string  m_y_axis_label;
    bool  m_show_grid;
    bool  m_show_legend;
};


}}

namespace plot {


enum struct DRAW_STYLE_2D : natural_8_bit
{
    POINTS_DOT,
    POINTS_PLUS,
    POINTS_CROSS,
    POINTS_STAR,
    POINTS_EMPTY_BOX,
    POINTS_FULL_BOX,
    POINTS_EMPTY_CIRCLE,
    POINTS_FULL_CIRCLE,
    POINTS_EMPTY_TRIANGLE_UP,
    POINTS_FULL_TRIANGLE_UP,
    POINTS_EMPTY_TRIANGLE_DOWN,
    POINTS_FULL_TRIANGLE_DOWN,
    POINTS_EMPTY_DIAMOND,
    POINTS_FULL_DIAMOND,
    POINTS_EMPTY_PENTAGON,
    POINTS_FULL_PENTAGON,

    LINES_DOTS,
    LINES_SOLID,
    LINES_LONG_DASHES,
    LINES_SHORT_DASHES,
    LINES_DENSE_DOTS,
    LINES_DASH_DOTS,
    LINES_SHORT_DASH_DOTS,
    LINES_DASH_DASHES,
    LINES_DASH_DOT_DOTS,
};


template<typename element_type__ = float_64_bit>
struct  functions2d : public detail::plot_base
{
    using  element_type = element_type__;

    static_assert(std::is_floating_point<element_type>::value || std::is_integral<element_type>::value,
                  "Only numeric types are allowed for the template parameter.");

    explicit functions2d(natural_32_bit const  num_functions = 1U);

    std::vector<element_type> const& x() const noexcept { return m_x_axis_data; }
    std::vector<element_type>& x() noexcept { return m_x_axis_data; }

    natural_64_bit num_functions() const noexcept { return m_functions.size(); }

    std::vector<element_type> const& f(natural_32_bit const function_index = 0U) const;
    std::vector<element_type>& f(natural_32_bit const function_index = 0U);

    std::vector<DRAW_STYLE_2D> const& f_style(natural_32_bit const function_index = 0U) const;
    std::vector<DRAW_STYLE_2D>& f_style(natural_32_bit const function_index = 0U);

    std::string const& f_legend(natural_32_bit const function_index = 0U) const;
    std::string& f_legend(natural_32_bit const function_index = 0U);

    std::ostream&  operator<<(std::ostream&  ostr) const;

private:
    std::vector<element_type>  m_x_axis_data;
    std::vector< std::vector<element_type> >  m_functions;
    std::vector< std::vector<DRAW_STYLE_2D> >  m_draw_styles;      //!< One function may be drawn several times;
                                                                   //!< Each time with a different draw style.
    std::vector<std::string>  m_legends;
};

template<typename element_type__>
functions2d<element_type__>::functions2d(natural_32_bit const  num_functions = 1U)
    : m_x_axis_data()
    , m_functions(num_functions,{})
    , m_draw_styles(num_functions,{})
    , m_legends(num_functions,"")
{
    ASSUMPTION(num_functions > 0U);
    ASSUMPTION(m_functions.size() == num_functions &&
               m_draw_styles.size() == num_functions &&
               m_legends.size() == num_functions
               );
}

template<typename element_type__>
std::vector<element_type__> const& functions2d<element_type__>::f(natural_32_bit const function_index = 0U) const
{
    ASSUMPTION(function_index < m_functions.size());
    return m_functions.at(function_index);
}

template<typename element_type__>
std::vector<element_type__>& functions2d<element_type__>::f(natural_32_bit const function_index = 0U)
{
    ASSUMPTION(function_index < m_functions.size());
    return m_functions.at(function_index);
}

template<typename element_type__>
std::vector<DRAW_STYLE_2D> const& functions2d<element_type__>::f_style(natural_32_bit const function_index = 0U) const
{
    ASSUMPTION(function_index < m_draw_styles.size());
    return m_draw_styles.at(function_index);
}

template<typename element_type__>
std::vector<DRAW_STYLE_2D>& functions2d<element_type__>::f_style(natural_32_bit const function_index = 0U)
{
    ASSUMPTION(function_index < m_draw_styles.size());
    return m_draw_styles.at(function_index);
}

template<typename element_type__>
std::string const& functions2d<element_type__>::f_legend(natural_32_bit const function_index = 0U) const
{
    ASSUMPTION(function_index < m_legends.size());
    return m_legends.at(function_index);
}

template<typename element_type__>
std::string& functions2d<element_type__>::f_legend(natural_32_bit const function_index = 0U)
{
    ASSUMPTION(function_index < m_legends.size());
    return m_legends.at(function_index);
}

template<typename element_type__>
std::ostream&  functions2d<element_type__>::operator<<(std::ostream&  ostr) const
{
    ASSUMPTION(!x().empty());
    ASSUMPTION(
            [](std::vector< std::vector<element_type__> > const&  f, natural_64_bit const  size) {
                for (auto const&  data : f)
                    if (data.size() != size)
                        return false;
                return true;
            }(m_functions,x().size())
            );

    detail::plot_base::operator<<(ostr);

    ostr << "plot ";
    for (natural_32_bit  j = 0U; j < num_functions(); ++j)
    {
        for (natural_32_bit  k = 0U; k < f_style(j).size(); ++k)
        {
            ostr << "'-' using 1:2 with ";
            switch (f_style(j).at(k))
            {
            case DRAW_STYLE_2D::POINTS_DOT: ostr << "points pointtype 0"; break;
            case DRAW_STYLE_2D::POINTS_PLUS: ostr << "points pointtype 1"; break;
            case DRAW_STYLE_2D::POINTS_CROSS: ostr << "points pointtype 2"; break;
            case DRAW_STYLE_2D::POINTS_STAR: ostr << "points pointtype 3"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_BOX: ostr << "points pointtype 4"; break;
            case DRAW_STYLE_2D::POINTS_FULL_BOX: ostr << "points pointtype 5"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_CIRCLE: ostr << "points pointtype 6"; break;
            case DRAW_STYLE_2D::POINTS_FULL_CIRCLE: ostr << "points pointtype 7"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_TRIANGLE_UP: ostr << "points pointtype 8"; break;
            case DRAW_STYLE_2D::POINTS_FULL_TRIANGLE_UP: ostr << "points pointtype 9"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_TRIANGLE_DOWN: ostr << "points pointtype 10"; break;
            case DRAW_STYLE_2D::POINTS_FULL_TRIANGLE_DOWN: ostr << "points pointtype 11"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_DIAMOND: ostr << "points pointtype 12"; break;
            case DRAW_STYLE_2D::POINTS_FULL_DIAMOND: ostr << "points pointtype 13"; break;
            case DRAW_STYLE_2D::POINTS_EMPTY_PENTAGON: ostr << "points pointtype 14"; break;
            case DRAW_STYLE_2D::POINTS_FULL_PENTAGON: ostr << "points pointtype 15"; break;
            case DRAW_STYLE_2D::LINES_DOTS: ostr << "lines linetype 0"; break;
            case DRAW_STYLE_2D::LINES_SOLID: ostr << "lines linetype 1"; break;
            case DRAW_STYLE_2D::LINES_LONG_DASHES: ostr << "lines linetype 2"; break;
            case DRAW_STYLE_2D::LINES_SHORT_DASHES: ostr << "lines linetype 3"; break;
            case DRAW_STYLE_2D::LINES_DENSE_DOTS: ostr << "lines linetype 4"; break;
            case DRAW_STYLE_2D::LINES_DASH_DOTS: ostr << "lines linetype 5"; break;
            case DRAW_STYLE_2D::LINES_SHORT_DASH_DOTS: ostr << "lines linetype 6"; break;
            case DRAW_STYLE_2D::LINES_DASH_DASHES: ostr << "lines linetype 7"; break;
            case DRAW_STYLE_2D::LINES_DASH_DOT_DOTS: ostr << "lines linetype 8"; break;
            default: UNREACHABLE();
            }
            ostr << " ";
            if (show_legend() && k == 0 && num_functions() > 1U && !f_legend(j).empty())
                ostr << "title '" << f_legend(j) << "' ";
            else
                ostr << "notitle ";
            ostr << (k+1ULL < f_style(j).size() ? ", " : "");
        }
        ostr << (j+1ULL < num_functions() ? ", " : "");
    }
    ostr << "\n";
    for (natural_32_bit  j = 0ULL; j < num_functions(); ++j)
        for (natural_32_bit  k = 0U; k < f_style(j).size(); ++k)
        {
            for (natural_64_bit  i = 0ULL; i < x().size(); ++i)
                ostr << "    " << x().at(i) << "   " << f(j).at(i) << "\n";
            ostr << "    e\n";
        }

    return ostr;
}


void  draw(boost::filesystem::path const&  script_file_pathname,
           boost::filesystem::path const&  output_plot_pathname,
           boost::filesystem::path const&  gnuplot_install_dir = boost::filesystem::path()
           );

template<typename  element_type, template<typename > class  plot_type>
void  draw(plot_type<element_type> const&  plt,
           boost::filesystem::path const&  output_script_file_pathname,
           boost::filesystem::path const&  output_plot_pathname = boost::filesystem::path(),
           boost::filesystem::path const&  gnuplot_install_dir = boost::filesystem::path()
           )
{
    static_assert(std::is_same< plot_type<element_type>,functions2d<element_type> >::value,
                  "We expect 'a plot' to be drown.");

    ASSUMPTION(!output_script_file_pathname.empty());
    {
        boost::filesystem::create_directories(output_script_file_pathname.parent_path());
        std::ofstream  ofs(output_script_file_pathname.string(),std::ostream::out);
        plt.operator<<(ofs);
    }
    if (!output_plot_pathname.empty())
    {
        ASSUMPTION(
                [](FILE_FORMAT const  format, std::string  extension) {
                    boost::algorithm::to_lower(extension);
                    switch (format)
                    {
                    case FILE_FORMAT::SVG: return extension == ".svg";
                    case FILE_FORMAT::PNG:  return extension == ".png";
                    default: UNREACHABLE();
                    }
                }(plt.file_format(),output_plot_pathname.extension().string())
                );
        draw(output_script_file_pathname,output_plot_pathname,gnuplot_install_dir);
    }
}


}

#endif
