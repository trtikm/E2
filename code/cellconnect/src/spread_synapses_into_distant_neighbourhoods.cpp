#include <cellconnect/spread_synapses_into_distant_neighbourhoods.hpp>
#include <cellab/utilities_for_transition_algorithms.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <thread>
#include <algorithm>
#include <functional>

namespace cellconnect {


void  spread_synapses_into_distant_neighbourhoods(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue> const  dynamic_state_ptr,
        cellab::kind_of_cell const  kind_of_target_cells_of_synapses,
        cellab::kind_of_cell const  kind_of_source_cells_of_synapses,
        natural_32_bit const  diameter_x,
        natural_32_bit const  diameter_y,
        std::vector<natural_32_bit> const&  matrix_of_counts_of_synapses_to_be_spread_into_columns_in_neighbourhood,
        natural_32_bit const  num_threads_avalilable_for_computation
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(dynamic_state_ptr.operator bool());
    ASSUMPTION(num_threads_avalilable_for_computation > 0U);

    std::shared_ptr<cellab::static_state_of_neural_tissue const> const static_state_ptr =
            dynamic_state_ptr->get_static_state_of_neural_tissue();

}



}
