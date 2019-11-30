set(THIS_TARGET_NAME ai)

add_library(${THIS_TARGET_NAME}
    ./include/ai/agents.hpp
    ./src/agents.cpp

    ./include/ai/agent_id.hpp
    ./include/ai/agent_kind.hpp
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

    ./include/ai/sensory_controller.hpp
    ./src/sensory_controller.cpp

    ./include/ai/sensory_controller_collision_contacts.hpp
    ./src/sensory_controller_collision_contacts.cpp 
    ./include/ai/sensory_controller_sight.hpp
    ./src/sensory_controller_sight.cpp 
    ./include/ai/sensory_controller_ray_cast_sight.hpp 
    ./src/sensory_controller_ray_cast_sight.cpp 

    ./include/ai/action_controller.hpp
    ./src/action_controller.cpp
    ./include/ai/action_regulator.hpp
    ./src/action_regulator.cpp

    ./include/ai/input_devices.hpp

    ./include/ai/blackboard.hpp
    ./src/blackboard.cpp

    ./include/ai/scene.hpp

    ./include/ai/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp
    
    ./include/ai/skeleton_utils.hpp
    ./src/skeleton_utils.cpp

    ./include/ai/retina.hpp

    ./include/ai/env/snapshot.hpp
    ./src/env/snapshot.cpp
    ./include/ai/env/snapshots_cache.hpp
    ./src/env/snapshots_cache.cpp

    ./include/ai/detail/rigid_body_motion.hpp
    ./src/detail/rigid_body_motion.cpp

    ./include/ai/detail/guarded_motion_actions_processor.hpp 
    ./src/detail/guarded_motion_actions_processor.cpp 
    
    ./include/ai/detail/ideal_velocity_buider.hpp 
    ./src/detail/ideal_velocity_buider.cpp 
    
    ./include/ai/detail/collider_utils.hpp
    ./src/detail/collider_utils.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
