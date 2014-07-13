#ifndef CELLAB_STATE_OF_CELL_HPP_INCLUDED
#   define CELLAB_STATE_OF_CELL_HPP_INCLUDED

#   include <cellab/cell_kinds.hpp>
#   include <cellab/bits_reference.hpp>
#   include <boost/scoped_array.hpp>
#   include <boost/noncopyable.hpp>

struct static_state_of_cell : boost::noncopyable
{
    static static_state_of_cell& instance();

    unsigned int min_membrane_potential_to_emit_spike(CELL_KIND const cell_kind) const;
    unsigned int decay_coeficient_of_membrane_potential(CELL_KIND const cell_kind) const;

    unsigned char radius_of_spatial_neighbourhood_of_cell(unsigned char const cell_kind) const;

private:
    static_state_of_cell();

    boost::scoped_array<unsigned int> m_min_membrane_potential_to_emit_spike;
    boost::scoped_array<float> m_decay_coeficient_of_membrane_potential;
};

struct dynamic_state_of_cell
{
    unsigned int membrane_potential() const;
    unsigned int energy_consumption() const;
    unsigned int intensity_of_extracelular_energy_consumption_signal() const;

    void set_membrane_potential(unsigned int const value);
    void set_energy_consumption(unsigned int const value);
    void set_intensity_of_extracelular_energy_consumption_signal(unsigned int const value);

    dynamic_state_of_cell& operator<<(bits_reference const& bits);

private:
    unsigned int m_membrane_potential;
    unsigned int m_energy_consumption;
    unsigned int m_intensity_of_extracelular_energy_consumption_signal;
};

dynamic_state_of_cell const& operator>>(dynamic_state_of_cell const& cell_state, bits_reference& bits);

#endif
