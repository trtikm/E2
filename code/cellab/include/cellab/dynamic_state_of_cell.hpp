#ifndef CELLAB_DYNAMIC_STATE_OF_CELL_HPP_INCLUDED
#   define CELLAB_DYNAMIC_STATE_OF_CELL_HPP_INCLUDED

struct bits_reference;

struct dynamic_state_of_cell
{
    dynamic_state_of_cell& operator<<(bits_reference const& bits);

//    unsigned int membrane_potential() const;
//    void set_membrane_potential(unsigned int const value);

//    unsigned int energy_consumption() const;
//    void set_energy_consumption(unsigned int const value);

//    unsigned int gradient_of_intensity_of_extracelular_energy_consumption_signal_along_x_axis() const;
//    unsigned int gradient_of_intensity_of_extracelular_energy_consumption_signal_along_y_axis() const;
//    unsigned int gradient_of_intensity_of_extracelular_energy_consumption_signal_along_columnar_axis() const;
//    void set_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_x_axis(unsigned int const value);
//    void set_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_y_axis(unsigned int const value);
//    void set_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_columnar_axis(unsigned int const value);

private:
//    unsigned int m_membrane_potential;
//    unsigned int m_energy_consumption;
//    unsigned int m_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_x_axis;
//    unsigned int m_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_y_axis;
//    unsigned int m_gradient_of_intensity_of_extracelular_energy_consumption_signal_along_columnar_axis;
};

bits_reference& operator>>(dynamic_state_of_cell const& dynamic_data_of_cell, bits_reference& bits);

#endif
