set(THIS_TARGET_NAME scene)

add_library(${THIS_TARGET_NAME}
    ./include/scene/scene.hpp
    ./src/scene.cpp

    ./include/scene/scene_selection.hpp 
    ./src/scene_selection.cpp 

    ./include/scene/scene_edit_utils.hpp 
    ./src/scene_edit_utils.cpp 

    ./include/scene/scene_history.hpp 
    ./src/scene_history.cpp 

    ./include/scene/scene_utils.hpp
    ./src/scene_utils.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
