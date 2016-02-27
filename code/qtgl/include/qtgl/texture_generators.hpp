#ifndef QTGL_TEXTURE_GENERATORS_HPP_INCLUDED
#   define QTGL_TEXTURE_GENERATORS_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>

namespace qtgl {


texture_ptr  create_chessboard_texture(
                            natural_32_bit const  texture_pixel_format = GL_COMPRESSED_RGBA,
                            natural_32_bit const  texture_x_wrapping_type = GL_REPEAT,
                            natural_32_bit const  texture_y_wrapping_type = GL_REPEAT,
                            natural_32_bit const  texture_min_filtering_type = GL_LINEAR_MIPMAP_LINEAR,
                            natural_32_bit const  texture_mag_filtering_type = GL_NEAREST
                            );


}


#endif
