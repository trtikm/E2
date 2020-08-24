set(THIS_TARGET_NAME netlab_cortex_functions)



add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp
    run.cpp

    simulator_base.hpp
    simulator_base.cpp

    exp_utils.hpp
    exp_utils.cpp

    exp_inhibitory_weight.hpp
    exp_inhibitory_weight.cpp

    exp_spiking_const_input_signal.hpp
    exp_spiking_const_input_signal.cpp

    exp_spiking_trains.hpp
    exp_spiking_trains.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    ai
    angeo
    netlab
    gfx
    osi
    com
    utility
    ${OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "experiments")

