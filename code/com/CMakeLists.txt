set(THIS_TARGET_NAME com)

add_library(${THIS_TARGET_NAME}
    ./include/com/object_guid.hpp
    ./src/object_guid.cpp

    ./include/com/frame_of_reference.hpp
    ./src/frame_of_reference.cpp

    ./include/com/simulation_context.hpp
    ./src/simulation_context.cpp
    
    ./include/com/simulator.hpp
    ./src/simulator.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")