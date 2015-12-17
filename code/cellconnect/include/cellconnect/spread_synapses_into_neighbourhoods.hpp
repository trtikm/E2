#ifndef CELLCONNECT_SPREAD_SYNAPSES_INTO_NEIGHBOURHOODS_HPP_INCLUDED
#   define CELLCONNECT_SPREAD_SYNAPSES_INTO_NEIGHBOURHOODS_HPP_INCLUDED

#   include <cellconnect/column_shift_function.hpp>
#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>

namespace cellconnect {


/**
 * It checks whether the tissue satisfies neccesary conditions for application of the function
 * 'spread_synapses_into_neighbourhoods' declared bellow. Basically, the tissue must be set-up
 * by the algorithm 'cellconnect::fill_coords_of_source_cells_of_synapses_in_tissue' before
 * any call of that function. Here we check for properties of the tissue guaranteed by that
 * algorithm.
 *
 * Observe the correspondence between paramenters of this checking function and the function
 * 'spread_synapses_into_neighbourhoods'. If you call this check for different values of
 * the parementers (possibly except 'num_threads_avalilable_for_computation') then for the
 * function 'spread_synapses_into_neighbourhoods', then a call to the function
 * 'spread_synapses_into_neighbourhoods' may produce wrong results (because the necessary
 * conditions actually was not checked and so they can be violated).
 */
bool  check_consistency_of_matrix_and_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cells_of_synapses,
        cellab::kind_of_cell const  kind_of_source_cells_of_synapses,
        std::vector<natural_32_bit> const&  matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


/**
 * It spreads synapses between all tissue cells of a given source kind 'kind_of_source_cells_of_synapses' and
 * a target kind 'kind_of_target_cells_of_synapses' amongst all columns in the tissue according to the
 * specification matrix 'matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood' and
 * the column-shift function 'move_to_target_column'.
 *
 * Details can be found in the documentation:
 *      file:///<E2-root-dir>/doc/project_documentation/cellconnect/cellconnect.html#spreading_synapses
 */
void  spread_synapses_into_neighbourhoods(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cells_of_synapses,
        cellab::kind_of_cell const  kind_of_source_cells_of_synapses,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        //! A matrix of diameter_x rows and diameter_y columns. Row-axis is aligmend with x-axis of the neural tissue.
        //! The matrix is stored in the vector in the column-major order.
        std::vector<natural_32_bit> const&  matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood,
        cellconnect::column_shift_function const&  move_to_target_column,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
