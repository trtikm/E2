#ifndef QTGL_GLAPI_HPP_INCLUDED
#   define QTGL_GLAPI_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <QOpenGLFunctions_4_2_Core>
#   include <QOpenGLContext>

namespace qtgl {


typedef QOpenGLFunctions_4_2_Core  opengl_api;
typedef QOpenGLContext  opengl_context;

inline constexpr natural_32_bit  opengl_major_version() noexcept { return 4U; }
inline constexpr natural_32_bit  opengl_minor_version() noexcept { return 2U; }

opengl_api&  glapi();
void  swap_buffers();


}

#endif
