set(THIS_TARGET_NAME gfxtuner)

set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_CXX_FLAGS}")

add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp
    run.cpp

    simulator.hpp
    simulator.cpp
    simulator_notifications.hpp

    scene.hpp
    scene.cpp
    
    gfx_object.hpp
    gfx_object.cpp
    
    scene_selection.hpp 
    scene_selection.cpp 

    scene_edit_utils.hpp 
    scene_edit_utils.cpp 

    scene_history.hpp 
    scene_history.cpp 

    scene_utils.hpp
    scene_utils.cpp

    program_window.hpp
    program_window.cpp
    
    status_bar.hpp
    status_bar.cpp
    
    menu_bar.hpp
    menu_bar.cpp
    
    window_tabs/tab_draw.hpp
    window_tabs/tab_draw.cpp

    window_tabs/tab_scene.hpp
    window_tabs/tab_scene.cpp

    window_tabs/tab_statistics.hpp
    window_tabs/tab_statistics.cpp
    
    window_tabs/tab_statistics_resources.hpp
    window_tabs/tab_statistics_resources.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    angeo
    qtgl
    utility
    ${QT5_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH}
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")

