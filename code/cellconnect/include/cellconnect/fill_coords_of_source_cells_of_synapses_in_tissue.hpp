#ifndef CELLCONNECT_FILL_COORDS_OF_SOURCE_CELLS_OF_SYNAPSES_IN_TISSUE_HPP_INCLUDED
#   define CELLCONNECT_FILL_COORDS_OF_SOURCE_CELLS_OF_SYNAPSES_IN_TISSUE_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>

namespace cellconnect {


/**
 * It sets-up source coordinates of synapses in all columns in the passed tissue. Each column is set-up
 * independently according to the passed matrix. The matrix is stored in the vector in the row-major
 * order. The passed number of available threads identify maximal number of simultaneously set-up columns.
 *
 * Details can be found in the documentation:
 *      file:///<E2-root-dir>/doc/cellconnect/cellconnect.html#column_setup
 */
void  fill_coords_of_source_cells_of_synapses_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
