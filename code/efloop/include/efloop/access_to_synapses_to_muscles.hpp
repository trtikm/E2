#ifndef EFLOOP_ACCESS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED
#   define EFLOOP_ACCESS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED

#include <cellab/neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/instance_wrapper.hpp>
#include <utility/assumptions.hpp>
#include <typeinfo>
#include <memory>
#include <mutex>

namespace efloop {


struct access_to_synapses_to_muscles
{
    explicit access_to_synapses_to_muscles(
            std::shared_ptr<cellab::neural_tissue> const  neural_tissue,
            std::mutex* const  mutex_to_synapses_to_muscles  = nullptr
            );

    std::shared_ptr<cellab::static_state_of_neural_tissue const>  get_static_state_of_tissue() const;

    bits_const_reference  get_bits_of_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle) const;

    template<typename class_synapse>
    void  read_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle,
                                 instance_wrapper<class_synapse>& storage_for_synapse) const;

    cellab::kind_of_cell  get_kind_of_source_cell(natural_32_bit const index_of_synapse_to_muscle) const;

private:
    std::shared_ptr<cellab::neural_tissue> m_neural_tissue;
    std::mutex*  m_mutex_to_synapses_to_muscles;
    static std::mutex  void_mutex;
};

template<typename class_synapse>
void  access_to_synapses_to_muscles::read_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle,
        instance_wrapper<class_synapse>& storage_for_synapse) const
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_synapses() == typeid(class_synapse).hash_code());
    ASSUMPTION(index_of_synapse_to_muscle < get_static_state_of_tissue()->num_synapses_to_muscles());
    bits_const_reference bits = get_bits_of_synapse_to_muscle( index_of_synapse_to_muscle );
    storage_for_synapse.construct_instance( bits );
}


}

#endif
