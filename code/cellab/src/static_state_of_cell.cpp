#include <cellab/static_state_of_cell.hpp>

namespace cellab {


static_state_of_cell::static_state_of_cell(
        natural_16_bit num_synapses_in_territory_of_cell
        )
    : m_number_of_synapses_in_territory_of_cell(num_synapses_in_territory_of_cell)
{}

natural_16_bit static_state_of_cell::num_synapses_in_territory_of_cell() const
{
    return m_number_of_synapses_in_territory_of_cell;
}


}
