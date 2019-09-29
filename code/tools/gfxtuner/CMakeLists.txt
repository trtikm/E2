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

    
    program_window.hpp
    program_window.cpp
    
    status_bar.hpp
    status_bar.cpp
    
    menu_bar.hpp
    menu_bar.cpp
    menu_bar_records_integration.hpp
    menu_bar_records_integration.cpp
    menu_bar_record_batch.hpp
    menu_bar_record_batch.cpp
    menu_bar_record_collider.hpp
    menu_bar_record_collider.cpp
    menu_bar_record_rigid_body.hpp
    menu_bar_record_rigid_body.cpp
    menu_bar_record_agent.hpp
    menu_bar_record_agent.cpp
    
    window_tabs/tab_draw.hpp
    window_tabs/tab_draw.cpp

    window_tabs/tab_scene.hpp
    window_tabs/tab_scene.cpp
    window_tabs/tab_scene_bool_lock.hpp
    window_tabs/tab_scene_record_id_reverse_builder.hpp
    window_tabs/tab_scene_record_id_reverse_builder.cpp
    window_tabs/tab_scene_records_integration.hpp
    window_tabs/tab_scene_records_integration.cpp
    window_tabs/tab_scene_tree_widget.hpp
    window_tabs/tab_scene_tree_widget.cpp
    window_tabs/tab_scene_utils.hpp
    window_tabs/tab_scene_utils.cpp
    window_tabs/tab_scene_record_batch.hpp
    window_tabs/tab_scene_record_batch.cpp
    window_tabs/tab_scene_record_collider.hpp
    window_tabs/tab_scene_record_collider.cpp
    window_tabs/tab_scene_record_collider_props_dialog.hpp 
    window_tabs/tab_scene_record_collider_props_dialog.cpp 
    window_tabs/tab_scene_record_rigid_body.hpp
    window_tabs/tab_scene_record_rigid_body.cpp
    window_tabs/tab_scene_record_agent.hpp
    window_tabs/tab_scene_record_agent.cpp

    window_tabs/tab_statistics.hpp
    window_tabs/tab_statistics.cpp
    window_tabs/tab_statistics_resources.hpp
    window_tabs/tab_statistics_resources.cpp
    window_tabs/tab_statistics_draw.hpp
    window_tabs/tab_statistics_draw.cpp
    window_tabs/tab_statistics_collisions.hpp
    window_tabs/tab_statistics_collisions.cpp
    window_tabs/tab_statistics_physics.hpp
    window_tabs/tab_statistics_physics.cpp
    window_tabs/tab_statistics_ai.hpp
    window_tabs/tab_statistics_ai.cpp

    dialog/effects_config_dialog.hpp
    dialog/effects_config_dialog.cpp
    dialog/insert_name_dialog.hpp
    dialog/insert_name_dialog.cpp
    dialog/insert_number_dialog.hpp
    dialog/insert_number_dialog.cpp

    simulation/simulator.hpp
    simulation/simulator.cpp
    simulation/simulator_notifications.hpp

    simulation/bind_ai_scene_to_simulator.hpp
    simulation/bind_ai_scene_to_simulator.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    angeo
    ai
    qtgl
    scene
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

