set(THIS_TARGET_NAME cellab)

add_library(${THIS_TARGET_NAME}
    ./include/cellab/static_state_of_cell.hpp
    ./src/static_state_of_cell.cpp

    ./include/cellab/dynamic_state_of_cell.hpp
    ./src/dynamic_state_of_cell.cpp

    ./include/cellab/static_state_of_synapse.hpp
    ./src/static_state_of_synapse.cpp

    ./include/cellab/dynamic_state_of_synapse.hpp
    ./src/dynamic_state_of_synapse.cpp

    ./include/cellab/static_state_of_signalling.hpp
    ./src/static_state_of_signalling.cpp

    ./include/cellab/dynamic_state_of_signalling.hpp
    ./src/dynamic_state_of_signalling.cpp

    ./include/cellab/static_state_of_neural_tissue.hpp
    ./src/static_state_of_neural_tissue.cpp

    ./include/cellab/dynamic_state_of_neural_tissue.hpp
    ./src/dynamic_state_of_neural_tissue.cpp

    ./include/cellab/homogenous_slice_of_tissue.hpp
    ./src/homogenous_slice_of_tissue.cpp

    ./include/cellab/algorithms_for_state_transition.hpp
#    ./include/cellab/transition_functions_of_neural_tissue.hpp
#    ./include/cellab/transition_functions_of_packed_tissue_elements.hpp
#    ./include/cellab/convertors_from_packed_to_unpacked_dynamic_states.hpp

    ./src/transition_of_synapses_to_muscles.cpp
    ./src/transition_of_synapses_of_tissue.cpp
    ./src/transition_of_synaptic_migration_in_tissue.cpp
    ./src/transition_of_signalling_in_tissue.cpp
    ./src/transition_of_cells_of_tissue.cpp
    )

#set_target_properties(${THIS_TARGET_NAME} PROPERTIES
#    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
#    )
#install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/${THIS_TARGET_NAME}")
