message("Including the following E2 tools to the build:")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/testapps")
    add_subdirectory(./testapps)
endif()

add_subdirectory(./gfxtuner)
    message("-- gfxtuner")

add_subdirectory(./setimagealphachannel)
    message("-- setimagealphachannel")

add_subdirectory(./e2sim)
    message("-- e2sim")

add_subdirectory(./fontcfg)
    message("-- fontcfg")
