set(THIS_TARGET_NAME aiold)

add_library(${THIS_TARGET_NAME}
    ./include/aiold/simulator.hpp
    ./src/simulator.cpp

    ./include/aiold/object_id.hpp

    ./include/aiold/agent_id.hpp

    ./include/aiold/agent_kind.hpp
    ./src/agent_kind.cpp

    ./include/aiold/agent.hpp
    ./src/agent.cpp
    
    ./include/aiold/agents.hpp
    ./src/agents.cpp

    ./include/aiold/cortex.hpp
    ./src/cortex.cpp

    ./include/aiold/cortex_mock.hpp
    ./src/cortex_mock.cpp
    ./include/aiold/cortex_random.hpp
    ./src/cortex_random.cpp
    ./include/aiold/cortex_robot.hpp
    ./src/cortex_robot.cpp

    ./include/aiold/motion_desire_props.hpp
    ./src/motion_desire_props.cpp

    ./include/aiold/sensory_controller.hpp
    ./src/sensory_controller.cpp

    ./include/aiold/sensory_controller_collision_contacts.hpp
    ./src/sensory_controller_collision_contacts.cpp 
    ./include/aiold/sensory_controller_sight.hpp
    ./src/sensory_controller_sight.cpp 
    ./include/aiold/sensory_controller_ray_cast_sight.hpp 
    ./src/sensory_controller_ray_cast_sight.cpp 

    ./include/aiold/action_controller.hpp
    ./src/action_controller.cpp

    ./include/aiold/detail/action_controller_interpolator.hpp
    ./src/detail/action_controller_interpolator.cpp
    ./include/aiold/detail/action_controller_interpolator_composed.hpp
    ./src/detail/action_controller_interpolator_composed.cpp
    ./include/aiold/detail/action_controller_interpolator_animation.hpp
    ./src/detail/action_controller_interpolator_animation.cpp
    ./include/aiold/detail/action_controller_interpolator_look_at.hpp
    ./src/detail/action_controller_interpolator_look_at.cpp
    ./include/aiold/detail/action_controller_interpolator_aim_at.hpp
    ./src/detail/action_controller_interpolator_aim_at.cpp

    ./include/aiold/detail/action_controller_roller.hpp
    ./src/detail/action_controller_roller.cpp

    ./include/aiold/detail/interpolation_speed_solver.hpp
    ./src/detail/interpolation_speed_solver.cpp

    ./include/aiold/input_devices.hpp

    ./include/aiold/blackboard.hpp
    ./src/blackboard.cpp
    ./include/aiold/blackboard_agent.hpp
    ./src/blackboard_agent.cpp
    ./include/aiold/blackboard_device.hpp
    ./src/blackboard_device.cpp

    ./include/aiold/scene_basic_types_binding.hpp
    ./include/aiold/scene.hpp

    ./include/aiold/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp
    
    ./include/aiold/skeleton_utils.hpp
    ./src/skeleton_utils.cpp

    ./include/aiold/property_map.hpp
    ./src/property_map.cpp

    ./include/aiold/retina.hpp

    ./include/aiold/env/snapshot.hpp
    ./src/env/snapshot.cpp
    ./include/aiold/env/snapshots_cache.hpp
    ./src/env/snapshots_cache.cpp

    ./include/aiold/env/sinet/snapshot_encoder.hpp 
    ./src/env/sinet/snapshot_encoder.cpp 
    ./include/aiold/env/sinet/number_encoder.hpp 
    ./src/env/sinet/number_encoder.cpp
    ./include/aiold/env/sinet/motion_desire_decoder.hpp
    ./src/env/sinet/motion_desire_decoder.cpp
    ./include/aiold/env/sinet/number_decoder.hpp 
    ./src/env/sinet/number_decoder.cpp 

    ./include/aiold/device_id.hpp

    ./include/aiold/device_kind.hpp
    ./src/device_kind.cpp

    ./include/aiold/device.hpp
    ./src/device.cpp
    
    ./include/aiold/devices.hpp
    ./src/devices.cpp

    ./include/aiold/sensor_id.hpp

    ./include/aiold/sensor_kind.hpp
    ./src/sensor_kind.cpp

    ./include/aiold/sensor.hpp
    ./src/sensor.cpp
    
    ./include/aiold/sensors.hpp
    ./src/sensors.cpp

    ./include/aiold/sensor_action.hpp
    ./src/sensor_action.cpp
    ./include/aiold/sensor_action_default_processor.hpp
    ./src/sensor_action_default_processor.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
