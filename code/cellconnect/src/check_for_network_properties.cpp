#include <cellconnect/check_for_network_properties.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <ostream>

namespace cellconnect { namespace {



}}

namespace cellconnect {


void  add_degree_distributions(std::unordered_map<natural_32_bit,natural_64_bit> const&  addon,
                               std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_addon_will_be_added
                               )
{
    TMPROF_BLOCK();

    for (auto const& elem : addon)
    {
        auto const  it = distribution_where_addon_will_be_added.find(elem.first);
        if (it == distribution_where_addon_will_be_added.cend())
            distribution_where_addon_will_be_added.insert(elem);
        else
            it->second += elem.second;
    }
}

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_all_addons_will_be_added
                               )
{
    TMPROF_BLOCK();

    for (auto const& addon : addons)
        add_degree_distributions(addon,distribution_where_all_addons_will_be_added);
}

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&
                                        distribution_where_all_addons_will_be_added
                               )
{
    TMPROF_BLOCK();

    ASSUMPTION(distribution_where_all_addons_will_be_added.empty() ||
               addons.size() == distribution_where_all_addons_will_be_added.size());

    if (distribution_where_all_addons_will_be_added.empty())
        distribution_where_all_addons_will_be_added.resize(addons.size());
    for (natural_64_bit  i = 0ULL; i < addons.size(); ++i)
        add_degree_distributions(addons.at(i),distribution_where_all_addons_will_be_added.at(i));
}

std::ostream&  degrees_distribution_to_gnuplot_plot(
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
                  << "set boxwidth " << boxwidth_scale << " absolute\n";

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


}
