import pycellab

def create_static_state_of_neural_tissue(
        num_kinds_of_cells_in_neural_tissue,
        num_kinds_of_sensory_cells,
        num_bits_per_cell,
        num_bits_per_synapse,
        num_tissue_cells_along_x_axis,
        num_tissue_cells_along_y_axis,
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column,
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells,
        array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells,
        row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses,
        array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles,
        num_sensory_cells,
        num_synapses_to_muscles):
    '''
        This function creates and returns an instance of 'static_state_of_neural_tissue' class
        from the passed construction data.

        You should call this function such that you create a distionary, D say, whose keys are names
        of parameters of this function. Then you call this function as:

                result = pycellab.create_static_state_of_neural_tissue(**D)

        Value of any of the following parameters:
            num_kinds_of_cells_in_neural_tissue
            num_kinds_of_sensory_cells
            num_bits_per_cell
            num_bits_per_synapse
            num_tissue_cells_along_x_axis
            num_tissue_cells_along_y_axis
        is
            a positive integer.

        Value of any of the following parameters:
            num_sensory_cells
            num_synapses_to_muscles
        is
            a non-negative integer.

        Further, any element of the list in:
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column
        has to be
            a positive integer number

        Any element of the list in:
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells
        has to be a dinctionary of the form:
            {
                "num_synapses_in_territory_of_cell" : <a positive integer>,
            }

        Any element of the list in:
            array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells
        has to be a dinctionary of the form:
            {
                "num_synapses_in_territory_of_cell" : 0,
            }

        Any element of the list in:
            row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses
        has to be a dinctionary of the form:
            {}
        Note that the list is interpreted as a row-major matrix. It means that dictionaries are
        stored as follows:
             0,    ...   ,N-1  # indices of dictionaries
            {..}, ...., {...}, # 0...(N-1)th dict, for (src_kind=  0, dst_kind=0),...,(src_kind=  0, dst_kind=N-1)
            ................., # ...
            {..}, ...., {...}, # M...M+(N-1)th dict, for (src_kind=N-1, dst_kind=0),...,(src_kind=N-1, dst_kind=N-1),
            where N is values of the parameter 'num_kinds_of_cells_in_neural_tissue' and M = N*N-N

        Any element of the list in:
            array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles
        has to be a dinctionary of the form:
            {}
    '''

    def push_back_static_state_of_cell(vector,num_synapses_in_territory_of_cell):
        pycellab.push_back_static_state_of_cell(vector,num_synapses_in_territory_of_cell)

    def push_back_static_state_of_synapse(vector):
        pycellab.push_back_static_state_of_synapse(vector)

    numbers_of_cells_in_column = pycellab.vector_of_natural_16_bit()
    for num in array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column:
        pycellab.push_back_natural_16_bit(numbers_of_cells_in_column,num)
    static_states_of_tissue_cells = pycellab.vector_of_static_states_of_cells()
    for cell in array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells:
        push_back_static_state_of_cell(static_states_of_tissue_cells,**cell)
    static_states_of_sensory_cells = pycellab.vector_of_static_states_of_cells()
    for cell in array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells:
        push_back_static_state_of_cell(static_states_of_sensory_cells,**cell)
    static_states_of_synapses = pycellab.vector_of_static_states_of_synapses()
    for synapse in row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses:
        push_back_static_state_of_synapse(static_states_of_synapses,**synapse)
    static_states_of_synapses_to_muscles = pycellab.vector_of_static_states_of_synapses()
    for synapse in array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles:
        push_back_static_state_of_synapse(static_states_of_synapses_to_muscles,**synapse)
    return pycellab.static_state_of_neural_tissue(
                num_kinds_of_cells_in_neural_tissue,
                num_kinds_of_sensory_cells,
                num_bits_per_cell,
                num_bits_per_synapse,
                num_tissue_cells_along_x_axis,
                num_tissue_cells_along_y_axis,
                numbers_of_cells_in_column,
                static_states_of_tissue_cells,
                static_states_of_sensory_cells,
                static_states_of_synapses,
                static_states_of_synapses_to_muscles,
                num_sensory_cells,
                num_synapses_to_muscles
                )

def create_dynamic_state_of_neural_tissue(static_state_of_neural_tissue):
    '''
        This function creates and returns an instance of 'dynamic_state_of_neural_tissue' class
        from the passed instance of 'static_state_of_neural_tissue' class created by the function
                pycellab.create_static_state_of_neural_tissue(...).
    '''
    # TODO: implement this function!
    pass
