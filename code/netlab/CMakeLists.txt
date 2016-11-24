set(THIS_TARGET_NAME netlab)

add_library(${THIS_TARGET_NAME}
    ./include/netlab/tracked_object_stats.hpp
    ./include/netlab/initialiser_of_movement_area_centers.hpp
    ./include/netlab/initialiser_of_ships_in_movement_areas.hpp
    ./include/netlab/network_objects_factory.hpp
    ./src/network_objects_factory.cpp

    ./include/netlab/ship_controller.hpp
    ./src/ship_controller.cpp

    ./include/netlab/network_layer_props.hpp
    ./src/network_layer_props.cpp

    ./include/netlab/network_props.hpp
    ./src/network_props.cpp

    ./include/netlab/network_objects.hpp
    ./src/network_objects.cpp

    ./include/netlab/network_indices.hpp
    ./src/network_indices.cpp

    ./include/netlab/network.hpp
    ./src/network.cpp

    ./include/netlab/utility.hpp
    ./src/utility.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
