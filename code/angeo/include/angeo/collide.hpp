#ifndef ANGEO_COLLIDE_HPP_INCLUDED
#   define ANGEO_COLLIDE_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_shape_feature_id.hpp>
#   include <angeo/coordinate_system.hpp>
#   include <vector>

namespace angeo {


enum struct  COORDINATE : natural_8_bit
{
    X = 0,
    Y = 1,
    Z = 2,
    W = 3,
    U = 4,
    V = 5,
};


inline natural_8_bit  as_number(COORDINATE const  coord) noexcept
{
    return (natural_8_bit)coord;
}


inline COORDINATE  as_coordinate(natural_8_bit const  index)
{
    ASSUMPTION(index <= as_number(COORDINATE::V));
    return (COORDINATE)index;
}


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
* NOTE: This algorithm can be used to compute collision of two capsules (the lines being their axes). Use comparison
*       |*output_line_1_closest_point_1 - *output_line_2_closest_point_1| <= capsule_1_radius + capsule_2_radius.
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
 * The returned 'collision_shape_feature_id' instance assumes this classification of cylinder features:
 *      - FACE 0 : All cylinder points inside plane
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_begin) = 0'
 *                 except points of the circle (cylinder_axis_line_begin, cylinder_radius) in that plane.
 *      - FACE 1 : All cylinder points inside plane
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_end) = 0'
 *                 except points of the circle (cylinder_axis_line_end, cylinder_radius) in that plane.
 *      - FACE 2 : All cylinder points satisfying:
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_begin) > 0'
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_end) < 0'
 *      - EDGE 0 : All cylinder points of the circle (cylinder_axis_line_begin, cylinder_radius) in the plane
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_begin) = 0'
 *      - EDGE 1 : All cylinder points of the circle (cylinder_axis_line_end, cylinder_radius) in the plane
 *                 '(cylinder_axis_line_end - cylinder_axis_line_begin) * (X - cylinder_axis_line_end) = 0'
 */
collision_shape_feature_id  closest_point_on_cylinder_to_point(
        vector3 const&  cylinder_axis_line_begin,
        vector3 const&  cylinder_axis_line_end,
        float_32_bit const  cylinder_radius,
        vector3 const&  point,
        vector3&  output_closest_point
        );


/**
 * The front face of the triangle is determined by unit normal vector. It is assumed
 * the normal vector points to the half-space of the front face of the triangle.
 *
 * If cull_triangle_back_side is true and the passed point does NOT lies in the half-space
 * of the FRONT face of the triangle, then the function does NOT write any data to memory
 * pointed to by 'output_*' parameters (even if they are not 'nullptr') and returns 'false'
 * immediatelly. Otherwise, the function writes computed data to all memory locations pointed
 * to be all not 'nullptr' output parameters 'output_*', and returns 'true'.
 *
 * NOTE: The interpretation of a value stored to memory pointed to by output_parameter_ptr
 *       depends on location of the closest point on the triangle. If the closest point is
 *       on the edge, then the written value is the parameter of the cloest point on the line.
 *       If the closest point is in the triangle's interior, then the written value is the
 *       distance of the triangle to the passed point. If the closest point is one of the
 *       triangle vertices, them the written value has no particular meaning (igore it).
 *
 * NOTE: The alogorithm can also be used for computation of collision between triangle and
 *       a sphere. Use then a comparison (if the function returns 'true'):
 *          |*output_triangle_closest_point_ptr - point| <= sphere_radius.
 */
bool  closest_point_of_triangle_to_point(
        vector3 const&  triangle_vertex_1,
        vector3 const&  triangle_vertex_2,
        vector3 const&  triangle_vertex_3,
        vector3 const&  unit_normal_vector,
        natural_8_bit const  edges_ignore_mask,
        bool const  cull_triangle_back_side,
        vector3 const&  point,
        vector3*  output_triangle_closest_point_ptr,
        collision_shape_feature_id*  output_triangle_shape_feature_id_ptr,
        float_32_bit*  output_parameter_ptr
        );

/// The front face of the triangle is the one having counter-clock-wise orientation
/// of vertices. The computed unit normal points to the half-space of the front face
/// of the triangle.
inline bool  closest_point_of_triangle_to_point(
        vector3 const&  triangle_vertex_1,
        vector3 const&  triangle_vertex_2,
        vector3 const&  triangle_vertex_3,
        natural_8_bit const  edges_ignore_mask,
        bool const  cull_triangle_back_side,
        vector3 const&  point,
        vector3*  output_triangle_closest_point_ptr,
        collision_shape_feature_id*  output_triangle_shape_feature_id_ptr,
        float_32_bit*  output_parameter_ptr
        )
{
    return closest_point_of_triangle_to_point(
                triangle_vertex_1,
                triangle_vertex_2,
                triangle_vertex_3,
                normalised(cross_product(triangle_vertex_2 - triangle_vertex_1, triangle_vertex_3 - triangle_vertex_1)),
                edges_ignore_mask,
                cull_triangle_back_side,
                point,
                output_triangle_closest_point_ptr,
                output_triangle_shape_feature_id_ptr,
                output_parameter_ptr
                );
}


/**
 * Ensure that 'triangle_unit_normal_vector' agrees with the counter-clock-wise order of triangle's verices.
 *   ASSUMPTION(dot_product(normal_T12, triangle_vertex_3 - triangle_vertex_1) < 0.0f);
 * Here is interpretation/use of 'triangle_edges_ignore_mask' parameter:
 *   if ((triangle_edges_ignore_mask & 1U) != 0U)
 *      ignore contacts with edge (triangle_vertex_1, triangle_vertex_2)
 *   if ((triangle_edges_ignore_mask & 2U) != 0U)
 *      ignore contacts with edge (triangle_vertex_2, triangle_vertex_3)
 *   if ((triangle_edges_ignore_mask & 4U) != 0U)
 *      ignore contacts with edge (triangle_vertex_3, triangle_vertex_1)
 */
natural_32_bit  closest_points_of_triangle_and_line(
        vector3 const&  triangle_vertex_1,
        vector3 const&  triangle_vertex_2,
        vector3 const&  triangle_vertex_3,
        vector3 const&  triangle_unit_normal_vector,
        natural_8_bit const  triangle_edges_ignore_mask,

        vector3  line_point_1,
        vector3  line_point_2,

        vector3*  output_triangle_closest_point_1,
        collision_shape_feature_id*  output_triangle_shape_feature_id_1,
        vector3*  output_line_closest_point_1,
        collision_shape_feature_id*  output_line_shape_feature_id_1,

        vector3*  output_triangle_closest_point_2,
        collision_shape_feature_id*  output_triangle_shape_feature_id_2,
        vector3*  output_line_closest_point_2,
        collision_shape_feature_id*  output_line_shape_feature_id_2
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
inline vector3  closest_point_of_bbox_to_point(
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3 const&  point
        )
{
    vector3  output_closest_point;
    closest_point_of_bbox_to_point(bbox_low_corner, bbox_high_corner, point, output_closest_point);
    return output_closest_point;
}


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
 *                                  second corner point of the sub-line intersecting with
 *                                  the boundig box will be stored, if the passed values
 *                                  is NOT nullptr.
 * @param skip_coord_index      Allows to skip clipping of the line in the passed coordinate index.
 *                              Valid values (i.e. coord. indices) are 0, 1, and 2. Any other passed
 *                              value will not cause skipping of any coordinate.
 */
bool  clip_line_into_bbox(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3* const  clipped_line_begin,
        vector3* const  clipped_line_end,
        float_32_bit* const  parameter_of_line_begin,
        float_32_bit* const  parameter_of_line_end,
        natural_32_bit const  skip_coord_index = std::numeric_limits<natural_32_bit>::max()
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


// Returns the number of closest points
natural_32_bit  closest_points_of_bbox_and_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,

        vector3&  output_bbox_closest_point_1,
        vector3&  output_line_closest_point_1,

        // Additional (i.e. not necessarily closest) points lying either both inside
        // the bbox or on and above the closest face of the box to the line.
        vector3&  output_bbox_closest_point_2,
        vector3&  output_line_closest_point_2
        );


inline bool  is_point_inside_sphere(
        vector3 const&  point_in_sphere_local_space,
        float_32_bit const  sphere_radius
        )
{
    return length_squared(point_in_sphere_local_space) <= sphere_radius * sphere_radius;
}

bool  is_point_inside_capsule(
        vector3 const&  point_in_sphere_local_space,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line
        );


float_32_bit  distance_from_center_of_capsule_to_surface_in_direction(
        float_32_bit const  half_distance_between_end_points, // end-points of the central line are aligned along the z-axis.
        float_32_bit const  thickness_from_central_line,
        vector3 const&  unit_direction
        );


inline float_32_bit  distance_from_center_of_sphere_to_surface(float_32_bit const  radius) { return radius; }


struct  convex_polyhedron
{
    std::vector<std::vector<vector2> >  polygons;
    std::vector<coordinate_system_explicit>  polygon_frames;    // Each coord system is expressed in the space of the polyhedron.
                                                                // The basis_vector_z() of each coord system represents the unit
                                                                // normal vector of polygon's plane. The nomal vector points inside
                                                                // the polyhedron.
};


enum struct POINT_SET_TYPE : natural_8_bit
{
    EMPTY = 0,
    GENERAL = 1,
    FULL = 2
};


struct clipped_polygon_description
{
    // INVARIANT: the fields below are valid only when the algorithm 'clip_polygon' below return POINT_SET_TYPE::GENERAL.

    std::size_t  index_start;   // Index of the first vertex of the original polygon which is in front of the clip plane.
    std::size_t  index_end;     // Index of the last vertex of the original polygon which is in front of the clip plane.
                                // NOTE: if index_end < index_start, then the indices of the original polygon in front of
                                //       the clip plane are [index_start,..,N-1,0,..,index_end], where N is number of the
                                //       vertices in the original polygon.

    vector2 point_start;        // The start point of the intersection edge of the clipped polygon with the clip plane.
    vector2 point_end;          // The end point of the intersection edge of the clipped polygon with the clip plane.

    scalar param_start;         // INVARIANT: point_start = P[index_end] + param_start * (P[index_end + 1] - P[index_end])
    scalar param_end;           // INVARIANT: point_end = P[index_start - 1] + param_end * (P[index_start] - P[index_start - 1])
                                // where 'P' is the array of vertices of the original polygon.
};


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,    // The first and the last point must be the same.
                                                        // Polygon points below the clip plane are clipped away.
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
                                                        // Polygon points below the clip plane are clipped away.
        matrix44 const&  to_polygon_space_matrix,
        vector3 const&  clip_origin,
        vector3 const&  clip_normal,
        clipped_polygon_description* const  description
        );


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,    // The first and the last point must be the same.
                                                        // Polygon points below any clip plane are clipped away.
        matrix44 const&  to_polygon_space_matrix,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,  // First is origin, second unit normal
        std::vector<vector2>* const  output_clipped_polygon_points,
        std::vector<collision_shape_feature_id>* const  output_collision_shape_feature_ids,
        std::vector<natural_32_bit>* const  output_indices_of_intersection_points
        );


/*
  Box polygons and feature indixes of faces, edges, and vertices
  ==============================================================
  Legend: <face-index>@<vertex/face-index>


                       4@3           7              4@2
                     4---------------------------------7
                2@0 /|1@3                         1@0 /| 3@3
                   / |                               / |
                  /  |       y -----1               /  |
                 /   |             /               /   |
               4/    |            /               /6   |
               /     |           x               /     |
              /      |               y          /      |
             / 1@2   |       5       |     1@1 /       |            Here is the local coord. system of box on the left:
            5---------------------------------6 3@2    |
         2@1| 5@0    |               |     5@1|        |11              Z
            |        |               4----- x |        |                |
            |       8|                        |    y   |                |
            |        |                        |    |   |                |
            |        |                        |    |   |                |
            |    2   |                        |    |   |                O----------- Y
            |   /|   |                        |    3   |               /
            |  / |   |                      10|   /    |              /
            | x  |   |                        |  /     |             /
            |    y   |      5------ x         | x      |            X
           9|        | 4@0  |                 |     4@1|
            |    2@3 0------|-----------------|--------3            The z-axis of the coord. system of each face points inside
            |       / 0@0   |       3         |    0@3/ 3@0         the box.
            |      /        y                 |      /
            |     /                           |     /
            |    /0              0----- y     |    /
            |   /               /             |   / 2
            |  /               /              |  /
            | /               x               | /
        2@2 |/5@3                          5@2|/ 3@1
            1---------------------------------2
              0@1           1              0@2

*/

void  compute_polygons_of_box(
        coordinate_system_explicit const&  location,
        vector3 const&  half_sizes_along_axes,
        convex_polyhedron&  output_polygons
        );


natural_16_bit  compute_feature_index_of_face_of_box(natural_16_bit const  face_index);
natural_16_bit  compute_feature_index_of_face_of_box(
        natural_8_bit const  dominant_coordinate_index,
        float_32_bit const  dominant_coordinate
        );


natural_16_bit  compute_feature_index_of_vertex_of_box(
        natural_16_bit const  face_feature_index,
        natural_16_bit const  vertex_index_in_face
        );
natural_16_bit  compute_feature_index_of_vertex_of_box(vector3 const&  point);


natural_16_bit  compute_feature_index_of_edge_of_box(
        natural_16_bit const  face_feature_index,
        natural_16_bit const  edge_index_in_face
        );
natural_16_bit  compute_feature_index_of_edge_of_box_from_two_faces(
        natural_16_bit  face_1_feature_index,
        natural_16_bit  face_2_feature_index
        );


natural_16_bit  compute_feature_index_of_feature_of_box(
        natural_16_bit const  face_feature_index,
        COLLISION_SHAPE_FEATURE_TYPE const  feature_type,
        natural_16_bit const  feature_index_in_face
        );


struct  closest_box_feature_to_a_point
{
    COLLISION_SHAPE_FEATURE_TYPE  feature_type;
    vector3  feature_vector_in_world_space; // Not used for feature type VERTEX.
    float_32_bit  distance_to_feature;
    natural_32_bit  feature_index;
};


collision_shape_feature_id  compute_closest_box_feature_to_a_point(
        vector3 const&  point_in_box_local_space,
        vector3 const&  box_half_sizes_along_axes,
        float_32_bit const  max_edge_thickness = 0.005f,
        natural_32_bit* const  output_order_ptr = nullptr,
        natural_8_bit* const  output_coordinate_ptr = nullptr
        );
void  compute_closest_box_feature_to_a_point(
        closest_box_feature_to_a_point&  output,
        vector3 const&  point_in_box_local_space,
        vector3 const&  box_half_sizes_along_axes,
        coordinate_system_explicit const&  box_location_in_word_space,
        float_32_bit const  max_edge_thickness = 0.005f
        );


vector3  compute_box_collision_unit_normal_and_penetration_depth_from_contact_point(
        vector3 const&  common_contact_point_in_world_space,

        coordinate_system_explicit const&  box_1_location,
        vector3 const&  box_1_half_sizes_along_axes,

        coordinate_system_explicit const&  box_2_location,
        vector3 const&  box_2_half_sizes_along_axes,

        float_32_bit&  output_penetration_depth,
        std::pair<collision_shape_feature_id, collision_shape_feature_id>* const  output_collision_shape_feature_id_ptr
        );


bool  collision_box_box(
        coordinate_system_explicit const&  box_1_location,
        vector3 const&  box_1_half_sizes_along_axes,
        convex_polyhedron const&  box_1_polygons,

        coordinate_system_explicit const&  box_2_location,
        vector3 const&  box_2_half_sizes_along_axes,
        convex_polyhedron const&  box_2_polygons,

        vector3* const  ouptut_collision_plane_unit_normal_in_world_space,
        std::vector<vector3>* const  output_collision_points_in_world_space,
        std::vector<float_32_bit>* const  output_penetration_depths_of_collision_points,
        std::vector<std::pair<collision_shape_feature_id, collision_shape_feature_id> >* const  output_collision_shape_feature_ids
        );


}

#endif
