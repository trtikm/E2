#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


boost::filesystem::path  chessboard_texture_file_path()
{
    return "generic/texture/chessboard_texture";
}


boost::filesystem::path  chessboard_texture_image_path()
{
    return chessboard_texture_file_path() / "image";
}


std::string  chessboard_texture_id()
{
    return "[texture]:" + chessboard_texture_file_path().string();
}


texture  make_chessboard_texture(
            natural_32_bit const  pixel_format,
            natural_32_bit const  x_wrapping_type,
            natural_32_bit const  y_wrapping_type,
            natural_32_bit const  min_filtering_type
            )
{
    static float const  image_data[2U*2U*3U] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
    };

    return texture(
            0U,
            texture_file(
                chessboard_texture_image_path(),
                pixel_format,
                x_wrapping_type,
                y_wrapping_type,
                min_filtering_type,
                GL_NEAREST,
                chessboard_texture_file_path()
                ),
            texture_image(
                2U, 2U,
                (natural_8_bit const*)image_data,
                (natural_8_bit const*)image_data + sizeof(image_data),
                GL_RGB, GL_FLOAT,
                chessboard_texture_image_path()
                ),
            chessboard_texture_id()
            );
}


}
