#ifndef ANGEO_AXIS_ALIGNED_BOUNDING_BOX_HPP_INCLUDED
#   define ANGEO_AXIS_ALIGNED_BOUNDING_BOX_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace angeo {


struct  axis_aligned_bounding_box
{
    vector3  min_corner;
    vector3  max_corner;
};


axis_aligned_bounding_box  compute_aabb_of_capsule(
        vector3 const&  point_1,
        vector3 const&  point_2,
        float_32_bit const  radius
        );


axis_aligned_bounding_box  compute_aabb_of_line(
        vector3 const&  point_1,
        vector3 const&  point_2
        );


axis_aligned_bounding_box  compute_aabb_of_triangle(
        vector3 const&  point_1,
        vector3 const&  point_2,
        vector3 const&  point_3
        );


}

#endif
