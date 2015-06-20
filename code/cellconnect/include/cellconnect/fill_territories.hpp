#ifndef CELLCONNECT_FILL_TERRITORIES_HPP_INCLUDED
#   define CELLCONNECT_FILL_TERRITORIES_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>
#   include <vector>
#   include <functional>

namespace cellconnect {


void  fill_territories(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        std::vector<natural_32_bit> matrix_num_tissue_cell_kinds_x_num_tissue_plus_sensory_cell_kinds,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
