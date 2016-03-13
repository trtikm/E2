# Here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/code/cellab/include"
    "${PROJECT_SOURCE_DIR}/code/cellconnect/include"
    "${PROJECT_SOURCE_DIR}/code/paralab/include"
    "${PROJECT_SOURCE_DIR}/code/ode/include"
    "${PROJECT_SOURCE_DIR}/code/efloop/include"
    "${PROJECT_SOURCE_DIR}/code/envlab/include"
    "${PROJECT_SOURCE_DIR}/code/qtgl/include"
    "${PROJECT_SOURCE_DIR}/code/plot/include"
    "${PROJECT_SOURCE_DIR}/code/utility/include"
    )
message("Including the following E2 libraries to the build:")
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
add_subdirectory(./plot)
    message("-- plot")
add_subdirectory(./utility)
    message("-- utility")

# Here follow tests, if they should be included.
if(NOT DEFINED E2_BUILD_ALSO_TESTS)
    set(E2_BUILD_ALSO_TESTS "Yes" CACHE STRING "Build also tests? (Yes/No)" FORCE)
endif()
message("Build also tests: " ${E2_BUILD_ALSO_TESTS})
string( TOLOWER "${E2_BUILD_ALSO_TESTS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    add_subdirectory(./tests)
endif()
