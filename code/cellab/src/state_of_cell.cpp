#include <cellab/state_of_cell.hpp>
#include <utility/assumptions.hpp>

#include <utility/development.hpp>

static_state_of_cell& static_state_of_cell::instance()
{
    static static_state_of_cell static_state;
    return static_state;
}

unsigned int static_state_of_cell::min_membrane_potential_to_emit_spike(
    CELL_KIND const cell_kind
    ) const
{
    ASSUMPTION(cell_kind < num_cell_kinds());
    return m_min_membrane_potential_to_emit_spike[cell_kind];
}

unsigned int static_state_of_cell::decay_coeficient_of_membrane_potential(
    CELL_KIND const cell_kind
    ) const
{
    ASSUMPTION(cell_kind < num_cell_kinds());
    return m_decay_coeficient_of_membrane_potential[cell_kind];
}

static_state_of_cell::static_state_of_cell()
    : m_min_membrane_potential_to_emit_spike(new unsigned int[num_cell_kinds()])
    , m_decay_coeficient_of_membrane_potential(new float[num_cell_kinds()])
{
    m_min_membrane_potential_to_emit_spike[EXCITATORY_LOCAL_NEURON] = 100;
    m_decay_coeficient_of_membrane_potential[EXCITATORY_LOCAL_NEURON] = 0.1;

    m_min_membrane_potential_to_emit_spike[EXCITATORY_PROJECTION_NEURON] = 200;
    m_decay_coeficient_of_membrane_potential[EXCITATORY_PROJECTION_NEURON] = 0.2;

    m_min_membrane_potential_to_emit_spike[INHIBITORY_LOCAL_NEURON] = 100;
    m_decay_coeficient_of_membrane_potential[INHIBITORY_LOCAL_NEURON] = 0.1;
}

unsigned int dynamic_state_of_cell::membrane_potential() const
{
    return m_membrane_potential;
}

unsigned int dynamic_state_of_cell::energy_consumption() const
{
    return m_energy_consumption;
}

unsigned int dynamic_state_of_cell::intensity_of_extracelular_energy_consumption_signal() const
{
    return m_intensity_of_extracelular_energy_consumption_signal;
}

void dynamic_state_of_cell::set_membrane_potential(unsigned int const value)
{
    m_membrane_potential = value;
}

void dynamic_state_of_cell::set_energy_consumption(unsigned int const value)
{
    m_energy_consumption = value;
}

void dynamic_state_of_cell::set_intensity_of_extracelular_energy_consumption_signal(
    unsigned int const value)
{
    m_intensity_of_extracelular_energy_consumption_signal = value;
}

dynamic_state_of_cell& dynamic_state_of_cell::operator<<(bits_reference const& bits)
{
    ASSUMPTION(bits.num_bits() == 10 + 7 + 5);

    bits_to_value(bits,0,10,m_membrane_potential);
    bits_to_value(bits,10,7,m_energy_consumption);
    bits_to_value(bits,17,5,m_intensity_of_extracelular_energy_consumption_signal);

    return *this;
}

dynamic_state_of_cell const& operator>>(dynamic_state_of_cell const& cell_state, bits_reference& bits)
{
    ASSUMPTION(bits.num_bits() == 10 + 7 + 5);

    value_to_bits(cell_state.membrane_potential(),10,bits,0);
    value_to_bits(cell_state.energy_consumption(),7,bits,10);
    value_to_bits(cell_state.intensity_of_extracelular_energy_consumption_signal(),5,bits,17);

    return cell_state;
}



