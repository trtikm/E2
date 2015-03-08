#ifndef EFLOOP_ACCESS_TO_SENSORY_CELLS_HPP_INCLUDED
#   define EFLOOP_ACCESS_TO_SENSORY_CELLS_HPP_INCLUDED

#include <cellab/neural_tissue.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/instance_wrapper.hpp>
#include <utility/assumptions.hpp>
#include <typeinfo>
#include <utility>
#include <memory>
#include <mutex>

namespace efloop {


struct access_to_sensory_cells
{
    explicit access_to_sensory_cells(
            std::shared_ptr<cellab::neural_tissue> const  neural_tissue,
            std::mutex* const  mutex_to_sensory_cells = nullptr
            );

    std::shared_ptr<cellab::static_state_of_neural_tissue const>  get_static_state_of_tissue() const;

    bits_const_reference  get_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell) const;
    bits_reference  get_bits_of_sensory_cell(natural_32_bit const index_of_sensory_cell);

    template<typename class_cell>
    void  read_sensory_cell(natural_32_bit const index_of_sensory_cell,
                            instance_wrapper<class_cell>& storage_for_cell) const;

    template<typename class_cell>
    void  write_sensory_cell(natural_32_bit const index_of_sensory_cell,
                             instance_wrapper<class_cell> const& storage_for_cell);

private:
    std::shared_ptr<cellab::neural_tissue> m_neural_tissue;
    std::mutex*  m_mutex_to_sensory_cells;
    static std::mutex  void_mutex;
};

template<typename class_cell>
void  access_to_sensory_cells::read_sensory_cell(
        natural_32_bit const index_of_sensory_cell,
        instance_wrapper<class_cell>& storage_for_cell) const
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_cells() == typeid(class_cell).hash_code());
    ASSUMPTION(index_of_sensory_cell < get_static_state_of_tissue()->num_sensory_cells());
    bits_const_reference const  bits = get_bits_of_sensory_cell(index_of_sensory_cell);
    storage_for_cell.construct_instance(bits);
}

template<typename class_cell>
void  access_to_sensory_cells::write_sensory_cell(natural_32_bit const index_of_sensory_cell,
                                                  instance_wrapper<class_cell> const& storage_for_cell)
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_cells() == typeid(class_cell).hash_code());
    ASSUMPTION(index_of_sensory_cell < get_static_state_of_tissue()->num_sensory_cells());
    bits_reference const  bits = get_bits_of_sensory_cell(index_of_sensory_cell);
    storage_for_cell.reference_to_instance() >> bits;
}


}

#endif
