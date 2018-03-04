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

    ./include/qtgl/buffer.hpp
    ./src/buffer.cpp

    ./include/qtgl/texture.hpp
    ./src/texture.cpp

    ./include/qtgl/batch.hpp
    ./src/batch.cpp

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

    ./include/qtgl/batch_generators.hpp
    ./src/generate_batch_grid.cpp
    ./src/generate_batch_lines.cpp
    ./src/generate_batch_basis_vectors.cpp
    ./src/generate_batch_box.cpp
    ./src/generate_batch_sphere.cpp
    ./src/generate_batch_perspective_frustum.cpp

    ./include/qtgl/buffer_generators.hpp
    ./src/generate_buffers_grid.cpp
    ./src/generate_buffers_basis.cpp
    ./src/generate_buffers_box.cpp
    ./src/generate_buffers_sphere.cpp
    ./src/generate_buffers_perspective_frustum.cpp

    ./include/qtgl/texture_generators.hpp
    ./src/create_chessboard_texture.cpp

    ./include/qtgl/gui_utils.hpp
    ./src/gui_utils.cpp

    ./include/qtgl/spatial_boundary.hpp 

    ./include/qtgl/detail/window.hpp
    ./src/detail/window.cpp
    ./include/qtgl/detail/read_line.hpp
    ./src/detail/read_line.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
