#- Installation notes:
#        - Use a desktop computer with at least eight hardware threads and 16GB RAM.
#        - Install MS Windows 8.1 or later, or Ubuntu 15.04 or later, or Linux MINT 17.2 or later.
#        - Install GIT (https://git-scm.com/) version 1.9.5 or later
#        - Use GIT to clone E2 repository (https://github.com/trtikm/E2.git) into some your <E2-install-dir>:
#               Create the directory <E2-install-dir>
#               cd <E2-install-dir>
#               git clone https://github.com/trtikm/E2.git .
#          Note that the ending dot '.' (at the line above) is a necessary part of the comand.
#          Also note that you can get E2 sources by downloading a ZIP package from (https://github.com/trtikm/E2)
#          by clicking to the 'Dowload ZIP' button. Then simply unpack the package into the <E2-install-dir>.
#        - Install a C++ compiler supporting C++11 standard and a debugger. Namely, on Windows install Visual Studion 2015
#          and Windows Development Tools (i.e. Windows Kit) 64 bit in order to get 'cdb.exe' debugger which will later be
#          auto-detected and used in Qt Creator. On Linux install GCC including G++ version 4.9 or later and GBD version 7.9
#          or later.
#        - Install CMake (http://www.cmake.org) version 3.0 or later.
#        - Install Python 64 bit (https://www.python.org) version 3.4 or later into some your <python-install-dir>.
#          On Linux install also the development version:
#               sudo apt-get install python3-dev
#        - Install Boost C++ libraries (http://www.boost.org) version 1.59.0 or later into some your <boost-install-dir>.
#          How to build Boost libraries on Windows 8.1 or later using Visual Studio 2015:
#               unpack the downloaded Boost ZIP package into <boost-install-dir>
#               cd <boost-install-dir>
#               .\bootstrap.bat
#               b2 --build-dir="<boost-install-dir>\build\msvc" address-model=64 toolset=msvc-14.0 stage
#          How to build Boost libraries on Linux using GCC:
#               unpack Boost ZIP package into some your <boost-install-dir>
#               cd <boost-install-dir>
#               ./bootstrap.sh --with-python=python3.4
#               ./b2 --build-dir="<boost-install-dir>/build/gcc" address-model=64 toolset=gcc stage
#        - Install Eigen linear algebra library (http://eigen.tuxfamily.org) version 3.2.5 or later into
#          some your <eigen-install-dir> directory. The Eigen library is header only, no building is necessary.
#          So, just unpack the downloaded package into the <eigen-install-dir> directory.
#        - Install Qt5 64 bit version 5.4.1 or later, including Qt Creator version 3.4.2 or later.
#- Configuration of the E2 project in the Qt Creator:
#        - start the Qt Creator
#        - First configure "Kits":
#               - Go to 'Tools>Options>Kits' and select 'Desktop (Default)' kit under 'Manual' kits.
#                 Select proper 'Compiler:' and 'Debugger:'.
#                 On Windows select compiler and debugger respectively:
#                           - Microsoft Visual C++ Compiler 14.0 (amd64)
#                           - Auto-detected CDB at ...\Debuggers\x64\cdb.exe
#                 On Linux  select compiler and debugger respectively:
#                           - GCC (x86 64bit in /usr/bin)
#                           - System GDB at /usr/bin/gdb
#        - Go to 'File>Open File or Project' (the shortcut is Ctrl+O) and open the file:
#               <E2-install-dir>/CMakeLists.txt
#        - Then specify the following build direcroy (according to build type you prefere):
#               <E2-install-dir>/build/<Debug/Release/RelWithDebInfo>
#        - Next, before you run CMake (by clicking the button 'Run CMake'), specify the folloing CMake arguments:
#               -DCMAKE_BUILD_TYPE=<Debug/Release/RelWithDebInfo>    (note: this should correspond to the directory above)
#               -DBOOST_ROOT=<boost-install-dir>
#               -DEIGEN_ROOT=<eigen-install-dir>
#               -DPYTHON_LIBRARY=<python-install-dir>
#          e.g. on MS Windows: -DCMAKE_BUILD_TYPE=Debug -DBOOST_ROOT=c:\3rd\boost_1_59_0 -DEIGEN_ROOT=c:\3rd\eigen_3_2_5 -DPYTHON_LIBRARY=c:\Python34
#          e.g. on Ubuntu: -DCMAKE_BUILD_TYPE=Debug -DBOOST_INSTALL_DIR=/home/marek/Software/boost_1_59_0 -DEIGEN_ROOT=/home/marek/Software/eigen_3_2_5 -DPYTHON_LIBRARY=/usr/lib/python3.4
#        - Then click the 'Run CMake' buttom. If no problem occurred (which is supposed to be the case), you can click
#          the button 'Finish'. Otherwise, you have to fix all issues reported by the CMake.
#        - Click to the button 'Projects' in the horizontal menu on the very left of the Qt Creator's window.
#        - In the section 'Build Septs' and row 'Make:' click to the button 'Details'. Then, in the list of 'Targets:' mark
#          the target 'install'.
#        - And go to 'Build>Build Project "..."' (the shortcut is Ctrl+B) to build E2 project. All created binaries will be
#          automatically installed under '<E2-install-dir>/dist' directory.
#- Recommended (but not necessary) additional packages:
#        - install Graphviz (http://www.graphviz.org) version 2.38 or later
#        - install PyCharm (https://www.jetbrains.com/pycharm) Python development IDE version 4.5.3 or later
#        - install TeX Live (https://www.tug.org/texlive) and TeXstudio (http://www.texstudio.org) version 2.9.4 or later
#          in order to bulid technological papers.


project(E2)

cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

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
set(Python_ADDITIONAL_VERSIONS 3.4.3)
find_package(PythonLibs 3.4.3 REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
link_directories(${PYTHON_LIBRARIES})
set(PYTHON_LIST_OF_LIBRARIES_TO_LINK_WITH
        ${PYTHON_LIBRARIES}
    )
message("Python includes were found in directory: " ${PYTHON_INCLUDE_DIRS})
message("Python libraries were found in directory: " ${PYTHON_LIBRARIES})

# find and add Eigen math library
message("Searching for Eigen ...")
if(NOT DEFINED EIGEN_ROOT)
    set(EIGEN_ROOT "Specify Eigen's installation directory." CACHE STRING "Specify Eigen's installation directory." FORCE)
    message(SEND_ERROR "Eigen root directory was not found!")
else()
    set(EIGEN_INCLUDE_DIRS ${EIGEN_ROOT})
    if(EXISTS "${EIGEN_INCLUDE_DIRS}" AND IS_DIRECTORY "${EIGEN_INCLUDE_DIRS}")
        include_directories(${EIGEN_INCLUDE_DIRS})
        message("Eigen includes were found in directory: " ${EIGEN_INCLUDE_DIRS})
    else()
        message("ERROR: Eigen includes was NOT found!")
        message("EIGEN_ROOT = " ${EIGEN_ROOT})
        message("EIGEN_INCLUDE_DIRS = " ${EIGEN_INCLUDE_DIRS})
        message(FATAL_ERROR "Specify a path to the Eigen math library using -DEIGEN_ROOT=<path-to-eigen_3_2_5>.")
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

# Add project specific code
add_subdirectory(./code)

message("Generating build files ...")
