#ifndef EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED
#   define EFLOOP_EXTERNAL_FEEDBACK_LOOP_HPP_INCLUDED

//#include <cellab/static_state_of_neural_tissue.hpp>
//#include <cellab/dynamic_state_of_neural_tissue.hpp>
//#include <utility/bits_reference.hpp>
#include <utility/basic_numeric_types.hpp>
#include <memory>
//#include <thread>

namespace cellab {
    struct dynamic_state_of_neural_tissue;
}
namespace envlab {
    struct rules_and_logic_of_environment;
}

//namespace efloop { namespace detail {
//    struct synchronisation_barriers;
//}}

//namespace efloop {


//struct read_write_access_to_sensory_cells
//{
//    read_write_access_to_sensory_cells(
//            std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_neural_tissue
//            );
//    natural_32_bit num_sensory_cells() const;
//    cellab::static_state_of_cell const& get_static_state_of_sensory_cell(
//            natural_32_bit const index_of_sensory_cell
//            ) const;
//    bits_reference get_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell) const;
//private:
//    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> m_dynamic_state_of_neural_tissue;
//};

//struct read_access_to_synapses_to_muscles
//{
//    read_access_to_synapses_to_muscles(
//            std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> dynamic_state_of_neural_tissue
//            );
//    natural_32_bit num_synapses_to_muscles() const;
//    cellab::static_state_of_synapse const& get_static_state_of_synapse_to_muscle(
//            natural_32_bit const index_of_synapse_to_muscle
//            ) const;
//    bits_const_reference get_bits_of_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle) const;
//private:
//    std::shared_ptr<cellab::dynamic_state_of_neural_tissue const> m_dynamic_state_of_neural_tissue;
//};

//}


namespace efloop {


struct external_feedback_loop
{
    external_feedback_loop(
            std::shared_ptr<cellab::dynamic_state_of_neural_tissue> dynamic_state_of_neural_tissue,
            std::shared_ptr<envlab::rules_and_logic_of_environment> rules_and_logic_of_environment
            );

    void compute_next_state_of_both_neural_tissue_and_environment(
            natural_32_bit max_number_of_threads_neural_tissue_can_create_and_run_simultaneously,
            natural_32_bit max_number_of_threads_environment_can_create_and_run_simultaneously
            );

private:
    std::shared_ptr<cellab::dynamic_state_of_neural_tissue> m_dynamic_state_of_neural_tissue;
    std::shared_ptr<envlab::rules_and_logic_of_environment> m_rules_and_logic_of_environment;
    //std::shared_ptr<detail::synchronisation_barriers> m_synchronisation_barriers;
};


}

#endif
