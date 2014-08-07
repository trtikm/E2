#ifndef CELLAB_STATIC_STATE_OF_CELL_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_CELL_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>


namespace cellab {


struct static_state_of_cell
{
    static_state_of_cell(
            natural_32_bit num_synapses_in_territory_of_cell
            );

    natural_32_bit num_synapses_in_territory_of_cell() const;

    //unsigned char radius_of_spatial_neighbourhood_of_cell() const;
    //unsigned int min_membrane_potential_to_emit_spike() const;
    //unsigned int decay_coeficient_of_membrane_potential() const;

private:
    natural_32_bit m_number_of_synapses_in_territory_of_cell;
};


}

#endif
