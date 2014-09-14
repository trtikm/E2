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

    natural_16_bit  num_kinds_of_sensory_cells() const;
    cellab::kind_of_cell  lowest_kind_of_sensory_cells() const;
    cellab::kind_of_cell  highest_kind_of_sensory_cells() const;

    natural_32_bit num_sensory_cells() const;

    std::pair<bits_reference,cellab::kind_of_cell>  get_bits_of_sensory_cell(
            natural_32_bit const index_of_sensory_cell
            );

    template<typename class_cell>
    cellab::kind_of_cell  read_sensory_cell(natural_32_bit const index_of_sensory_cell,
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
cellab::kind_of_cell  access_to_sensory_cells::read_sensory_cell(
        natural_32_bit const index_of_sensory_cell,
        instance_wrapper<class_cell>& storage_for_cell) const
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_cells() == typeid(class_cell).hash_code());
    ASSUMPTION(index_of_sensory_cell < num_sensory_cells());
    std::lock_guard<std::mutex> const  lock_access_to_sensory_cells(*m_mutex_to_sensory_cells);
    storage_for_cell.construct_instance(
            bits_const_reference(
                    m_neural_tissue->get_dynamic_state_of_neural_tissue()->find_bits_of_sensory_cell(
                                index_of_sensory_cell
                                )
                    )
            );
    return m_neural_tissue->get_static_state_of_neural_tissue()->compute_kind_of_sensory_cell_from_its_index(
                index_of_sensory_cell);
}

template<typename class_cell>
void  access_to_sensory_cells::write_sensory_cell(natural_32_bit const index_of_sensory_cell,
                                                  instance_wrapper<class_cell> const& storage_for_cell)
{
    ASSUMPTION(m_neural_tissue->get_hash_code_of_class_for_cells() == typeid(class_cell).hash_code());
    ASSUMPTION(index_of_sensory_cell < num_sensory_cells());
    std::lock_guard<std::mutex> const  lock_access_to_sensory_cells(*m_mutex_to_sensory_cells);
    storage_for_cell.reference_to_instance() >>
        m_neural_tissue->get_dynamic_state_of_neural_tissue()->find_bits_of_sensory_cell(
                    index_of_sensory_cell
                    );
}


}

#endif
