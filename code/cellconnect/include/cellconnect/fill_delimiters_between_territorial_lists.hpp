#ifndef CELLCONNECT_FILL_DELIMITER_LISTS_HPP_INCLUDED
#   define CELLCONNECT_FILL_DELIMITER_LISTS_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace cellconnect {


void  fill_delimiters_between_territorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


void  fill_delimiters_between_territorial_lists_for_cell_kind(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_cells_to_be_considered,
                //!< Only delimiters in territories of tissue cells of this kind will be filled.
                //!< I.e. all other delimiters will be skipped (not modified).
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
