set(THIS_TARGET_NAME qtgl)

set(CMAKE_AUTOMOC ON)
#set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${QT5_CXX_FLAGS}")

add_library(${THIS_TARGET_NAME}

    ./include/qtgl/window.hpp
    ./include/qtgl/detail/window.hpp
    ./src/window.cpp

    ./include/qtgl/notification_listener.hpp
    ./src/notification_listener.cpp

    ./include/qtgl/widget_base.hpp

    ./include/qtgl/real_time_simulator.hpp
    ./src/real_time_simulator.cpp

    ./include/qtgl/glapi.hpp
    ./src/glapi.cpp

    ./include/qtgl/draw.hpp
    ./src/draw.cpp

    ./include/qtgl/shader.hpp
    ./src/shader.cpp

    ./include/qtgl/buffer.hpp
    ./src/buffer.cpp

    ./include/qtgl/texture.hpp
    ./src/texture.cpp
    ./include/qtgl/texture_database.hpp
    ./src/texture_database.cpp

    ./include/qtgl/camera.hpp
    ./src/camera.cpp

    ./include/qtgl/coordinate_system.hpp
    ./src/coordinate_system.cpp

    ./include/qtgl/free_fly.hpp
    ./src/free_fly.cpp

    ./include/qtgl/window_props.hpp
    ./src/window_props.cpp

    ./include/qtgl/keyboard_props.hpp
    ./src/keyboard_props.cpp

    ./include/qtgl/mouse_props.hpp
    ./src/mouse_props.cpp

    ./include/qtgl/buffer_generators.hpp
    ./src/create_grid.cpp

    ./include/qtgl/gui_utils.hpp
    ./src/gui_utils.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
