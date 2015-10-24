set(THIS_TARGET_NAME efloop)

add_library(${THIS_TARGET_NAME}
    ./include/efloop/external_feedback_loop.hpp
    ./src/external_feedback_loop.cpp

    ./include/efloop/access_to_sensory_cells.hpp
    ./src/access_to_sensory_cells.cpp

    ./include/efloop/access_to_synapses_to_muscles.hpp
    ./src/access_to_synapses_to_muscles.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
