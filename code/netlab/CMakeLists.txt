set(THIS_TARGET_NAME netlab)

add_library(${THIS_TARGET_NAME}
    ./include/netlab/uid.hpp
 
    ./include/netlab/sockets.hpp
    ./include/netlab/unit.hpp
    ./include/netlab/layer.hpp

    ./include/netlab/network.hpp
    ./src/network.cpp
    
    ./include/netlab/statistics.hpp
    ./src/statistics.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
