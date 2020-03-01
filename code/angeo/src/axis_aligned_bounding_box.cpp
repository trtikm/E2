#include <angeo/axis_aligned_bounding_box.hpp>
#include <array>

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


axis_aligned_bounding_box  compute_aabb_of_box(
        vector3 const&  origin,
        vector3 const&  basis_x_vector,
        vector3 const&  basis_y_vector,
        vector3 const&  basis_z_vector,
        vector3 const&  half_sizes_along_axes
        )
{
    vector3 const  v(
            half_sizes_along_axes(0) * absolute_value(basis_x_vector(0)) +
            half_sizes_along_axes(1) * absolute_value(basis_y_vector(0)) +
            half_sizes_along_axes(2) * absolute_value(basis_z_vector(0)) ,

            half_sizes_along_axes(0) * absolute_value(basis_x_vector(1)) +
            half_sizes_along_axes(1) * absolute_value(basis_y_vector(1)) +
            half_sizes_along_axes(2) * absolute_value(basis_z_vector(1)) ,

            half_sizes_along_axes(0) * absolute_value(basis_x_vector(2)) +
            half_sizes_along_axes(1) * absolute_value(basis_y_vector(2)) +
            half_sizes_along_axes(2) * absolute_value(basis_z_vector(2))
            );

    return  { origin - v, origin + v };
}


void  compute_union_bbox(
        axis_aligned_bounding_box const&  bbox_1,
        axis_aligned_bounding_box const&  bbox_2,
        axis_aligned_bounding_box&  union_bbox
        )
{
    for (std::size_t j = 0UL; j != 3UL; ++j)
    {
        union_bbox.min_corner(j) = std::min(bbox_1.min_corner(j), bbox_2.min_corner(j));
        union_bbox.max_corner(j) = std::max(bbox_1.max_corner(j), bbox_2.max_corner(j));
    }
}


void  extend_union_bbox(
        axis_aligned_bounding_box&  union_bbox,
        vector3 const&  point
        )
{
    compute_union_bbox(union_bbox, { point, point }, union_bbox);
}


void  transform_bbox(
        axis_aligned_bounding_box const&  bbox,
        matrix44 const&  transformation,
        axis_aligned_bounding_box&  transformed_bbox
        )
{
    std::array<vector3, 8UL> const  corners {
        transform_point(vector3(bbox.min_corner(0), bbox.min_corner(1), bbox.min_corner(2)), transformation),
        transform_point(vector3(bbox.max_corner(0), bbox.min_corner(1), bbox.min_corner(2)), transformation),
        transform_point(vector3(bbox.max_corner(0), bbox.max_corner(1), bbox.min_corner(2)), transformation),
        transform_point(vector3(bbox.min_corner(0), bbox.max_corner(1), bbox.min_corner(2)), transformation),

        transform_point(vector3(bbox.min_corner(0), bbox.min_corner(1), bbox.max_corner(2)), transformation),
        transform_point(vector3(bbox.max_corner(0), bbox.min_corner(1), bbox.max_corner(2)), transformation),
        transform_point(vector3(bbox.max_corner(0), bbox.max_corner(1), bbox.max_corner(2)), transformation),
        transform_point(vector3(bbox.min_corner(0), bbox.max_corner(1), bbox.max_corner(2)), transformation),
    };

    transformed_bbox.min_corner = corners.at(0U);
    transformed_bbox.max_corner = corners.at(0U);
    for (std::size_t  i = 1UL; i != corners.size(); ++i)
        for (std::size_t j = 0UL; j != 3UL; ++j)
        {
            if (transformed_bbox.min_corner(j) > corners.at(i)(j))
                transformed_bbox.min_corner(j) = corners.at(i)(j);
            if (transformed_bbox.max_corner(j) < corners.at(i)(j))
                transformed_bbox.max_corner(j) = corners.at(i)(j);
        }
}


}
