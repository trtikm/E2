set(THIS_TARGET_NAME netexp)

add_library(${THIS_TARGET_NAME}
    ./include/netexp/experiment_factory.hpp
    ./src/experiment_factory.cpp
    
    ./include/netexp/algorithm.hpp
    ./src/algorithm.cpp

    ./include/netexp/ship_controller_flat_space.hpp
    ./src/ship_controller_flat_space.cpp

    ./include/netexp/incremental_initialiser_of_movement_area_centers.hpp
    ./src/incremental_initialiser_of_movement_area_centers.cpp

    ./src/calibration/calibration.cpp
    ./src/performance/performance.cpp
    ./src/dbg_spiking_develop/dbg_spiking_develop.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
