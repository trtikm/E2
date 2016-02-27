#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


texture_ptr  create_chessboard_texture(
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type,
                            natural_32_bit const  texture_mag_filtering_type
                            )
{
    TMPROF_BLOCK();

    static float const  image_data[2U*2U*3U] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
    };
    natural_8_bit const* const  begin = (natural_8_bit const*)image_data;
    natural_8_bit const* const  end = begin + sizeof(image_data);
    return texture::create(2U,2U,image_bytes_sequence(begin,end),GL_RGB,GL_FLOAT,
                           texture_pixel_format,
                           texture_x_wrapping_type,
                           texture_y_wrapping_type,
                           texture_min_filtering_type,
                           texture_mag_filtering_type
                           );
}


}
