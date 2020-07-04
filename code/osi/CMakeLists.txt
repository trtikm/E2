set(THIS_TARGET_NAME osi)

add_library(${THIS_TARGET_NAME}
    ./include/osi/provider.hpp
    ./src/provider.cpp

    ./include/osi/simulator.hpp
    ./src/simulator.cpp

    ./include/osi/keyboard_key_names.hpp
    ./include/osi/mouse_button_names.hpp
    ./include/osi/opengl.hpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
