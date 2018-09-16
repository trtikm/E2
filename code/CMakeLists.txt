# Here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/code/netlab/include"
    "${PROJECT_SOURCE_DIR}/code/netexp/include"
    "${PROJECT_SOURCE_DIR}/code/netview/include"
    "${PROJECT_SOURCE_DIR}/code/angeo/include"
    "${PROJECT_SOURCE_DIR}/code/cellab/include"
    "${PROJECT_SOURCE_DIR}/code/cellconnect/include"
    "${PROJECT_SOURCE_DIR}/code/paralab/include"
    "${PROJECT_SOURCE_DIR}/code/ode/include"
    "${PROJECT_SOURCE_DIR}/code/efloop/include"
    "${PROJECT_SOURCE_DIR}/code/envlab/include"
    "${PROJECT_SOURCE_DIR}/code/qtgl/include"
    "${PROJECT_SOURCE_DIR}/code/scene/include"
    "${PROJECT_SOURCE_DIR}/code/plot/include"
    "${PROJECT_SOURCE_DIR}/code/utility/include"
    )
message("Including the following E2 libraries to the build:")
add_subdirectory(./netlab)
    message("-- netlab")
add_subdirectory(./netexp)
    message("-- netexp")
add_subdirectory(./netview)
    message("-- netview")
add_subdirectory(./angeo)
    message("-- angeo")
add_subdirectory(./cellab)
    message("-- cellab")
add_subdirectory(./cellconnect)
    message("-- cellconnect")
add_subdirectory(./paralab)
    message("-- paralab")
add_subdirectory(./ode)
    message("-- ode")
add_subdirectory(./efloop)
    message("-- efloop")
add_subdirectory(./envlab)
    message("-- envlab")
add_subdirectory(./qtgl)
    message("-- qtgl")
add_subdirectory(./scene)
    message("-- scene")
add_subdirectory(./plot)
    message("-- plot")
add_subdirectory(./utility)
    message("-- utility")
add_subdirectory(./pycellab)
    message("-- pycellab")
    
# Here follow tests, if they should be included.
if(NOT DEFINED E2_BUILD_ALSO_TOOLS)
    set(E2_BUILD_ALSO_TOOLS "Yes" CACHE STRING "Build also tools? (Yes/No)" FORCE)
endif()
message("Build also tools: " ${E2_BUILD_ALSO_TOOLS})
string( TOLOWER "${E2_BUILD_ALSO_TOOLS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    include_directories("${PROJECT_SOURCE_DIR}/code/tools")
    add_subdirectory(./tools)
endif()

    
# Here follow tests, if they should be included.
if(NOT DEFINED E2_BUILD_ALSO_TESTS)
    set(E2_BUILD_ALSO_TESTS "Yes" CACHE STRING "Build also tests? (Yes/No)" FORCE)
endif()
message("Build also tests: " ${E2_BUILD_ALSO_TESTS})
string( TOLOWER "${E2_BUILD_ALSO_TESTS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    include_directories("${PROJECT_SOURCE_DIR}/code/tests")
    add_subdirectory(./tests)
endif()
