set(THIS_TARGET_NAME qtgl)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_CXX_FLAGS}")

add_library(${THIS_TARGET_NAME}

    ./include/qtgl/window.hpp

    ./include/qtgl/notification_listener.hpp
    ./src/notification_listener.cpp

    ./include/qtgl/widget_base.hpp

    ./include/qtgl/real_time_simulator.hpp
    ./src/real_time_simulator.cpp

    ./include/qtgl/glapi.hpp
    ./src/glapi.cpp

    ./include/qtgl/draw.hpp
    ./src/draw.cpp

    ./include/qtgl/draw_state.hpp
    ./src/draw_state.cpp

    ./include/qtgl/shader.hpp
    ./src/shader.cpp

    ./include/qtgl/shader_data_bindings.hpp
    ./src/shader_data_bindings.cpp

    ./include/qtgl/shader_compose.hpp
    ./src/shader_compose.cpp
    
    ./include/qtgl/effects_config.hpp
    ./src/effects_config.cpp

    ./include/qtgl/buffer.hpp
    ./src/buffer.cpp

    ./include/qtgl/texture.hpp
    ./src/texture.cpp

    ./include/qtgl/texture_generators.hpp
    ./src/texture_generator_chessboard.cpp

    ./include/qtgl/batch.hpp
    ./src/batch.cpp

    ./include/qtgl/batch_available_resources.hpp
    ./src/batch_available_resources.cpp

    ./include/qtgl/batch_generators.hpp
    ./src/batch_generator_grid.cpp
    ./src/batch_generator_lines3d.cpp
    ./src/batch_generator_basis_vectors.cpp
    ./src/batch_generator_wireframe_box.cpp
    ./src/batch_generator_wireframe_sphere.cpp
    ./src/batch_generator_perspective_frustum.cpp

    ./include/qtgl/camera.hpp
    ./src/camera.cpp

    ./include/qtgl/camera_utils.hpp
    ./src/camera_utils.cpp

    ./include/qtgl/free_fly.hpp
    ./src/free_fly.cpp

    ./include/qtgl/window_props.hpp
    ./src/window_props.cpp

    ./include/qtgl/keyboard_props.hpp
    ./src/keyboard_props.cpp

    ./include/qtgl/mouse_props.hpp
    ./src/mouse_props.cpp

    ./include/qtgl/modelspace.hpp
    ./src/modelspace.cpp

    ./include/qtgl/keyframe.hpp
    ./src/keyframe.cpp

    ./include/qtgl/gui_utils.hpp
    ./src/gui_utils.cpp

    ./include/qtgl/spatial_boundary.hpp 
    
    ./include/qtgl/skeleton_alignment.hpp
    ./src/skeleton_alignment.cpp

    ./include/qtgl/detail/window.hpp
    ./src/detail/window.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
