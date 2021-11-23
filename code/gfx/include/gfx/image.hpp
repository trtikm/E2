#ifndef GFX_IMAGE_HPP_INCLUDED
#   define GFX_IMAGE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <filesystem>
#   include <vector>

namespace gfx {


struct image_rgba_8888
{
    natural_32_bit  width;
    natural_32_bit  height;
    std::vector<natural_8_bit> data;
};


void  load_png_image(std::filesystem::path const&  path, image_rgba_8888&  img);
void  save_png_image(std::filesystem::path const&  path, image_rgba_8888 const&  img);
void  flip_image_vertically(image_rgba_8888&  img);


}

#endif
