set(THIS_TARGET_NAME ai)

add_library(${THIS_TARGET_NAME}
    ./include/ai/agents.hpp
    ./src/agents.cpp

    ./include/ai/agent_id.hpp
    ./include/ai/agent.hpp
    ./src/agent.cpp
    
    ./include/ai/cortex.hpp
    ./src/cortex.cpp

    ./include/ai/cortex_mock.hpp
    ./include/ai/cortex_mock_human.hpp
    ./src/cortex_mock_human.cpp

    ./include/ai/cortex_io.hpp

    ./include/ai/cortex_input_encoder.hpp
    ./include/ai/cortex_input_encoder_human.hpp
    ./src/cortex_input_encoder_human.cpp
    
    ./include/ai/cortex_output_decoder.hpp
    ./include/ai/cortex_output_decoder_human.hpp
    ./src/cortex_output_decoder_human.cpp

    ./include/ai/sensory_controller.hpp
    ./include/ai/sensory_controller_human.hpp
    ./src/sensory_controller_human.cpp

    ./include/ai/action_controller.hpp
    ./include/ai/action_controller_human.hpp
    ./src/action_controller_human.cpp

    ./include/ai/input_devices.hpp

    ./include/ai/blackboard.hpp
    ./include/ai/blackboard_human.hpp

    ./include/ai/environment_models.hpp
    ./include/ai/skeleton_composition.hpp
    ./include/ai/skeletal_motion_templates.hpp
    ./src/skeletal_motion_templates.cpp

    ./include/ai/skeleton_utils.hpp
    ./src/skeleton_utils.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
