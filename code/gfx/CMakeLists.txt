set(THIS_TARGET_NAME gfx)

add_library(${THIS_TARGET_NAME}

    ./include/gfx/draw.hpp
    ./src/draw.cpp

    ./include/gfx/draw_state.hpp
    ./src/draw_state.cpp

    ./include/gfx/shader.hpp
    ./src/shader.cpp

    ./include/gfx/shader_data_bindings.hpp
    ./src/shader_data_bindings.cpp

    ./include/gfx/shader_data_linkers.hpp
    ./src/shader_data_linkers.cpp

    ./include/gfx/shader_compose.hpp
    ./src/shader_compose.cpp
    
    ./include/gfx/effects_config.hpp
    ./src/effects_config.cpp

    ./include/gfx/buffer.hpp
    ./src/buffer.cpp

    ./include/gfx/texture.hpp
    ./src/texture.cpp

    ./include/gfx/image.hpp
    ./src/image.cpp

    ./include/gfx/texture_generators.hpp
    ./src/texture_generator_chessboard.cpp

    ./include/gfx/batch.hpp
    ./src/batch.cpp

    ./include/gfx/batch_available_resources.hpp
    ./src/batch_available_resources.cpp

    ./include/gfx/batch_generators.hpp
    ./src/batch_generator_grid.cpp
    ./src/batch_generator_lines3d.cpp
    ./src/batch_generator_basis_vectors.cpp
    ./src/batch_generator_coord_cross.cpp
    ./src/batch_generator_arrow.cpp
    ./src/batch_generator_wireframe_box.cpp
    ./src/batch_generator_wireframe_capsule.cpp
    ./src/batch_generator_wireframe_sphere.cpp
    ./src/batch_generator_sprite.cpp
    ./src/batch_generator_text.cpp
    ./src/batch_generator_triangle_mesh.cpp
    ./src/batch_generator_perspective_frustum.cpp

    ./include/gfx/camera.hpp
    ./src/camera.cpp

    ./include/gfx/camera_utils.hpp
    ./src/camera_utils.cpp

    ./include/gfx/free_fly.hpp
    ./src/free_fly.cpp

    ./include/gfx/modelspace.hpp
    ./src/modelspace.cpp

    ./include/gfx/keyframe.hpp
    ./src/keyframe.cpp

    ./include/gfx/spatial_boundary.hpp 
    
    ./include/gfx/skeleton_alignment.hpp
    ./src/skeleton_alignment.cpp

    ./include/gfx/detail/as_json.hpp 
    ./include/gfx/detail/from_json.hpp 
    ./src/detail/from_json.cpp 
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
