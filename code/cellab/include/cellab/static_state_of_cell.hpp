#ifndef CELLAB_STATIC_STATE_OF_CELL_HPP_INCLUDED
#   define CELLAB_STATIC_STATE_OF_CELL_HPP_INCLUDED

namespace cellab {


struct static_state_of_cell
{
    static_state_of_cell(
            unsigned short num_synapses_in_territory_of_cell
            );

    unsigned short num_synapses_in_territory_of_cell() const;

    //unsigned char radius_of_spatial_neighbourhood_of_cell() const;
    //unsigned int min_membrane_potential_to_emit_spike() const;
    //unsigned int decay_coeficient_of_membrane_potential() const;

private:
    unsigned short m_number_of_synapses_in_territory_of_cell;
};


}

#endif
