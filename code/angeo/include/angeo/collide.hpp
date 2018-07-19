#ifndef ANGEO_COLLIDE_HPP_INCLUDED
#   define ANGEO_COLLIDE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

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
 *
 * NOTE: This algorithm can be used to compute collision of the line and sphere (the point being
 *       the centre). Use comparison |*output_closest_point - point| <= sphere_radius.
 *
 * NOTE: This algorithm can be used to compute collision of a capsule (the line being its axis)
 *        and sphere (the point being the centre). Use comparison
 *        |*output_closest_point - point| <= sphere_radius + capsule_radius.
 *
 */
float_32_bit  closest_point_on_line_to_point(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  point,
        vector3* const  output_closest_point
        );


/**
* It computes one or two pairs of closest points on the passed lines. The case when two pairs are
* returned may only occur when both lines are parallel. Each point in each pair can be obtained
* directly and/or parameter of the point on the corresponding line can be obtained. 
*
* INPUT PARAMS:
*
* @param line_1_begin                   The first end point of the first line.
* @param line_1_end                     The second end point of the first line.
* 
* @param line_2_begin                   The first end point of the second line.
* @param line_2_end                     The second end point of the second line.
*
* OUTPUT PARAMS:
*
* output_line_1_closest_point_1         A pointer to memory where to write the first closest point on the first line.
* output_line_1_closest_point_param_1   A pointer to memory where to write the parameter of first closest point on the first line.
* output_line_2_closest_point_1         A pointer to memory where to write the first closest point on the second line.
* output_line_2_closest_point_param_1   A pointer to memory where to write the parameter of first closest point on the second line.
*
* output_line_1_closest_point_2         A pointer to memory where to write the second closest point on the first line.
* output_line_1_closest_point_param_2   A pointer to memory where to write the parameter of second closest point on the first line.
* output_line_2_closest_point_2         A pointer to memory where to write the second closest point on the second line.
* output_line_2_closest_point_param_2   A pointer to memory where to write the parameter of second closest point on the second line.
*
* RETURN VALUE:
*
* @return A number of pairs of closest points computed. The value can either be 1 or 2.
*
* NOTE: This algorithm can be used to compute collision of the line and sphere (the point being
*       the centre). Use comparison |*output_closest_point - point| <= sphere_radius.
*
* NOTE: This algorithm can be used to compute collision of two capsules (the lines being their axes). Use comparison
*       |*output_line_1_closest_point_1 - *output_line_2_closest_point_1| <= capsule_1_radius + capsule_2_radius.
*       If 2 pairs are returned, then add also this comparion
*       |*output_line_1_closest_point_2 - *output_line_2_closest_point_2| <= capsule_1_radius + capsule_2_radius.
*
*/
natural_32_bit  closest_points_of_two_lines(
        // INPUT SECTION:

        // The first line
        vector3 const&  line_1_begin,
        vector3 const&  line_1_end,

        // The second line
        vector3 const&  line_2_begin,
        vector3 const&  line_2_end,

        // OUTPUT SECTION:

        // Pair 1 of clossest points (always available):
        // - the closest point on the first line
        vector3*  output_line_1_closest_point_1,
        float_32_bit*  output_line_1_closest_point_param_1,
        // - the closest point on the second line
        vector3*  output_line_2_closest_point_1,
        float_32_bit*  output_line_2_closest_point_param_1,

        // Pair 2 of clossest points (may be available only when both lines are parallel)
        // - the closest point on the first line
        vector3*  output_line_1_closest_point_2,
        float_32_bit*  output_line_1_closest_point_param_2,
        // - the closest point on the second line
        vector3*  output_line_2_closest_point_2,
        float_32_bit*  output_line_2_closest_point_param_2
        );


/**
 * It computes the point of the passed axis-aligned boundig box which is the closes to the passed point.
 *
 * @param output_closest_point  Reference to a memory where the nearest point will be stored.
 *                              The references point and output_closest_point may be aliased.
 *
 * NOTE: This algorithm can be used to compute collision of a box (transformed into space where it becomes
 *       the passed axis aligned bounding box) and sphere (the point being the centre).
 *       Use comparison |output_closest_point - point| <= sphere_radius.
 *
 * NOTE: The second function detects whether there is a non-empty intersection.
 *
 */
void  closest_point_of_bbox_to_point(
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3 const&  point,
        vector3&  output_closest_point
        );


/**
* Returns false iff the point is outside the axis-aligned bounding box.
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
 *
 * NOTE: This algorithm can be used to compute collision of a the plane and a sphere (the point being the centre).
 *       Use comparison *output_distance_to_plane <= sphere_radius.
 *
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


/**
 * Computes and intersection of two axis-aligned bounding boxes.
 *
 * Aliasing of any "intersection_bbox_*_corner" with any "bbox_*_*_corner" is allowed. In that
 * case the "bbox_*_*_corner" will be overwritten by the alliased "intersection_bbox_*_corner".
 * However, if "intersection_bbox_low_corner" and "intersection_bbox_high_corner" are aliased,
 * then the result is undefined.
 *
 * NOTE: the second function only detects whether there is a non-empty intersection.
 *
 */
bool  collision_bbox_bbox(
        vector3 const&  bbox_0_low_corner,
        vector3 const&  bbox_0_high_corner,
        vector3 const&  bbox_1_low_corner,
        vector3 const&  bbox_1_high_corner,
        vector3&  intersection_bbox_low_corner,
        vector3&  intersection_bbox_high_corner
        );
bool  collision_bbox_bbox(
        vector3 const&  bbox_0_low_corner,
        vector3 const&  bbox_0_high_corner,
        vector3 const&  bbox_1_low_corner,
        vector3 const&  bbox_1_high_corner
        );


enum struct POINT_SET_TYPE : natural_8_bit
{
    EMPTY = 0,
    GENERAL = 1,
    FULL = 2
};


struct clipped_polygon_description
{
    std::size_t  index_start;
    std::size_t  index_end;

    vector2 point_start;
    vector2 point_end;

    scalar param_start;
    scalar param_end;
};


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,    // The first and the last point must be the same.
        vector2 const&  clip_origin,
        vector2 const&  clip_normal,
        clipped_polygon_description* const  description
        );


POINT_SET_TYPE  instersection_of_plane_with_xy_coord_plane(
        vector3 const&  origin,
        vector3 const&  normal,
        vector2&  intersection_origin,
        vector2&  intersection_normal
        );


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,    // The first and the last point must be the same.
        matrix44 const&  to_polygon_space_matrix,
        vector3 const&  clip_origin,
        vector3 const&  clip_normal,
        clipped_polygon_description* const  description
        );


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,    // The first and the last point must be the same.
        matrix44 const&  to_polygon_space_matrix,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::vector<vector2>* const  output_clipped_polygon_points,
        std::vector<natural_32_bit>* const  output_indices_of_intersection_points
        );


}

#endif
