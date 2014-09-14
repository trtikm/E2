#ifndef EFLOOP_ACCESS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED
#   define EFLOOP_ACCESS_TO_SYNAPSES_TO_MUSCLES_HPP_INCLUDED

#include <cellab/neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/instance_wrapper.hpp>
#include <utility/assumptions.hpp>
#include <typeinfo>
#include <utility>
#include <memory>
#include <mutex>

namespace efloop {


struct access_to_synapses_to_muscles
{
    explicit access_to_synapses_to_muscles(
            std::shared_ptr<cellab::neural_tissue> const  neural_tissue,
            std::mutex* const  mutex_to_synapses_to_muscles  = nullptr
            );

    natural_16_bit  num_kinds_of_cells() const;
    natural_16_bit  num_kinds_of_tissue_cells() const;

    bool  is_it_kind_of_tissue_cell(cellab::kind_of_cell cell_kind) const;
    bool  is_it_kind_of_sensory_cell(cellab::kind_of_cell cell_kind) const;

    natural_32_bit num_synapses_to_muscles() const;

    std::pair<bits_const_reference,cellab::kind_of_cell>  get_bits_of_synapse_to_muscle(
            natural_32_bit const index_of_synapse_to_muscle
            ) const;

    template<typename class_synapse>
    cellab::kind_of_cell  read_synapse_to_muscle(natural_32_bit const index_of_synapse_to_muscle,
                                                 instance_wrapper<class_synapse>& storage_for_synapse) const;

private:
    std::shared_ptr<cellab::neural_tissue> m_neural_tissue;
    std::mutex*  m_mutex_to_synapses_to_muscles;
    static std::mutex  void_mutex;
};

template<typename class_synapse>
cellab::kind_of_cell  access_to_synapses_to_muscles::read_synapse_to_muscle(
        natural_32_bit const index_of_synapse_to_muscle,
        instance_wrapper<class_synapse>& storage_for_synapse) const
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_synapses() == typeid(class_synapse).hash_code());
    ASSUMPTION(index_of_synapse_to_muscle < num_synapses_to_muscles());
    std::pair<bits_const_reference,cellab::kind_of_cell> bits_and_kind =
            get_bits_of_synapse_to_muscle( index_of_synapse_to_muscle );
    storage_for_synapse.construct_instance( bits_and_kind.first );
    return bits_and_kind.second;
}


}

#endif
