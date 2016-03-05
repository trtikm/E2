#ifndef QTGL_TEXTURE_GENERATORS_HPP_INCLUDED
#   define QTGL_TEXTURE_GENERATORS_HPP_INCLUDED

#   include <qtgl/texture.hpp>
#   include <qtgl/glapi.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>

namespace qtgl {


boost::filesystem::path  chessboard_texture_imaginary_image_path() noexcept;

texture_properties  make_chessboard_texture_properties(
                            natural_32_bit const  texture_pixel_format = GL_COMPRESSED_RGBA,
                            natural_32_bit const  texture_x_wrapping_type = GL_REPEAT,
                            natural_32_bit const  texture_y_wrapping_type = GL_REPEAT,
                            natural_32_bit const  texture_min_filtering_type = GL_LINEAR_MIPMAP_LINEAR
                            );

texture_ptr  create_chessboard_texture(texture_properties_ptr const  props);
texture_ptr  create_chessboard_texture(texture_properties const&  texture_props = make_chessboard_texture_properties());


}


#endif
