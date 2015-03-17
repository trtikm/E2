#ifndef CELLCONNECT_BASIC_ADDITION_OF_SYNAPSES_TO_TERRITORIES_OF_CELLS_HPP_INCLUDED
#   define CELLCONNECT_BASIC_ADDITION_OF_SYNAPSES_TO_TERRITORIES_OF_CELLS_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace cellconnect {


void  basic_addition_of_synapses_to_territories_of_cells(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr
        );


}

#endif
