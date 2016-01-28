#ifndef CELLCONNECT_CHECK_FOR_NETWORK_PROPERTIES_HPP_INCLUDED
#   define CELLCONNECT_CHECK_FOR_NETWORK_PROPERTIES_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/territorial_state_of_synapse.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_map>
#   include <memory>
#   include <string>
#   include <iosfwd>

namespace cellconnect {



void  compute_in_degrees_of_tissue_cells_of_given_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        bool const  ignore_delimiters_lists_and_check_territorial_states_of_all_synapses,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  num_threads_avalilable_for_computation,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_in_degrees,
                //!< A output matrix of num_rows_in_output_distribution_matrix x num_columns_in_output_distribution_matrix
                //!< elements. The row-axis is aligmend with x-axis of the neural tissue. The matrix is stored in the vector
                //!< in the row-major order.
        cellab::territorial_state_of_synapse const  territorial_state_to_be_considered =
                cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
        );

void  compute_out_degrees_of_tissue_cells_of_given_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
        natural_32_bit const  num_rows_in_output_distribution_matrix,
        natural_32_bit const  num_columns_in_output_distribution_matrix,
        natural_32_bit const  num_threads_avalilable_for_computation,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  output_matrix_with_distribution_of_out_degrees,
                //!< A output matrix of num_rows_in_output_distribution_matrix x num_columns_in_output_distribution_matrix
                //!< elements. The row-axis is aligmend with x-axis of the neural tissue. The matrix is stored in the vector
                //!< in the row-major order.
        cellab::territorial_state_of_synapse const  territorial_state_to_be_considered =
                cellab::SIGNAL_DELIVERY_TO_CELL_OF_TERRITORY
        );


void  add_degree_distributions(
        std::unordered_map<natural_32_bit,natural_64_bit> const&  addon,
        std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_addon_will_be_added
        );

void  add_degree_distributions(
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
        std::unordered_map<natural_32_bit,natural_64_bit>& distribution_where_all_addons_will_be_added
        );

void  add_degree_distributions(
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
        std::vector< std::unordered_map<natural_32_bit,natural_64_bit> >&  distribution_where_all_addons_will_be_added
        );


std::ostream&  degrees_distribution_to_gnuplot_plot(
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


}

#endif
