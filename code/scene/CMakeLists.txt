set(THIS_TARGET_NAME scene)

add_library(${THIS_TARGET_NAME}
    ./include/scene/scene.hpp
    ./src/scene.cpp
    
    ./include/scene/scene_node_id.hpp
    ./src/scene_node_id.cpp

    ./include/scene/scene_node_record_id.hpp
    ./include/scene/scene_record_id.hpp

    ./include/scene/scene_selection.hpp
    ./src/scene_selection.cpp

    ./include/scene/scene_editing.hpp
    ./src/scene_editing.cpp

    ./include/scene/scene_history.hpp
    ./include/scene/scene_history_node.hpp
    ./include/scene/scene_history_nodes_default.hpp
    ./include/scene/scene_history_nodes_specialised.hpp
    ./src/scene_history.cpp

    ./include/scene/scene_utils.hpp
    ./include/scene/scene_utils_specialised.hpp
    ./src/scene_utils.cpp

    ./include/scene/records/skeleton_props.hpp
    ./include/scene/records/batch/utils.hpp
    ./include/scene/records/batch/history_nodes.hpp
    ./include/scene/records/collider/collider.hpp
    ./include/scene/records/collider/utils.hpp
    ./include/scene/records/collider/history_nodes.hpp
    ./include/scene/records/rigid_body/rigid_body.hpp
    ./include/scene/records/rigid_body/utils.hpp
    ./include/scene/records/rigid_body/history_nodes.hpp
    ./include/scene/records/agent/agent.hpp
    ./include/scene/records/agent/utils.hpp
    ./include/scene/records/agent/history_nodes.hpp
    ./include/scene/records/device/device.hpp
    ./include/scene/records/device/utils.hpp
    ./include/scene/records/device/history_nodes.hpp
    ./include/scene/records/sensor/sensor.hpp
    ./include/scene/records/sensor/utils.hpp
    ./include/scene/records/sensor/history_nodes.hpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
