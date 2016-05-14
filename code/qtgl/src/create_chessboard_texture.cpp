#include <qtgl/texture_generators.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace qtgl {


boost::filesystem::path  chessboard_texture_imaginary_image_path() noexcept
{
    return "/gtgl/generated_textures/chessboard_texture";
}

texture_properties  make_chessboard_texture_properties(
                            natural_32_bit const  texture_pixel_format,
                            natural_32_bit const  texture_x_wrapping_type,
                            natural_32_bit const  texture_y_wrapping_type,
                            natural_32_bit const  texture_min_filtering_type
                            )
{
    return {
        chessboard_texture_imaginary_image_path(),
        texture_pixel_format,
        texture_x_wrapping_type,
        texture_y_wrapping_type,
        texture_min_filtering_type,
        GL_NEAREST
        };
}

texture_ptr  create_chessboard_texture(texture_properties_ptr const  props)
{
    TMPROF_BLOCK();

    ASSUMPTION(props->image_file() == chessboard_texture_imaginary_image_path());
    ASSUMPTION(props->mag_filtering_type() == GL_NEAREST);

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

    return texture::create(image_props,props);
}

texture_ptr  create_chessboard_texture(texture_properties const&  texture_props)
{
    TMPROF_BLOCK();

    ASSUMPTION(texture_props.image_file().empty() ||
               texture_props.image_file() == chessboard_texture_imaginary_image_path());
    ASSUMPTION(texture_props.mag_filtering_type() == GL_NEAREST);

    texture_properties_ptr const  props{ new texture_properties{
            chessboard_texture_imaginary_image_path(),
            texture_props.pixel_format(),
            texture_props.x_wrapping_type(),
            texture_props.y_wrapping_type(),
            texture_props.min_filtering_type(),
            GL_NEAREST
            }};

    return create_chessboard_texture(props);
}


}
