set(THIS_TARGET_NAME ai)

add_library(${THIS_TARGET_NAME}
    ./include/ai/simulator.hpp
    ./src/simulator.cpp

    ./include/ai/agent_id.hpp

    ./include/ai/agent_kind.hpp
    ./src/agent_kind.cpp

    ./include/ai/agent.hpp
    ./src/agent.cpp
    
    ./include/ai/cortex.hpp
    ./src/cortex.cpp

    ./include/ai/cortex_mock.hpp
    ./src/cortex_mock.cpp
    ./include/ai/cortex_random.hpp
    ./src/cortex_random.cpp
    ./include/ai/cortex_robot.hpp
    ./src/cortex_robot.cpp

    ./include/ai/motion_desire_props.hpp
    ./src/motion_desire_props.cpp

    ./include/ai/action_controller.hpp
    ./src/action_controller.cpp

    ./include/ai/sight_controller.hpp
    ./src/sight_controller.cpp

    ./include/ai/detail/action_controller_interpolator.hpp
    ./src/detail/action_controller_interpolator.cpp
    ./include/ai/detail/action_controller_interpolator_composed.hpp
    ./src/detail/action_controller_interpolator_composed.cpp
    ./include/ai/detail/action_controller_interpolator_animation.hpp
    ./src/detail/action_controller_interpolator_animation.cpp
    ./include/ai/detail/action_controller_interpolator_look_at.hpp
    ./src/detail/action_controller_interpolator_look_at.cpp
    ./include/ai/detail/action_controller_interpolator_aim_at.hpp
    ./src/detail/action_controller_interpolator_aim_at.cpp

    ./include/ai/detail/action_controller_roller.hpp
    ./src/detail/action_controller_roller.cpp

    ./include/ai/detail/interpolation_speed_solver.hpp
    ./src/detail/interpolation_speed_solver.cpp

    ./include/ai/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp
    
    ./include/ai/scene_binding.hpp
    ./src/scene_binding.cpp

    ./include/ai/skeleton_utils.hpp
    ./src/skeleton_utils.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
