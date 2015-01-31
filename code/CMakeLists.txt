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
    message("Build type not set => setting 'Release' as default.")
    set(CMAKE_BUILD_TYPE "Release")
endif()
message("Build type = " ${CMAKE_BUILD_TYPE})
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    add_definitions(-DDEBUG)
elseif(CMAKE_BUILD_TYPE MATCHES "Release")
    add_definitions(-DRELEASE)
elseif(CMAKE_BUILD_TYPE MATCHES "RelWithDebInfo")
    add_definitions(-DRELEASE)
endif()

if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions("-std=c++11 -Wall -Wno-unused-local-typedefs -fPIC")
    if ((CMAKE_BUILD_TYPE STREQUAL Debug) OR
        (CMAKE_BUILD_TYPE STREQUAL RelWithDebInfo))
        add_definitions("-ggdb3")
    endif()
    message("Compiler = GCC")
else()
    message("Compiler = UNKNOWN")
endif()

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../dist" CACHE STRING "Install path" FORCE)
endif()
set(CMAKE_INSTALL_RPATH "./")
message("Project directory = " ${PROJECT_SOURCE_DIR})
message("Install directory = " ${CMAKE_INSTALL_PREFIX})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(DEFAULT_STATIC_LIB_PREFIX "lib")
    set(DEFAULT_STATIC_LIB_SUFFIX ".a")
    set(DEFAULT_DYNAMIC_LIB_SUFFIX ".so")
else()
    set(DEFAULT_STATIC_LIB_PREFIX "")
    set(DEFAULT_STATIC_LIB_SUFFIX ".lib")
    set(DEFAULT_DYNAMIC_LIB_SUFFIX ".dll")
endif()

# find and add Boost
if(NOT BOOST_ROOT)
    set(BOOST_ROOT "/home/marek/Software/boost_1_55_0/dist" CACHE STRING "A path to Boost library." FORCE)
endif()
message("Searching for Boost library ...")
set(Boost_USE_MULTITHREADED ON)
find_package(Boost REQUIRED)
find_package(Boost COMPONENTS
                system
                thread
                chrono
                filesystem
                log
                log_setup
                program_options
                python
             REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_RPATH}" "${Boost_LIBRARY_DIRS}")
add_definitions("-DBOOST_LOG_DYN_LINK -DBOOST_LOG_SETUP")
set(BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${Boost_LIBRARIES}
    )
message("Boost was found in directory: " ${BOOST_ROOT})
message("Boost includes was found in directory: " ${Boost_INCLUDE_DIRS})
message("Boost libraries was found in directory: " ${Boost_LIBRARY_DIRS})

# find and add Python
message("Searching for Python ...")
find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})
set(PYTHON_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${PYTHON_LIBRARIES}
    )
message("Python includes was found in directory: " ${PYTHON_INCLUDE_DIRS})
message("Python libraries was found in directory: " ${PYTHON_LIBRARIES})

# here follow all libraries of E2
include_directories(
    "${PROJECT_SOURCE_DIR}/cellab/include"
    "${PROJECT_SOURCE_DIR}/cellconnect/include"
    "${PROJECT_SOURCE_DIR}/efloop/include"
    "${PROJECT_SOURCE_DIR}/envlab/include"
    "${PROJECT_SOURCE_DIR}/utility/include"
    )
message("Including the following E2 libraries to the build:")
add_subdirectory(./cellab)
    message("-- cellab")
add_subdirectory(./cellconnect)
    message("-- cellconnect")
add_subdirectory(./efloop)
    message("-- efloop")
add_subdirectory(./envlab)
    message("-- envlab")
add_subdirectory(./utility)
    message("-- utility")

# here follow all shared libraries which provide python access to
# the corresponding E2 libraries
#add_subdirectory(./pycellab)

# here follow tests, if they should be included.
if(NOT E2_BUILD_ALSO_TESTS)
    set(E2_BUILD_ALSO_TESTS "Yes" CACHE STRING "Build also tests? (Yes/No)" FORCE)
endif()
message("Build also tests: " ${E2_BUILD_ALSO_TESTS})
string( TOLOWER "${E2_BUILD_ALSO_TESTS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    add_subdirectory(./tests)
endif()

# here follow benchmarks, if they should be included.
if(NOT E2_BUILD_ALSO_BENCHMARKS)
    set(E2_BUILD_ALSO_BENCHMARKS "Yes" CACHE STRING "Build also benchmarks? (Yes/No)" FORCE)
endif()
message("Build also benchmarks: " ${E2_BUILD_ALSO_BENCHMARKS})
string( TOLOWER "${E2_BUILD_ALSO_BENCHMARKS}" E2_TEMPORARY_VARIBLE)
if(E2_TEMPORARY_VARIBLE STREQUAL "yes")
    message("Including the following E2 benchmarks to the build:")
#    add_subdirectory(./benchmarks/...)
#        message("-- ")
endif()

message("Generating build files ...")

