#ifndef GFX_TEXTURE_GENERATORS_HPP_INCLUDED
#   define GFX_TEXTURE_GENERATORS_HPP_INCLUDED

#   include <osi/opengl.hpp>
#   include <gfx/texture.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <boost/filesystem/path.hpp>
#   include <string>

namespace gfx {


boost::filesystem::path  chessboard_texture_file_path();
boost::filesystem::path  chessboard_texture_image_path();
std::string  chessboard_texture_id();


texture  make_chessboard_texture(
            natural_32_bit const  pixel_format = GL_RGBA,
            natural_32_bit const  x_wrapping_type = GL_REPEAT,
            natural_32_bit const  y_wrapping_type = GL_REPEAT,
            natural_32_bit const  min_filtering_type = GL_LINEAR_MIPMAP_LINEAR
            );


}


#endif
