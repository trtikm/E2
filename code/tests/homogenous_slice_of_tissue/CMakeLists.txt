set(THIS_TARGET_NAME homogenous_slice_of_tissue)

add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp

    run.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    cellab
    utility
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_${CMAKE_BUILD_TYPE}"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tests")
