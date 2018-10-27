set(THIS_TARGET_NAME angeo)

add_library(${THIS_TARGET_NAME}
    ./include/angeo/tensor_math.hpp
    ./src/tensor_math.cpp

    ./include/angeo/coordinate_system.hpp
    ./src/coordinate_system.cpp

    ./include/angeo/collide.hpp
    ./src/collide.cpp

    ./include/angeo/utility.hpp
    ./src/utility.cpp

    ./include/angeo/shape.hpp

    ./include/angeo/proximity_map.hpp
    
    ./include/angeo/axis_aligned_bounding_box.hpp
    ./src/axis_aligned_bounding_box.cpp

    ./include/angeo/collision_shape_id.hpp
    ./include/angeo/collision_shape_feature_id.hpp
    ./include/angeo/collision_object_id.hpp
    ./include/angeo/collision_object_id_pair.hpp
    ./include/angeo/collision_object_and_shape_feature_id.hpp
    ./include/angeo/collision_object_acceptor.hpp
    ./include/angeo/collision_material.hpp

    ./include/angeo/contact_id.hpp
    ./include/angeo/contact_acceptor.hpp

    ./include/angeo/collision_scene.hpp
    ./src/collision_scene.cpp

    ./include/angeo/rigid_body.hpp
    ./include/angeo/motion_constraint_system.hpp
    ./src/motion_constraint_system.cpp
    ./include/angeo/rigid_body_simulator.hpp
    ./src/rigid_body_simulator.cpp
    
    ./include/angeo/friction_coefficients.hpp
    ./src/friction_coefficients.cpp
    
    ./include/angeo/mass_and_inertia_tensor.hpp
    ./src/mass_and_inertia_tensor.cpp
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "lib")
