import pycellab
import pycellabutils

def scriptMain():
    list_of_construction_data = [
        # static_state_of_neural_tissue no. 01
        {
            "num_kinds_of_cells_in_neural_tissue" : 3,
            "num_kinds_of_sensory_cells" : 1,
            "num_bits_per_cell" : 32,
            "num_bits_per_synapse" : 32,
            "num_tissue_cells_along_x_axis" : 50,
            "num_tissue_cells_along_y_axis" : 50,
            "array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column" : [
                5,2,3
                ],
            "array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells" : [
                    {
                        "num_synapses_in_territory_of_cell" : 11
                    },
                    {
                        "num_synapses_in_territory_of_cell" : 22
                    },
                    {
                        "num_synapses_in_territory_of_cell" : 12
                    },
                ],
            "array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells" : [
                    {
                        "num_synapses_in_territory_of_cell" : 0
                    },
                ],
            "row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses" : [
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
                ],
            "array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles" : [
                    { # for tissue cell of kind=0,
                    },
                    { # for tissue cell of kind=1,
                    },
                    { # for tissue cell of kind=2,
                    },
                ],
            "num_sensory_cells" : 4,
            "num_synapses_to_muscles" : 5
        }
        # static_state_of_neural_tissue no. 02
        # TODO!
        ]

    def equivalent(construction_data, static_state_of_neural_tissue):

        if not (construction_data["num_kinds_of_cells_in_neural_tissue"] ==
                static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue()):
            return False

        if not (construction_data["num_kinds_of_sensory_cells"] ==
                static_state_of_neural_tissue.num_kinds_of_sensory_cells()):
            return False

        if not (construction_data["num_bits_per_cell"] ==
                static_state_of_neural_tissue.num_bits_per_cell()):
            return False

        if not (construction_data["num_bits_per_synapse"] ==
                static_state_of_neural_tissue.num_bits_per_synapse()):
            return False

        if not (construction_data["num_tissue_cells_along_x_axis"] ==
                static_state_of_neural_tissue.num_tissue_cells_along_x_axis()):
            return False

        if not (construction_data["num_tissue_cells_along_y_axis"] ==
                static_state_of_neural_tissue.num_tissue_cells_along_y_axis()):
            return False

        index = 0
        for num in construction_data["array_where_indices_are_kinds_of_tissue_cells_and_values_are_numbers_of_tissue_cells_in_column"]:
            assert(index < static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue())
            if not (num == static_state_of_neural_tissue.num_tissue_cells_in_column(index)):
                return False
            index = index + 1

        index = 0
        for D in construction_data["array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_tissue_cells"]:
            assert(index < static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue())
            if not (D["num_synapses_in_territory_of_cell"] ==
                    static_state_of_neural_tissue.get_static_state_of_tissue_cell(index)
                                                 .num_synapses_in_territory_of_cell()):
                return False
            index = index + 1

        index = 0
        for D in construction_data["array_where_indices_are_kinds_of_sensory_cells_and_values_are_static_states_of_sensory_cells"]:
            assert(index < static_state_of_neural_tissue.num_kinds_of_sensory_cells())
            if not (D["num_synapses_in_territory_of_cell"] ==
                    static_state_of_neural_tissue.get_static_state_of_sensory_cell(index)
                                                 .num_synapses_in_territory_of_cell()):
                return False
            index = index + 1

        index_src = 0
        index_tgt = 0
        for D in construction_data["row_major_matrix_where_indices_are_pairs_of_kinds_of_tissue_cells_and_values_are_static_states_of_synapses"]:
            assert(index_src < static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue())
            assert(index_tgt < static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue())
#            if not (D["??"] ==
#                    static_state_of_neural_tissue.get_static_state_of_synapse(index_src,index_tgt)
#                                                 .??()):
#                return False
            index_tgt = index_tgt + 1
            if index_tgt == static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue():
                index_src = index_src + 1
                index_tgt = 0

        index = 0
        for D in construction_data["array_where_indices_are_kinds_of_tissue_cells_and_values_are_static_states_of_synapses_to_muscles"]:
            assert(index < static_state_of_neural_tissue.num_kinds_of_cells_in_neural_tissue())
#            if not (D["??"] ==
#                    static_state_of_neural_tissue.get_static_state_of_synapse_to_muscles(index)
#                                                 .??()):
#                return False
            index = index + 1

        if not (construction_data["num_sensory_cells"] ==
                static_state_of_neural_tissue.num_sensory_cells()):
            return False
        if not (construction_data["num_synapses_to_muscles"] ==
                static_state_of_neural_tissue.num_synapses_to_muscles()):
            return False

        return True

    print("Test of CELLAB library no. 001")
    print("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n")
    print("The test creates " + str(len(list_of_construction_data)) + " instance(s) of " +
          "static_state_of_neural_tissue and for each\n"
          "it checks whether data in the instance match data which the instance was\n" +
          "created from.\n")
    print("Running the tests:")
    list_of_indices_of_failed_construction_data = []
    index_of_construction_data = 0
    for construction_data in list_of_construction_data:
        try:
            print("    * testing instance no. " + str(index_of_construction_data))
            static_state_of_neural_tissue = pycellabutils.create_static_state_of_neural_tissue(**construction_data)
            if equivalent(construction_data,static_state_of_neural_tissue):
                print("        success")
            else:
                print("        FAILURE! (equivalence has failed)")
                list_of_indices_of_failed_construction_data.append(
                    (index_of_construction_data,"Equivalence has failed")
                    )
        #except (RuntimeError, TypeError, ValueError, NameError, IOError) as e:
        except Exception as e:
            print("        FAILURE! (exception was thrown)")
            list_of_indices_of_failed_construction_data.append(
                (index_of_construction_data,"Exception was thrown: " + str(e))
                )
        index_of_construction_data = index_of_construction_data + 1
    if len(list_of_indices_of_failed_construction_data) == 0:
        print("\nTesting was SUCCESSFULL, no failure occurred.")
        return True
    else:
        print("\nTesting has FAILED for these indices of creation data:")
        print(list_of_indices_of_failed_construction_data)
        return False

if __name__ == "__main__":
    scriptMain()
    print "\nbye!"
