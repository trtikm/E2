project(E2)

cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

if(NOT CMAKE_BUILD_TYPE)
    message("Build type not set => setting 'Release' as default.")
    set(CMAKE_BUILD_TYPE "Release"  CACHE STRING "Release" FORCE)
endif()
message("Build type = " ${CMAKE_BUILD_TYPE})
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_definitions(-DRELEASE)
else()
    message(SEND_ERROR "Unknown build type. Use either Debug, Release, or RelWithDebInfo. E.g.: -DCMAKE_BUILD_TYPE=Release")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-Wall -Wno-unused-local-typedefs")
    if ((CMAKE_BUILD_TYPE STREQUAL Debug) OR
        (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
        add_definitions("-ggdb3")
    endif()
    message("Compiler = GCC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_definitions("/bigobj")
    #add_definitions("/Wall")
    add_definitions("-D_ENABLE_EXTENDED_ALIGNED_STORAGE")
    message("Compiler = MSVC")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_definitions("
        -Wno-dangling-else
        -Wno-overloaded-virtual
        -Wno-unused-lambda-capture
        -Wno-potentially-evaluated-expression
        -Wno-#pragma-messages
        -Wno-deprecated-declarations
        ")
    message("Compiler = Clang/LLVM")
else()
    message(SEND_ERROR "Unknown (unsupported) compiler detected. Supported is only GCC (on Linux) and MSVC (on Windows).")
endif()

macro(append_compiler_flags FLAGS)
  set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${FLAGS}")
endmacro()

macro(append_linker_flags FLAGS)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${FLAGS}")
endmacro()

if(EMSCRIPTEN)
    message("Target platform: WEBASSEMBLY")

    set(CMAKE_EXECUTABLE_SUFFIX ".html")
    add_definitions("
        -Wno-dangling-else
        -Wno-overloaded-virtual
        -Wno-unused-lambda-capture
        -Wno-potentially-evaluated-expression
        -Wno-#pragma-messages
        -Wno-deprecated-declarations
        ")
    add_compile_definitions(WEBASSEMBLY)
    append_linker_flags("-s ALLOW_MEMORY_GROWTH=1")
    append_linker_flags("-s USE_GLFW=3 -s USE_WEBGL2=1") #"-O2 -s USE_SDL=2 -s USE_WEBGL2=1"
	append_linker_flags("-s MIN_WEBGL_VERSION=2 -s MAX_WEBGL_VERSION=2")
    append_linker_flags("-s GL_ASSERTIONS=1")
    append_linker_flags("-s GL_DEBUG=1")
    append_linker_flags("-s LLD_REPORT_UNDEFINED")
    append_linker_flags("-s NO_DISABLE_EXCEPTION_CATCHING")

    # find and add Eigen math library
    message("Searching for Eigen ...")
    find_package(Eigen3 CONFIG REQUIRED)
    set(EIGEN_LIST_OF_LIBRARIES_TO_LINK_WITH "Eigen3::Eigen")
    message("Eigen3 found at: ${EIGEN3_INCLUDE_DIRS}")
    add_definitions( ${EIGEN3_DEFINITIONS} )
    include_directories("${EIGEN3_INCLUDE_DIRS}")

    # find and add LODEPNG library
    message("Searching for LODEPNG ...")
    find_package(lodepng CONFIG REQUIRED)
    find_path(lodepng_INCLUDES lodepng.h)
    include_directories("${lodepng_INCLUDES}")
    set(LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH "lodepng")

    #set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH "")
    set(OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH "")
    set(GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH "")
    set(GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH "")
else()
    message("Target platform: PC")

    # find and add Boost
    message("Searching for Boost library ...")
    find_package(Boost COMPONENTS
                    program_options
                 REQUIRED)
    set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH
            ${Boost_LIBRARIES}
        )

    # find and add Eigen math library
    message("Searching for Eigen ...")
    find_package(Eigen3 CONFIG REQUIRED)
    set(EIGEN_LIST_OF_LIBRARIES_TO_LINK_WITH "Eigen3::Eigen")
    message("Eigen3 found at: ${EIGEN3_INCLUDE_DIRS}")
    add_definitions( ${EIGEN3_DEFINITIONS} )
    include_directories("${EIGEN3_INCLUDE_DIRS}")

    # find and add OpenGL
    message("Searching for OpenGL ...")
    find_package(OpenGL REQUIRED)
    # include_directories(${OPENGL_INCLUDE_DIRS})
    link_directories(${OPENGL_LIBRARY_DIRS})
    add_definitions(${OPENGL_DEFINITIONS})
    set(OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH
            ${OPENGL_LIBRARIES}
        )
    if(NOT OPENGL_INCLUDE_DIR)
        message("OpenGL includes were found in directory: <empty>")
    else()
        message("OpenGL includes were found in directory: " ${OPENGL_INCLUDE_DIR})
    endif()

    # find and add GLAD library
    message("Searching for GLAD ...")
    find_package(glad CONFIG REQUIRED)
    set(GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH "glad::glad")

    # find and add GLFW library
    message("Searching for GLFW ...")
    find_package(glfw3 CONFIG REQUIRED)
    set(GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH "glfw")

    # find and add LODEPNG library
    message("Searching for LODEPNG ...")
    find_package(lodepng CONFIG REQUIRED)
    find_path(lodepng_INCLUDES lodepng.h)
    include_directories("${lodepng_INCLUDES}")
    set(LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH "lodepng")
endif()

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist" CACHE STRING "Install path" FORCE)
set(CMAKE_INSTALL_RPATH "./")
#set(CMAKE_BINARY_DIR "${PROJECT_SOURCE_DIR}/build" CACHE STRING "Build directory" FORCE)
set(E2_3RD_INCLUDE_DIR "${CMAKE_TOOLCHAIN_FILE}/../../../installed/${VCPKG_TARGET_TRIPLET}/include")
include_directories(${E2_3RD_INCLUDE_DIR})
message("3rd includes directory = ${E2_3RD_INCLUDE_DIR}")
message("Sources directory = ${PROJECT_SOURCE_DIR}/code")
message("Install directory = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/code")

# Add project specific code
add_subdirectory(./code)

install(DIRECTORY ./data DESTINATION . PATTERN Blender EXCLUDE)

message("Generating build files ...")
