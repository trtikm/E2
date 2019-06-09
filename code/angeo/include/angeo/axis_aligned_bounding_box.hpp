#ifndef ANGEO_AXIS_ALIGNED_BOUNDING_BOX_HPP_INCLUDED
#   define ANGEO_AXIS_ALIGNED_BOUNDING_BOX_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace angeo {


struct  axis_aligned_bounding_box
{
    vector3  min_corner;
    vector3  max_corner;
};


inline axis_aligned_bounding_box  compute_aabb_of_sphere(
        vector3 const&  center,
        float_32_bit const  radius
        )
{
    vector3 const  shift(radius, radius, radius);
    return { center - shift, center + shift };
}


inline axis_aligned_bounding_box  compute_aabb_of_sphere(float_32_bit const  radius)
{
    return { vector3(-radius, -radius, -radius), vector3(radius, radius, radius) };
}


axis_aligned_bounding_box  compute_aabb_of_capsule(
        vector3 const&  point_1,
        vector3 const&  point_2,
        float_32_bit const  radius
        );


inline axis_aligned_bounding_box  compute_aabb_of_capsule(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line
        )
{
    return { vector3(-thickness_from_central_line, -thickness_from_central_line, -half_distance_between_end_points),
             vector3(+thickness_from_central_line, +thickness_from_central_line, +half_distance_between_end_points) };
}


axis_aligned_bounding_box  compute_aabb_of_line(
        vector3 const&  point_1,
        vector3 const&  point_2
        );


axis_aligned_bounding_box  compute_aabb_of_triangle(
        vector3 const&  point_1,
        vector3 const&  point_2,
        vector3 const&  point_3
        );


void  compute_union_bbox(
        axis_aligned_bounding_box const&  bbox_1,
        axis_aligned_bounding_box const&  bbox_2,
        axis_aligned_bounding_box&  union_bbox
        );

// Expands 'union_bbox' so that it captures also 'bbox'.
inline void  extend_union_bbox(
        axis_aligned_bounding_box&  union_bbox,
        axis_aligned_bounding_box const&  bbox
        )
{
    compute_union_bbox(union_bbox, bbox, union_bbox);
}


void  extend_union_bbox(
        axis_aligned_bounding_box&  union_bbox,
        vector3 const&  point
        );


inline vector3  center_of_bbox(axis_aligned_bounding_box const&  bbox) { return 0.5f * (bbox.min_corner + bbox.max_corner); }
inline float_32_bit  radius_of_bbox(axis_aligned_bounding_box const&  bbox) { return 0.5f * length(bbox.min_corner - bbox.max_corner); }


void  transform_bbox(
        axis_aligned_bounding_box const&  bbox,
        matrix44 const&  transformation,
        axis_aligned_bounding_box&  transformed_bbox
        );


inline axis_aligned_bounding_box  transform_bbox(
        axis_aligned_bounding_box const&  bbox,
        matrix44 const&  transformation
        )
{
    axis_aligned_bounding_box  transformed_bbox;
    transform_bbox(bbox, transformation, transformed_bbox);
    return transformed_bbox;
}


}

#endif
