#include <efloop/access_to_sensory_cells.hpp>

namespace efloop {


std::mutex  access_to_sensory_cells::void_mutex;

access_to_sensory_cells::access_to_sensory_cells(
        std::shared_ptr<cellab::neural_tissue> const  neural_tissue,
        std::mutex* const  mutex_to_sensory_cells
        )
    : m_neural_tissue(neural_tissue)
    , m_mutex_to_sensory_cells(
          mutex_to_sensory_cells != nullptr ? mutex_to_sensory_cells : &void_mutex
          )
{
    ASSUMPTION(m_neural_tissue.operator bool());
}

natural_16_bit  access_to_sensory_cells::num_kinds_of_sensory_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_kinds_of_sensory_cells();
}

cellab::kind_of_cell  access_to_sensory_cells::lowest_kind_of_sensory_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->lowest_kind_of_sensory_cells();
}

cellab::kind_of_cell  access_to_sensory_cells::highest_kind_of_sensory_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_kinds_of_cells() - 1U;
}

natural_32_bit access_to_sensory_cells::num_sensory_cells() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue()->num_sensory_cells();
}

std::pair<bits_reference,cellab::kind_of_cell>
access_to_sensory_cells::access_to_sensory_cells::get_bits_of_sensory_cell(
        natural_32_bit const index_of_sensory_cell
        )
{
    ASSUMPTION(index_of_sensory_cell < num_sensory_cells());
    std::lock_guard<std::mutex> const  lock_access_to_sensory_cells(*m_mutex_to_sensory_cells);
    return std::make_pair(
                m_neural_tissue->get_dynamic_state_of_neural_tissue()->find_bits_of_sensory_cell(
                        index_of_sensory_cell
                        ),
                m_neural_tissue->get_static_state_of_neural_tissue()->compute_kind_of_sensory_cell_from_its_index(
                        index_of_sensory_cell
                        )
                );
}


}
