#include <cellab/static_state_of_cell.hpp>

namespace cellab {


static_state_of_cell::static_state_of_cell(
        unsigned short max_number_of_synapses_in_territory_of_cell
        )
    : m_max_number_of_synapses_in_territory_of_cell(max_number_of_synapses_in_territory_of_cell)
{}

unsigned short static_state_of_cell::max_number_of_synapses_in_territory_of_cell() const
{
    return m_max_number_of_synapses_in_territory_of_cell;
}


}
