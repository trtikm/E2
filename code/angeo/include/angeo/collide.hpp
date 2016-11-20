#ifndef ANGEO_COLLIDE_HPP_INCLUDED
#   define ANGEO_COLLIDE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace angeo {


/**
 * It computes a point to a parameter 't' on the line 'X = line_begin + t * (line_end - line_begin)'
 * in range [0,1] such that X is the closest point to the passed one.
 *
 * @param line_begin    The first corner of the line.
 * @param line_end      The second corner of the line.
 * @param point         It is the point for which the function searches the closest point on the line.
 * @param output_closest_point  Pointer to a memory where the nearest point will be stored,
 *                              if the pointer is not nullptr.
 * @return A parameter to the closest point to the passed one.
 *         Value 0.0f means it is 'line_begin' and value 1.0f
 *         means it is line_end.
 */
float_32_bit  closest_point_on_line_to_point(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  point,
        vector3* const  output_closest_point
        );


/**
 *
 * @param point
 * @param bbox_low_corner
 * @param bbox_high_corner
 * @return
 */
bool  collision_point_and_bbox(
        vector3 const&  point,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner
        );


/**
 * It determines a collision state between a point and a plane.
 *
 * @param point
 *              Any point.
 * @param plane_origin
 *              A point on the plane.
 * @param plane_unit_normal
 *              The unit normal vector of the plane.
 * @param output_distance_to_plane
 *              It is a distance of the passed point to the nearest point on the plane.
 *              A negative value indicates it is a distance from behind of the plane.
 *              The value nullptr can be passed indicating 'not interested in this output'.
 * @param output_nearest_point_in_plane
 *              It is the point on the plane which is the nearest to the passed point.
 *              The value nullptr can be passed indicating 'not interested in this output'.
 */
void  collision_point_and_plane(
        vector3 const&  point,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal,
        float_32_bit*  const  output_distance_to_plane,
        vector3* const  output_nearest_point_in_plane
        );


/**
 * It determines a collision state between a ray and a plane.
 *
 * @param ray_origin
 *              A point on the ray.
 * @param ray_unit_direction
 *              The unit vector representing the direction of the ray.
 * @param plane_origin
 *              A point on the plane.
 * @param plane_unit_normal
 *              The unit normal vector of the plane.
 * @param output_cos_angle_between_plane_normal_and_ray_direction
 *              The value nullptr can be passed indicating 'not interested in this output'.
 * @param output_distance_of_ray_origin_to_plane
 *              It is a normal (nearest) distance of the ray_origin to the plane.
 *              The value nullptr can be passed indicating 'not interested in this output'.
 * @param output_distance_to_intersection_point
 *              The value nullptr can be passed indicating 'not interested in this output'.
 * @param output_intersection_point
 *              The value nullptr can be passed indicating 'not interested in this output'.
 * @return It returns false, if cosine of angle between plane_unit_normal and ray_unit_direction
 *         is less than 1e-3f. In that case, values output_distance_to_intersection_point
 *         and output_intersection_point are NOT written (even if passed not nullptr).
 */
bool  collision_ray_and_plane(
        vector3 const&  ray_origin,
        vector3 const&  ray_unit_direction,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal,
        float_32_bit*  const  output_cos_angle_between_plane_normal_and_ray_direction,
        float_32_bit*  const  output_distance_of_ray_origin_to_plane,
        float_32_bit*  const  output_distance_to_intersection_point,
        vector3* const  output_intersection_point
        );


/**
 * It checks whether the line has not empty intersection with the bounding box.
 * If yes, then there are computed corner points of the sub-line representing
 * the intersection and the function returns true. Otherwise the function
 * returns false.
 *
 * @param line_begin    The first corner point of the line
 * @param line_end      The second corner point of the line
 * @param bbox_low_corner   The low-corner point of the bounding box
 * @param bbox_high_corner  The high-corner point of the bounding box
 * @param clipped_line_begin    Pointer to memory where the first corner point of the
 *                              sub-line intersecting with the boundig box will be stored,
 *                              if the passed values is NOT nullptr.
 * @param clipped_line_end      Pointer to memory where the second corner point of the
 *                              sub-line intersecting with the boundig box will be stored,
 *                              if the passed values is NOT nullptr.
 * @param parameter_of_line_begin   Pointer to memory where the line-parameter to the
 *                                  first corner point of the sub-line intersecting with
 *                                  the boundig box will be stored, if the passed values
 *                                  is NOT nullptr.
 * @param parameter_of_line_end     Pointer to memory where the line-parameter to the
 *                                  first corner point of the sub-line intersecting with
 *                                  the boundig box will be stored, if the passed values
 *                                  is NOT nullptr.
 */
bool  clip_line_into_bbox(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3* const  clipped_line_begin,
        vector3* const  clipped_line_end,
        float_32_bit* const  parameter_of_line_begin,
        float_32_bit* const  parameter_of_line_end
        );


}

#endif
