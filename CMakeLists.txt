project(E2)
#
# This is the main (root) build script for all C++ source code of the project.
# You can find a lot of useful information about installation and building
# of the source code on all supported patform in the project documentation:
#
#   - offline session: ./index.html
#   - online session: https://rawgit.com/trtikm/E2/master/index.html
#

cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

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
                system
                thread
                chrono
                log_setup
                log
                filesystem
                program_options
                python
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

# find and add Python
message("Searching for Python ...")
set(Python_ADDITIONAL_VERSIONS 3.4)
find_package(PythonLibs 3.4 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})
set(PYTHON_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${PYTHON_LIBRARIES}
    )
message("Python includes were found in directory: " ${PYTHON_INCLUDE_DIRS})
message("Python libraries were found in directory: " ${PYTHON_LIBRARIES})

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

# Find the QtWidgets library
message("Searching for Qt5Widgets ...")
if(NOT DEFINED QT5_INSTALL_DIR)
    set(QT5_INSTALL_DIR "Specify QT5's installation directory." CACHE STRING "Specify QT5's installation directory." FORCE)
endif()
if(EXISTS "${QT5_INSTALL_DIR}" AND IS_DIRECTORY "${QT5_INSTALL_DIR}")
    set(CMAKE_PREFIX_PATH  ${QT5_INSTALL_DIR})
endif()
find_package(Qt5Widgets REQUIRED)
include_directories(${Qt5Widgets_INCLUDE_DIRS})
link_directories(${Qt5Widgets_LIBRARY_DIRS})
add_definitions(${Qt5Widgets_DEFINITIONS})
set(QT5_CXX_FLAGS ${Qt5Widgets_EXECUTABLE_COMPILE_FLAGS})
if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    if(QT5_CXX_FLAGS)
        string(REPLACE "-fPIC" "     " QT5_CXX_FLAGS ${QT5_CXX_FLAGS})
    endif()
    set(QT5_CXX_FLAGS "${QT5_CXX_FLAGS} /wd4373")
endif()
set(QT5_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${Qt5Widgets_LIBRARIES}
    )
message("Qt5Widgets includes was found in directory: " ${Qt5Widgets_INCLUDE_DIRS})
message("Qt5Widgets libraries was found in directory: " ${Qt5Widgets_LIBRARY_DIRS})


# Add project specific code
add_subdirectory(./code)

install(
    DIRECTORY
    ./data/shared
    DESTINATION
    ./data
    )

message("Generating build files ...")
