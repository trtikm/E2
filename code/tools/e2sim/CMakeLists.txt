set(THIS_TARGET_NAME e2sim)



add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp
    run.cpp
    )

if(EMSCRIPTEN)
    set_target_properties(${THIS_TARGET_NAME}
        PROPERTIES
        LINK_FLAGS "--preload-file ${CMAKE_SOURCE_DIR}/data@/"
        )
endif()

target_link_libraries(${THIS_TARGET_NAME}
    ai
    angeo
    osi
    com
    gfx
    netlab
    utility
    ${EIGEN_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ai
    angeo
    osi
    com
    gfx
    netlab
    utility
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

    
if(EMSCRIPTEN)
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools/webgl")
    file(GLOB target_install_files "${CMAKE_CURRENT_BINARY_DIR}/*_Emscripten_*")
    install(FILES ${target_install_files} DESTINATION "tools/webgl")
else()
    install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")
endif()

