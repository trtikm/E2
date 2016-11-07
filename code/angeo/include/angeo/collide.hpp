#ifndef ANGEO_COLLIDE_HPP_INCLUDED
#   define ANGEO_COLLIDE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>

namespace angeo {


/**
 * It determines a collision state between a point and a plane.
 *
 * @param point
 *              Any point.
 * @param plane_origin
 *              A point on the plane.
 * @param plane_unit_normal
 *              A unit normal vector of the plane.
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
 *              A unit vector representing the direction of the ray.
 * @param plane_origin
 *              A point on the plane.
 * @param plane_unit_normal
 *              A unit normal vector of the plane.
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


}

#endif
