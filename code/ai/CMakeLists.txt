set(THIS_TARGET_NAME ai)

add_library(${THIS_TARGET_NAME}
    ./include/ai/simulator.hpp
    ./src/simulator.cpp

    ./include/ai/object_id.hpp

    ./include/ai/agent_id.hpp

    ./include/ai/agent_kind.hpp
    ./src/agent_kind.cpp

    ./include/ai/agent.hpp
    ./src/agent.cpp
    
    ./include/ai/agents.hpp
    ./src/agents.cpp

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

    ./include/ai/detail/action_controller_interpolator.hpp
    ./src/detail/action_controller_interpolator.cpp
    ./include/ai/detail/action_controller_interpolator_animation.hpp
    ./src/detail/action_controller_interpolator_animation.cpp
    ./include/ai/detail/action_controller_interpolator_look_at.hpp
    ./src/detail/action_controller_interpolator_look_at.cpp
    ./include/ai/detail/action_controller_interpolator_matter.hpp
    ./src/detail/action_controller_interpolator_matter.cpp

    ./include/ai/detail/action_controller_roller.hpp
    ./src/detail/action_controller_roller.cpp

    ./include/ai/input_devices.hpp

    ./include/ai/blackboard.hpp
    ./src/blackboard.cpp
    ./include/ai/blackboard_agent.hpp
    ./src/blackboard_agent.cpp
    ./include/ai/blackboard_device.hpp
    ./src/blackboard_device.cpp

    ./include/ai/scene_basic_types_binding.hpp
    ./include/ai/scene.hpp

    ./include/ai/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp
    
    ./include/ai/skeleton_utils.hpp
    ./src/skeleton_utils.cpp

    ./include/ai/property_map.hpp
    ./src/property_map.cpp

    ./include/ai/retina.hpp

    ./include/ai/env/snapshot.hpp
    ./src/env/snapshot.cpp
    ./include/ai/env/snapshots_cache.hpp
    ./src/env/snapshots_cache.cpp

    ./include/ai/env/sinet/snapshot_encoder.hpp 
    ./src/env/sinet/snapshot_encoder.cpp 
    ./include/ai/env/sinet/number_encoder.hpp 
    ./src/env/sinet/number_encoder.cpp
    ./include/ai/env/sinet/motion_desire_decoder.hpp
    ./src/env/sinet/motion_desire_decoder.cpp
    ./include/ai/env/sinet/number_decoder.hpp 
    ./src/env/sinet/number_decoder.cpp 

    ./include/ai/detail/rigid_body_motion.hpp
    ./src/detail/rigid_body_motion.cpp

    ./include/ai/detail/guarded_motion_actions_processor.hpp 
    ./src/detail/guarded_motion_actions_processor.cpp 
    
    ./include/ai/detail/ideal_velocity_buider.hpp 
    ./src/detail/ideal_velocity_buider.cpp 

    ./include/ai/detail/expression_evaluator.hpp
    ./src/detail/expression_evaluator.cpp

    ./include/ai/detail/collider_utils.hpp
    ./src/detail/collider_utils.cpp

    ./include/ai/device_id.hpp

    ./include/ai/device_kind.hpp
    ./src/device_kind.cpp

    ./include/ai/device.hpp
    ./src/device.cpp
    
    ./include/ai/devices.hpp
    ./src/devices.cpp

    ./include/ai/sensor_id.hpp

    ./include/ai/sensor_kind.hpp
    ./src/sensor_kind.cpp

    ./include/ai/sensor.hpp
    ./src/sensor.cpp
    
    ./include/ai/sensors.hpp
    ./src/sensors.cpp

    ./include/ai/sensor_action.hpp
    ./src/sensor_action.cpp
    ./include/ai/sensor_action_default_processor.hpp
    ./src/sensor_action_default_processor.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
