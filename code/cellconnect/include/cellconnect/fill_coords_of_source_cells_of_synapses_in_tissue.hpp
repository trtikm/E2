#ifndef CELLCONNECT_FILL_COORDS_OF_SOURCE_CELLS_OF_SYNAPSES_IN_TISSUE_HPP_INCLUDED
#   define CELLCONNECT_FILL_COORDS_OF_SOURCE_CELLS_OF_SYNAPSES_IN_TISSUE_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>

namespace cellconnect {


void  fill_coords_of_source_cells_of_synapses_in_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> const&  matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
