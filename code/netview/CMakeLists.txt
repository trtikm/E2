set(THIS_TARGET_NAME netview)

add_library(${THIS_TARGET_NAME}
    ./include/netview/raycast.hpp
    ./src/raycast.cpp

    ./include/netview/enumerate.hpp
    ./src/enumerate.cpp

    ./include/netview/utility.hpp
    ./src/utility.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
