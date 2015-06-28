#ifndef CELLCONNECT_FILL_DELIMETER_LISTS_HPP_INCLUDED
#   define CELLCONNECT_FILL_DELIMETER_LISTS_HPP_INCLUDED

#   include <cellab/dynamic_state_of_neural_tissue.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <memory>

namespace cellconnect {


void  fill_delimeters_between_territorial_lists(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        natural_32_bit const  num_threads_avalilable_for_computation
        );


}

#endif
