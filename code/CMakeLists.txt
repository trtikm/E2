# Here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/code/ai/include"
    "${PROJECT_SOURCE_DIR}/code/angeo/include"
    "${PROJECT_SOURCE_DIR}/code/com/include"
    "${PROJECT_SOURCE_DIR}/code/gfx/include"
    "${PROJECT_SOURCE_DIR}/code/netlab/include"
    "${PROJECT_SOURCE_DIR}/code/osi/include"
    "${PROJECT_SOURCE_DIR}/code/utility/include"
    )
message("Including the following E2 libraries to the build:")
add_subdirectory(./ai)
    message("-- ai")
add_subdirectory(./angeo)
    message("-- angeo")
add_subdirectory(./com)
    message("-- com")
add_subdirectory(./gfx)
    message("-- gfx")
add_subdirectory(./netlab)
    message("-- netlab")
add_subdirectory(./osi)
    message("-- osi")
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
