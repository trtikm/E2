project(E2)
#
# This is the main (root) build script for all C++ source code of the project.
# You can find a lot of useful information about installation and building
# of the source code on all supported patform in the project documentation:
#
#   - offline session: ./index.html
#   - online session: https://rawgit.com/trtikm/E2/master/index.html
#

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

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=c++11 -Wall -Wno-unused-local-typedefs")
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
else()
    message(SEND_ERROR "Unknown (unsupported) compiler detected. Supported is only GCC (on Linux) and MSVC (on Windows).")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/dist" CACHE STRING "Install path" FORCE)
endif()
set(CMAKE_INSTALL_RPATH "./")
message("Sources directory = ${PROJECT_SOURCE_DIR}/code")
message("Install directory = ${CMAKE_INSTALL_PREFIX}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/code")

# find and add Boost
message("Searching for Boost library ...")
if(NOT DEFINED BOOST_INSTALL_DIR)
    set(BOOST_INSTALL_DIR "Specify boost's installation directory." CACHE STRING "Specify boost's installation directory." FORCE)
endif()
if(EXISTS "${BOOST_INSTALL_DIR}" AND IS_DIRECTORY "${BOOST_INSTALL_DIR}")
    set(BOOST_ROOT ${BOOST_INSTALL_DIR})
endif()
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
find_package(Boost COMPONENTS
                chrono
                log_setup
                log
                filesystem
                program_options
             REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
add_definitions("-DBOOST_LOG_SETUP")
set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${Boost_LIBRARIES}
    )
if(DEFINED BOOST_ROOT)
    message("Boost was found in directory: " ${BOOST_ROOT})
    message("Boost includes were found in directory: " ${Boost_INCLUDE_DIRS})
    message("Boost libraries were found in directory: " ${Boost_LIBRARY_DIRS})
else()
    message(SEND_ERROR "The installation directory of Boost was not FOUND!")
endif()

# find and add Eigen math library
message("Searching for Eigen ...")
if(NOT DEFINED EIGEN_INSTALL_DIR)
    set(EIGEN_INSTALL_DIR "Specify Eigen's installation directory." CACHE STRING "Specify Eigen's installation directory." FORCE)
    message(SEND_ERROR "Eigen installation directory was not found!")
else()
    set(EIGEN_INCLUDE_DIRS ${EIGEN_INSTALL_DIR})
    if(EXISTS "${EIGEN_INCLUDE_DIRS}" AND IS_DIRECTORY "${EIGEN_INCLUDE_DIRS}")
        include_directories(${EIGEN_INCLUDE_DIRS})
        message("Eigen includes were found in directory: " ${EIGEN_INCLUDE_DIRS})
    else()
        message("ERROR: Eigen includes was NOT found!")
        message("EIGEN_INSTALL_DIR = " ${EIGEN_INSTALL_DIR})
        message("EIGEN_INCLUDE_DIRS = " ${EIGEN_INCLUDE_DIRS})
        message(FATAL_ERROR "Specify a path to the Eigen math library using -DEIGEN_INSTALL_DIR=<path-to-eigen-install-dir>.")
    endif()
endif()

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
if(NOT DEFINED GLAD_INSTALL_DIR)
    set(GLAD_INSTALL_DIR "Specify GLAD's installation directory." CACHE STRING "Specify GLADS's installation directory." FORCE)
    message(SEND_ERROR "GLAD installation directory was not found!")
else()
    set(GLAD_INCLUDE_DIR "${GLAD_INSTALL_DIR}/include")
    if(EXISTS "${GLAD_INCLUDE_DIR}" AND IS_DIRECTORY "${GLAD_INCLUDE_DIR}")
        include_directories(${GLAD_INCLUDE_DIR})
        message("GLAD includes were found in directory: " ${GLAD_INCLUDE_DIR})
    else()
        message("ERROR: GLAD includes was NOT found!")
        message("GLAD_INSTALL_DIR = " ${GLAD_INSTALL_DIR})
        message("GLAD_INCLUDE_DIR = " ${GLAD_INCLUDE_DIR})
        message(FATAL_ERROR "Specify a correct path to the GLAD install directory -DGLAD_INSTALL_DIR=<path-to-GLAD-install-dir>.")
    endif()
    set(GLAD_LIBRARY_FILE "${GLAD_INSTALL_DIR}/lib/glad.lib")
    if(EXISTS "${GLAD_LIBRARY_FILE}")
        set(GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH
                ${GLAD_LIBRARY_FILE}
            )
        message("GLAD static library was found: " ${GLAD_LIBRARY_FILE})
    else()
        message("ERROR: GLAD static library was NOT found!")
        message("GLAD_INSTALL_DIR = " ${GLAD_INSTALL_DIR})
        message("GLAD_INCLUDE_DIR = " ${GLAD_INCLUDE_DIR})
        message("GLAD_LIBRARY_FILE = " ${GLAD_LIBRARY_FILE})
        message(FATAL_ERROR "Specify a correct path to the GLAD install directory -DGLAD_INSTALL_DIR=<path-to-GLAD-install-dir>.")
    endif()
endif()

# find and add GLFW library
message("Searching for GLFW ...")
if(NOT DEFINED GLFW_INSTALL_DIR)
    set(GLFW_INSTALL_DIR "Specify GLFW's installation directory." CACHE STRING "Specify GLFWS's installation directory." FORCE)
    message(SEND_ERROR "GLFW installation directory was not found!")
else()
    set(GLFW_INCLUDE_DIR "${GLFW_INSTALL_DIR}/include")
    if(EXISTS "${GLFW_INCLUDE_DIR}" AND IS_DIRECTORY "${GLFW_INCLUDE_DIR}")
        include_directories(${GLFW_INCLUDE_DIR})
        message("GLFW includes were found in directory: " ${GLFW_INCLUDE_DIR})
    else()
        message("ERROR: GLFW includes was NOT found!")
        message("GLFW_INSTALL_DIR = " ${GLFW_INSTALL_DIR})
        message("GLFW_INCLUDE_DIR = " ${GLFW_INCLUDE_DIR})
        message(FATAL_ERROR "Specify a correct path to the GLFW install directory -DGLFW_INSTALL_DIR=<path-to-GLFW-install-dir>.")
    endif()
    set(GLFW_LIBRARY_FILE "${GLFW_INSTALL_DIR}/lib/glfw3.lib")
    if(EXISTS "${GLFW_LIBRARY_FILE}")
        set(GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH
                ${GLFW_LIBRARY_FILE}
            )
        message("GLFW static library was found: " ${GLFW_LIBRARY_FILE})
    else()
        message("ERROR: GLFW static library was NOT found!")
        message("GLFW_INSTALL_DIR = " ${GLFW_INSTALL_DIR})
        message("GLFW_INCLUDE_DIR = " ${GLFW_INCLUDE_DIR})
        message("GLFW_LIBRARY_FILE = " ${GLFW_LIBRARY_FILE})
        message(FATAL_ERROR "Specify a correct path to the GLFW install directory -DGLFW_INSTALL_DIR=<path-to-GLFW-install-dir>.")
    endif()
endif()

# find and add LODEPNG library
message("Searching for LODEPNG ...")
if(NOT DEFINED LODEPNG_INSTALL_DIR)
    set(LODEPNG_INSTALL_DIR "Specify LODEPNG's installation directory." CACHE STRING "Specify LODEPNGS's installation directory." FORCE)
    message(SEND_ERROR "LODEPNG installation directory was not found!")
else()
    set(LODEPNG_INCLUDE_DIR "${LODEPNG_INSTALL_DIR}/include")
    if(EXISTS "${LODEPNG_INCLUDE_DIR}" AND IS_DIRECTORY "${LODEPNG_INCLUDE_DIR}")
        include_directories(${LODEPNG_INCLUDE_DIR})
        message("LODEPNG includes were found in directory: " ${LODEPNG_INCLUDE_DIR})
    else()
        message("ERROR: LODEPNG includes was NOT found!")
        message("LODEPNG_INSTALL_DIR = " ${LODEPNG_INSTALL_DIR})
        message("LODEPNG_INCLUDE_DIR = " ${LODEPNG_INCLUDE_DIR})
        message(FATAL_ERROR "Specify a correct path to the LODEPNG install directory -DLODEPNG_INSTALL_DIR=<path-to-LODEPNG-install-dir>.")
    endif()
    set(LODEPNG_LIBRARY_FILE "${LODEPNG_INSTALL_DIR}/lib/lodepng_${CMAKE_BUILD_TYPE}.lib")
    if(EXISTS "${LODEPNG_LIBRARY_FILE}")
        set(LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH
                ${LODEPNG_LIBRARY_FILE}
            )
        message("LODEPNG static library was found: " ${LODEPNG_LIBRARY_FILE})
    else()
        message("ERROR: LODEPNG static library was NOT found!")
        message("LODEPNG_INSTALL_DIR = " ${LODEPNG_INSTALL_DIR})
        message("LODEPNG_INCLUDE_DIR = " ${LODEPNG_INCLUDE_DIR})
        message("LODEPNG_LIBRARY_FILE = " ${LODEPNG_LIBRARY_FILE})
        message(FATAL_ERROR "Specify a correct path to the LODEPNG install directory -DLODEPNG_INSTALL_DIR=<path-to-LODEPNG-install-dir>.")
    endif()
endif()


# Add project specific code
add_subdirectory(./code)

install(DIRECTORY ./data DESTINATION . PATTERN Blender EXCLUDE)

message("Generating build files ...")
