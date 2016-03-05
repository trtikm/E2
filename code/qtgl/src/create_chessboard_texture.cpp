#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


texture_ptr  create_chessboard_texture(texture_properties_ptr const  texture_props)
{
    TMPROF_BLOCK();

    ASSUMPTION(texture_props->image_file() == chessboard_texture_imaginary_image_path());

    static float const  image_data[2U*2U*3U] = {
        0.0f, 0.0f, 0.0f,   1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,   0.0f, 0.0f, 0.0f
    };
    static texture_image_properties const  image_props{
            2U,2U,
            (natural_8_bit const*)image_data,
            (natural_8_bit const*)image_data + sizeof(image_data),
            GL_RGB,GL_FLOAT
            };

    return texture::create(image_props,texture_props);
}


texture_ptr  create_chessboard_texture(
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type,
                            natural_32_bit const  texture_mag_filtering_type
                            )
{
    texture_properties_ptr const  texture_props{ new texture_properties{
            chessboard_texture_imaginary_image_path(),
            texture_pixel_format,
            texture_x_wrapping_type,
            texture_y_wrapping_type,
            texture_min_filtering_type,
            texture_mag_filtering_type
            }};

    return create_chessboard_texture(texture_props);
}


}
