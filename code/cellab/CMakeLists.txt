add_library(cellab
    ./src/neural_tissue.cpp
    ./src/bits_reference.cpp
    ./src/cell_kinds.cpp
    ./include/cellab/neural_tissue.hpp
    ./include/cellab/bits_reference.hpp
    ./include/cellab/cell_kinds.hpp

    ./include/cellab/static_state_of_neural_tissue.hpp
    ./include/cellab/dynamic_state_of_neural_tissue.hpp
    ./include/cellab/static_state_of_cell.hpp
    ./include/cellab/dynamic_state_of_cell.hpp
    ./include/cellab/static_state_of_synapse.hpp
    ./include/cellab/dynamic_state_of_synapse.hpp

    ./src/state_of_cell.cpp
    ./include/cellab/state_of_cell.hpp
    )

#install(TARGETS brod_utils DESTINATION lib/utils)

