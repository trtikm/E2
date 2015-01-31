set(THIS_TARGET_NAME cellconnect)

add_library(${THIS_TARGET_NAME}
    ./include/cellconnect/compute_link_terminal_coordinates.hpp
    ./src/compute_link_terminal_coordinates.cpp

    ./include/cellconnect/incremental_link_addition.hpp
    )

#set_target_properties(${THIS_TARGET_NAME} PROPERTIES
#    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
#    )
#install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib/${THIS_TARGET_NAME}")
