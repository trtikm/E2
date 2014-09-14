#include <cellab/neural_tissue.hpp>
#include <utility/assumptions.hpp>

namespace cellab {


neural_tissue::neural_tissue(
        std::shared_ptr<cellab::dynamic_state_of_neural_tissue>
            dynamic_state_of_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_to_muscle
            transition_function_of_packed_synapse_to_muscle,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_synapse_inside_tissue
            transition_function_of_packed_synapse_inside_tissue,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_signalling
            transition_function_of_packed_signalling,
        single_threaded_in_situ_transition_function_of_packed_dynamic_state_of_cell
            transition_function_of_packed_cell
        )
    : m_dynamic_state_of_tissue(dynamic_state_of_tissue)
    , m_transition_function_of_packed_synapse_to_muscle(transition_function_of_packed_synapse_to_muscle)
    , m_transition_function_of_packed_synapse_inside_tissue(transition_function_of_packed_synapse_inside_tissue)
    , m_transition_function_of_packed_signalling(transition_function_of_packed_signalling)
    , m_transition_function_of_packed_cell(transition_function_of_packed_cell)
    , m_hash_code_of_class_for_cells(typeid(bits_reference).hash_code())
    , m_hash_code_of_class_for_synapses(typeid(bits_reference).hash_code())
    , m_hash_code_of_class_for_signalling(typeid(bits_reference).hash_code())
{
    ASSUMPTION(m_dynamic_state_of_tissue.operator bool());
}

std::shared_ptr<static_state_of_neural_tissue const>
neural_tissue::get_static_state_of_neural_tissue() const
{
    return m_dynamic_state_of_tissue->get_static_state_of_neural_tissue();
}

std::shared_ptr<dynamic_state_of_neural_tissue>
neural_tissue::get_dynamic_state_of_neural_tissue()
{
    return m_dynamic_state_of_tissue;
}

void  neural_tissue::apply_transition_of_synapses_to_muscles(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_synapses_to_muscles(
                get_dynamic_state_of_neural_tissue(),
                m_transition_function_of_packed_synapse_to_muscle,
                num_avalilable_thread_for_creation_and_use
                );
}

void  neural_tissue::apply_transition_of_synapses_of_tissue(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_synapses_of_tissue(
                get_dynamic_state_of_neural_tissue(),
                m_transition_function_of_packed_synapse_inside_tissue,
                num_avalilable_thread_for_creation_and_use
                );
}

void  neural_tissue::apply_transition_of_territorial_lists_of_synapses(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_territorial_lists_of_synapses(
                get_dynamic_state_of_neural_tissue(),
                num_avalilable_thread_for_creation_and_use
                );
}

void  neural_tissue::apply_transition_of_synaptic_migration_in_tissue(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_synaptic_migration_in_tissue(
                get_dynamic_state_of_neural_tissue(),
                num_avalilable_thread_for_creation_and_use
                );
}

void  neural_tissue::apply_transition_of_signalling_in_tissue(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_signalling_in_tissue(
                get_dynamic_state_of_neural_tissue(),
                m_transition_function_of_packed_signalling,
                num_avalilable_thread_for_creation_and_use
                );
}

void  neural_tissue::apply_transition_of_cells_of_tissue(
        natural_32_bit num_avalilable_thread_for_creation_and_use
        )
{
    cellab::apply_transition_of_cells_of_tissue(
                get_dynamic_state_of_neural_tissue(),
                m_transition_function_of_packed_cell,
                num_avalilable_thread_for_creation_and_use
                );
}

std::size_t neural_tissue::get_hash_code_of_class_for_cells() const
{
    return m_hash_code_of_class_for_cells;
}

std::size_t neural_tissue::get_hash_code_of_class_for_synapses() const
{
    return m_hash_code_of_class_for_synapses;
}

std::size_t neural_tissue::get_hash_code_of_class_for_signalling() const
{
    return m_hash_code_of_class_for_signalling;
}


}
