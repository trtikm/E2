# Here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/code/aiold/include"
    "${PROJECT_SOURCE_DIR}/code/angeo/include"
    "${PROJECT_SOURCE_DIR}/code/qtgl/include"
    "${PROJECT_SOURCE_DIR}/code/gfx/include"
    "${PROJECT_SOURCE_DIR}/code/scene/include"
    "${PROJECT_SOURCE_DIR}/code/netlab/include"
    "${PROJECT_SOURCE_DIR}/code/osi/include"
    "${PROJECT_SOURCE_DIR}/code/com/include"
    "${PROJECT_SOURCE_DIR}/code/utility/include"
    )
message("Including the following E2 libraries to the build:")
add_subdirectory(./aiold)
    message("-- aiold")
add_subdirectory(./angeo)
    message("-- angeo")
add_subdirectory(./qtgl)
    message("-- qtgl")
add_subdirectory(./gfx)
    message("-- gfx")
add_subdirectory(./scene)
    message("-- scene")
add_subdirectory(./netlab)
    message("-- netlab")
add_subdirectory(./osi)
    message("-- osi")
add_subdirectory(./com)
    message("-- com")
add_subdirectory(./utility)
    message("-- utility")
    
# Here follow tools, if they should be included.
if(NOT DEFINED E2_BUILD_ALSO_TOOLS)
    set(E2_BUILD_ALSO_TOOLS "Yes" CACHE STRING "Build also tools? (Yes/No)" FORCE)
endif()
message("Build also tools: " ${E2_BUILD_ALSO_TOOLS})
string( TOLOWER "${E2_BUILD_ALSO_TOOLS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    include_directories("${PROJECT_SOURCE_DIR}/code/tools")
    add_subdirectory(./tools)
endif()


# Here follow experiments, if they should be included.
if(NOT DEFINED E2_BUILD_ALSO_EXPERIMENTS)
    set(E2_BUILD_ALSO_EXPERIMENTS "Yes" CACHE STRING "Build also experiments? (Yes/No)" FORCE)
endif()
message("Build also experiments: " ${E2_BUILD_ALSO_EXPERIMENTS})
string( TOLOWER "${E2_BUILD_ALSO_EXPERIMENTS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    include_directories("${PROJECT_SOURCE_DIR}/code/experiments")
    add_subdirectory(./experiments)
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
