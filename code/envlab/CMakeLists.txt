set(THIS_TARGET_NAME envlab)

add_library(${THIS_TARGET_NAME}
    ./include/envlab/rules_and_logic_of_environment.hpp
    ./src/rules_and_logic_of_environment.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
