set(THIS_TARGET_NAME netlab)

add_library(${THIS_TARGET_NAME}
    ./include/netlab/network.hpp
    ./include/netlab/network_uid.hpp
    ./src/network.cpp
    ./src/network_configurations.cpp
    ./src/network_prefabs.cpp
    ./src/network_statistics.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
