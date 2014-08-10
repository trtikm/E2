add_library(cellab
    ./include/cellab/static_state_of_cell.hpp
    ./src/static_state_of_cell.cpp

    ./include/cellab/dynamic_state_of_cell.hpp
    ./src/dynamic_state_of_cell.cpp

    ./include/cellab/static_state_of_synapse.hpp
    ./src/static_state_of_synapse.cpp

    ./include/cellab/dynamic_state_of_synapse.hpp
    ./src/dynamic_state_of_synapse.cpp

    ./include/cellab/static_state_of_neural_tissue.hpp
    ./src/static_state_of_neural_tissue.cpp

    ./include/cellab/dynamic_state_of_neural_tissue.hpp
    ./src/dynamic_state_of_neural_tissue.cpp

    ./include/cellab/algorithms_for_state_transition.hpp
    ./src/transition_of_synapses_to_muscles.cpp
    ./src/transition_of_synapses_of_tissue.cpp
    ./src/transition_of_synaptic_migration_in_tissue.cpp
    ./src/transition_of_spaialy_local_intercellular_signaling_in_tissue.cpp
    ./src/transition_of_cells_of_tissue.cpp
    )

#install(TARGETS brod_utils DESTINATION lib/utils)
