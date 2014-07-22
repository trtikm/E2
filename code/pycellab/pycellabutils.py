import pycellab

def create_static_state_of_neural_tissue(
        num_kinds_of_cells,
        num_bits_per_cell,
        num_bits_per_synapse,
        num_cells_along_x_axis,
        num_cells_along_y_axis,
        array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column,
        array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells,
        row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses
        ):
    def push_back_static_state_of_cell(vector,
            max_number_of_synapses_in_territory_of_cell
            ):
        pycellab.push_back_static_state_of_cell(vector,
            max_number_of_synapses_in_territory_of_cell
            )
    def push_back_static_state_of_synapse(vector
            ):
        pycellab.push_back_static_state_of_synapse(vector
            )
    numbers_of_cells_in_column = pycellab.vector_of_ushorts()
    for num in array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column:
        pycellab.push_back_ushort(numbers_of_cells_in_column,num)
    static_states_of_cells = pycellab.vector_of_static_states_of_cells()
    for cell in array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells:
        push_back_static_state_of_cell(static_states_of_cells,**cell)
    static_states_of_synapses = pycellab.vector_of_static_states_of_synapses()
    for synapse in row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses:
        push_back_static_state_of_synapse(static_states_of_synapses,**synapse)
    return pycellab.static_state_of_neural_tissue(
                num_kinds_of_cells,
                num_bits_per_cell,
                num_bits_per_synapse,
                num_cells_along_x_axis,
                num_cells_along_y_axis,
                numbers_of_cells_in_column,
                static_states_of_cells,
                static_states_of_synapses
                )

def create_dynamic_state_of_neural_tissue(static_state_of_neural_tissue):
    return {}
