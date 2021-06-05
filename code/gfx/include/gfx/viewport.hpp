#ifndef GFX_VIEWPORT_HPP_INCLUDED
#   define GFX_VIEWPORT_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace gfx {


struct  viewport
{
    viewport();
    viewport(
            float_32_bit const  left_,
            float_32_bit const  right_,
            float_32_bit const  bottom_,
            float_32_bit const  top_,
            float_32_bit const  pixel_width_mm_,
            float_32_bit const  pixel_height_mm_
            );

    bool  is_point_inside(vector2 const&  p) const;

    float_32_bit  width() const { return right - left + 1.0f; }
    float_32_bit  height() const { return top - bottom + 1.0f; }

    float_32_bit  left;
    float_32_bit  right;
    float_32_bit  bottom;
    float_32_bit  top;
    float_32_bit  pixel_width_mm;
    float_32_bit  pixel_height_mm;
};


void  make_current(viewport const&  vp);


}

#endif
