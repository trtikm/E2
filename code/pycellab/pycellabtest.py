import pycellab
import pycellabutils

def scriptMain():
    static_state_of_neural_tissue = pycellabutils.create_static_state_of_neural_tissue(**{
        "num_kinds_of_cells" : 3,
        "num_bits_per_cell" : 32,
        "num_bits_per_synapse" : 32,
        "num_cells_along_x_axis" : 50,
        "num_cells_along_y_axis" : 50,
        "array_where_indices_are_kinds_of_cells_and_values_are_numbers_of_cells_in_column" : [
            5,2,3
            ],
        "array_where_indices_are_kinds_of_cells_and_values_are_static_states_of_cells" : [
                {
                    "max_number_of_synapses_in_territory_of_cell" : 11
                },
                {
                    "max_number_of_synapses_in_territory_of_cell" : 22
                },
                {
                    "max_number_of_synapses_in_territory_of_cell" : 12
                },
            ],
        "row_major_matrix_where_indices_are_pairs_of_kinds_of_cells_and_values_are_static_states_of_synapses" : [
                { # src_kind=0, dst_kind=0
                },
                { # src_kind=0, dst_kind=1
                },
                { # src_kind=0, dst_kind=2
                },

                { # src_kind=1, dst_kind=0
                },
                { # src_kind=1, dst_kind=1
                },
                { # src_kind=1, dst_kind=2
                },

                { # src_kind=2, dst_kind=0
                },
                { # src_kind=2, dst_kind=1
                },
                { # src_kind=2, dst_kind=2
                },
            ]
        })

    dynamic_state_of_neural_tissue = pycellabutils.create_dynamic_state_of_neural_tissue(
        static_state_of_neural_tissue
        )
    print dynamic_state_of_neural_tissue

if __name__ == "__main__":
    scriptMain()
    print "\nbye!"
