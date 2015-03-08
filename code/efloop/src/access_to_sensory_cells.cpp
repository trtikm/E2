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

std::shared_ptr<cellab::static_state_of_neural_tissue const>  access_to_sensory_cells::get_static_state_of_tissue() const
{
    return m_neural_tissue->get_static_state_of_neural_tissue();
}

bits_const_reference  access_to_sensory_cells::get_bits_of_sensory_cell(
        natural_32_bit const index_of_sensory_cell
        ) const
{
    return const_cast<access_to_sensory_cells*>(this)->get_bits_of_sensory_cell(index_of_sensory_cell);
}

bits_reference  access_to_sensory_cells::get_bits_of_sensory_cell(
        natural_32_bit const index_of_sensory_cell
        )
{
    ASSUMPTION(index_of_sensory_cell < get_static_state_of_tissue()->num_sensory_cells());
    std::lock_guard<std::mutex> const  lock_access_to_sensory_cells(*m_mutex_to_sensory_cells);
    return m_neural_tissue->get_dynamic_state_of_neural_tissue()->find_bits_of_sensory_cell(index_of_sensory_cell);
}


}
