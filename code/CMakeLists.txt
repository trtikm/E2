#* Configuration in Qt Creator (for Debug build):
#    * Build direcroy:
#        /home/marek/root/E2/code/build/Debug
#    * CMake arguments :
#        -DCMAKE_BUILD_TYPE=Debug
#        -DBOOST_ROOT="/home/marek/Software/boost_1_55_0/dist"
#    * Build step (make) :
#        - make install

project(E2)

cmake_minimum_required(VERSION 2.8)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=c++11 -Wall -fPIC")
    if ((CMAKE_BUILD_TYPE STREQUAL Debug) OR
        (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
        add_definitions("-ggdb3")
    endif()
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../dist")
endif()
set(CMAKE_INSTALL_RPATH "./")

# find and add Boost
#set(BOOST_ROOT "/home/marek/Software/boost_1_55_0/dist")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
find_package(Boost COMPONENTS
                system
                thread
                chrono
                filesystem
                log
                log_setup
                python
             REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}" "${Boost_LIBRARY_DIRS}")
add_definitions("-DBOOST_LOG_DYN_LINK -DBOOST_LOG_SETUP")

# find and add Python
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})

# here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/utility/include"
    "${PROJECT_SOURCE_DIR}/cellab/include"
    )
add_subdirectory(./utility)
add_subdirectory(./cellab)

# here follow all shared libraries which provide python access to
# the corresponding E2 libraries
add_subdirectory(./pycellab)
