set(THIS_TARGET_NAME ai)

add_library(${THIS_TARGET_NAME}
    ./include/ai/simulator.hpp
    ./src/simulator.cpp

    ./include/ai/agent_id.hpp

    ./include/ai/agent_state_variables.hpp
    ./src/agent_state_variables.cpp

    ./include/ai/agent_system_variables.hpp
    ./src/agent_system_variables.cpp

    ./include/ai/agent_system_state.hpp
    ./src/agent_system_state.cpp

    ./include/ai/agent_config.hpp
    ./src/agent_config.cpp

    ./include/ai/agent.hpp
    ./src/agent.cpp
    
    ./include/ai/cortex.hpp
    ./src/cortex.cpp

    ./include/ai/cortex_mock.hpp
    ./src/cortex_mock.cpp
    ./include/ai/cortex_random.hpp
    ./src/cortex_random.cpp
    ./include/ai/cortex_default.hpp
    ./src/cortex_default.cpp
    ./include/ai/cortex_robot.hpp
    ./src/cortex_robot.cpp

    ./include/ai/motion_desire_props.hpp
    ./src/motion_desire_props.cpp

    ./include/ai/action_controller.hpp
    ./src/action_controller.cpp

    ./include/ai/sight_controller.hpp
    ./src/sight_controller.cpp

    ./include/ai/detail/action_controller_roller.hpp
    ./src/detail/action_controller_roller.cpp

    ./include/ai/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp
    
    ./include/ai/skeleton_interpolators.hpp
    ./src/skeleton_interpolators.cpp
    
    ./include/ai/scene_binding.hpp
    ./src/scene_binding.cpp

    ./include/ai/skeleton_utils.hpp
    ./src/skeleton_utils.cpp
    ./include/ai/utils_ptree.hpp
    ./src/utils_ptree.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
