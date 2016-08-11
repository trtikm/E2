set(THIS_TARGET_NAME efloop_construction_and_simulation)

add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp
    run.cpp

    my_cell.hpp
    my_synapse.hpp
    my_signalling.hpp

    my_neural_tissue.cpp
    my_neural_tissue.hpp

    my_environment.cpp
    my_environment.hpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    efloop
    cellab
    envlab
    utility
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tests")
