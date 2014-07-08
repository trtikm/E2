#include <cellab/neural_tissue.hpp>
#include <cellab/bits_reference.hpp>
#include <cellab/cell_kinds.hpp>
#include <cellab/state_of_cell.hpp>
#include <iostream>

//int main()
int foo()
{
    unsigned char const num_bits_per_cell_kind[NUM_KINDS_OF_CELLS] = {22,22,22};
    unsigned short const num_cells_of_kind_in_column[NUM_KINDS_OF_CELLS] = {30,40,20};
    unsigned char const max_num_synapses_per_cell_kind[NUM_KINDS_OF_CELLS] = {5,9,4};
    neural_tissue NT(10,10,num_bits_per_cell_kind,num_cells_of_kind_in_column,
                     30,max_num_synapses_per_cell_kind);

    bits_reference bis_ref = NT.cell_bits_reference(0,0,EXCITATORY_LOCAL_NEURON,0);
    dynamic_state_of_cell cell_state;
    cell_state.set_membrane_potential(10);
    cell_state.set_energy_consumption(20);
    cell_state.set_intensity_of_extracelular_energy_consumption_signal(30);
    cell_state >> bis_ref;
    cell_state << bis_ref;

    CELL_KIND cell_kind = EXCITATORY_LOCAL_NEURON;
    unsigned char char_cell_kind = cell_kind;
    char_cell_kind = char_cell_kind;

    return 0;
}
