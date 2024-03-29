#include <gfx/image.hpp>
#include <utility/msgstream.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/canonical_path.hpp>
#include <filesystem>
#include <lodepng.h>
#include <algorithm>
#include <stdexcept>

namespace gfx { 


image_rgba_8888::image_rgba_8888(natural_32_bit const  width_, natural_32_bit const  height_)
    : width(width_)
    , height(height_)
    , data()
{
    data.resize(width * height * 4U, 0U);
}


void  load_png_image(std::filesystem::path const&  path, image_rgba_8888&  img)
{
    TMPROF_BLOCK();

    ASSUMPTION(std::filesystem::is_regular_file(path));

    unsigned int width, height;
    unsigned int error_code = lodepng::decode(img.data, width, height, path.string().c_str(), LCT_RGBA, 8U);
    if (error_code != 0)
        throw std::runtime_error(msgstream() << "lodepng::decode() failed to load PNG image '" << path.string()
                                             << "'. Error code=" << error_code << ", message=" << lodepng_error_text(error_code));
    INVARIANT((unsigned int)img.data.size() == width * height * 4U);

    img.width = (natural_32_bit)width;
    img.height = (natural_32_bit)height;
}


void  save_png_image(std::filesystem::path const&  path, image_rgba_8888 const&  img)
{
    TMPROF_BLOCK();
    ASSUMPTION(img.width > 0U && img.height > 0U);
    lodepng::encode(path.string(), img.data, img.width, img.height, LCT_RGBA, 8U);
}


void  flip_image_vertically(image_rgba_8888&  img)
{
    for (int lo = 0, hi = (int)img.height - 1; lo < hi; ++lo, --hi)
        for (natural_32_bit*  lo_ptr = (natural_32_bit*)img.data.data() + lo * (int)img.width,
                           *  lo_end = lo_ptr + img.width,
                           *  hi_ptr = (natural_32_bit*)img.data.data() + hi * (int)img.width;
                lo_ptr != lo_end; ++lo_ptr, ++hi_ptr)
            std::swap(*lo_ptr, *hi_ptr);
}


}
