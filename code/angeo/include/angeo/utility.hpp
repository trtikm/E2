#ifndef ANGEO_UTILITY_HPP_INCLUDED
#   define ANGEO_UTILITY_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace angeo {


template<typename  output_forward_iterator_type>
void  get_corner_points_of_bounding_box(
        vector3 const&  bbox_lo,
        vector3 const&  bbox_hi,
        output_forward_iterator_type  output_iterator
        )
{
    *output_iterator = bbox_lo;                                         ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_lo(1), bbox_lo(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_hi(1), bbox_lo(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_lo(0), bbox_hi(1), bbox_lo(2) };   ++output_iterator;

    *output_iterator = vector3{ bbox_lo(0), bbox_lo(1), bbox_hi(2) };   ++output_iterator;
    *output_iterator = vector3{ bbox_hi(0), bbox_lo(1), bbox_hi(2) };   ++output_iterator;
    *output_iterator = bbox_hi;                                         ++output_iterator;
    *output_iterator = vector3{ bbox_lo(0), bbox_hi(1), bbox_hi(2) };   ++output_iterator;
}


}

#endif
