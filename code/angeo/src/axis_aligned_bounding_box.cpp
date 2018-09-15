#include <angeo/axis_aligned_bounding_box.hpp>

namespace angeo {


axis_aligned_bounding_box  compute_aabb_of_capsule(
        vector3 const&  point_1,
        vector3 const&  point_2,
        float_32_bit const  radius
        )
{
    return  {
                vector3(std::min(point_1(0), point_2(0)) - radius,
                        std::min(point_1(1), point_2(1)) - radius,
                        std::min(point_1(2), point_2(2)) - radius
                        ),
                vector3(std::max(point_1(0), point_2(0)) + radius,
                        std::max(point_1(1), point_2(1)) + radius,
                        std::max(point_1(2), point_2(2)) + radius
                        )
            };
}


axis_aligned_bounding_box  compute_aabb_of_line(
        vector3 const&  point_1,
        vector3 const&  point_2
        )
{
    return  {
                vector3(std::min(point_1(0), point_2(0)),
                        std::min(point_1(1), point_2(1)),
                        std::min(point_1(2), point_2(2))
                        ),
                vector3(std::max(point_1(0), point_2(0)),
                        std::max(point_1(1), point_2(1)),
                        std::max(point_1(2), point_2(2))
                        )
            };
}


axis_aligned_bounding_box  compute_aabb_of_triangle(
        vector3 const&  point_1,
        vector3 const&  point_2,
        vector3 const&  point_3
        )
{
    return  {
                vector3(std::min(std::min(point_1(0), point_2(0)), point_3(0)),
                        std::min(std::min(point_1(1), point_2(1)), point_3(1)),
                        std::min(std::min(point_1(2), point_2(2)), point_3(2))
                        ),
                vector3(std::max(std::max(point_1(0), point_2(0)), point_3(0)),
                        std::max(std::max(point_1(1), point_2(1)), point_3(1)),
                        std::max(std::max(point_1(2), point_2(2)), point_3(2))
                        )
            };
}


}
