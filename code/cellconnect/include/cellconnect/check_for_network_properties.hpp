#ifndef CELLCONNECT_CHECK_FOR_NETWORK_PROPERTIES_HPP_INCLUDED
#   define CELLCONNECT_CHECK_FOR_NETWORK_PROPERTIES_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <cellab/territorial_state_of_synapse.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_map>
#   include <memory>

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


void  add_degree_distributions(std::unordered_map<natural_32_bit,natural_64_bit> const&  addon,
                               std::unordered_map<natural_32_bit,natural_64_bit>& ditribution_where_addon_will_be_added
                               );

void  add_degree_distributions(std::vector< std::unordered_map<natural_32_bit,natural_64_bit> > const&  addons,
                               std::unordered_map<natural_32_bit,natural_64_bit>& ditribution_where_all_addons_will_be_added
                               );

}

#endif
