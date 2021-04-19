#include <gfx/viewport.hpp>
#include <utility/assumptions.hpp>

namespace gfx {


viewport::viewport(
        float_32_bit const  left_,
        float_32_bit const  right_,
        float_32_bit const  bottom_,
        float_32_bit const  top_,
        float_32_bit const  pixel_width_mm_,
        float_32_bit const  pixel_height_mm_
        )
    : left(left_)
    , right(right_)
    , bottom(bottom_)
    , top(top_)
    , pixel_width_mm(pixel_width_mm_)
    , pixel_height_mm(pixel_height_mm_)
{
    ASSUMPTION(left < right);
    ASSUMPTION(bottom < top);
    ASSUMPTION(pixel_width_mm > 0.0f && pixel_height_mm > 0.0f);
}


}
