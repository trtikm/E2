set(THIS_TARGET_NAME cellconnect)

add_library(${THIS_TARGET_NAME}
    ./include/cellconnect/add_links_from_sensory_cells.hpp
    ./src/add_links_from_sensory_cells.cpp

    ./include/cellconnect/add_links_to_synapses_to_muscles.hpp
    ./src/add_links_to_synapses_to_muscles.cpp

    ./include/cellconnect/incremental_addition_of_tissue_links.hpp
    ./src/incremental_addition_of_tissue_links.cpp

    ./include/cellconnect/basic_addition_of_synapses_to_territories_of_cells.hpp
    ./src/basic_addition_of_synapses_to_territories_of_cells.cpp

    ./include/cellconnect/utility/choose_tissue_coordinates.hpp
    ./src/utility/choose_tissue_coordinates.cpp

    ./include/cellconnect/utility/clip_tissue_coordinate.hpp
    ./src/utility/clip_tissue_coordinate.cpp

    ./include/cellconnect/utility/from_polar_to_tissue_coordinates.hpp
    ./src/utility/from_polar_to_tissue_coordinates.cpp

    ./include/cellconnect/utility/access_to_byte_buffer_of_tissue_cell.hpp
    ./src/utility/access_to_byte_buffer_of_tissue_cell.cpp
    )

#set_target_properties(${THIS_TARGET_NAME} PROPERTIES
#    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
#    )
#install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/${THIS_TARGET_NAME}")
