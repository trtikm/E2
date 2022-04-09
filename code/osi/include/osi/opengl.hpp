#ifndef OSI_OPENGL_HPP_INCLUDED
#   define OSI_OPENGL_HPP_INCLUDED

#   include <utility/config.hpp>
#   if PLATFORM() == PLATFORM_WEBASSEMBLY()
#       include <webgl/webgl2.h>
#       include <GLFW/glfw3.h>
#   else
#       include <glad/glad.h>
#   endif

#endif
