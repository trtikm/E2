#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>

namespace angeo {


float_32_bit  closest_point_on_line_to_point(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  point,
        vector3* const  output_closest_point
        )
{
    vector3 const u = line_end - line_begin;
    float_32_bit u_dot = dot_product(u,u);
    if (u_dot < 1e-6f)
        return 0.0f;
    float_32_bit t = dot_product(point - line_begin,u) / u_dot;
    t = t < 0.001f ? 0.0f :
        t > 0.999f ? 1.0f :
                     t    ;
    if (output_closest_point != nullptr)
        *output_closest_point = line_begin + t * u;
    return t;
}


void  closest_point_of_bbox_to_point(
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3 const&  point,
        vector3&  output_closest_point
        )
{
    output_closest_point(0) = std::min(std::max(bbox_low_corner(0),point(0)),bbox_high_corner(0));
    output_closest_point(1) = std::min(std::max(bbox_low_corner(1),point(1)),bbox_high_corner(1));
    output_closest_point(2) = std::min(std::max(bbox_low_corner(2),point(2)),bbox_high_corner(2));
}


bool  collision_point_and_bbox(
        vector3 const&  point,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner
        )
{
    return point(0) >= bbox_low_corner(0) && point(0) <= bbox_high_corner(0) &&
           point(1) >= bbox_low_corner(1) && point(1) <= bbox_high_corner(1) &&
           point(2) >= bbox_low_corner(2) && point(2) <= bbox_high_corner(2) ;
}


void  collision_point_and_plane(
        vector3 const&  point,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal,
        float_32_bit*  const  output_distance_to_plane,
        vector3* const  output_nearest_point_in_plane
        )
{
    float_32_bit const  parameter = dot_product(plane_unit_normal,point - plane_origin);
    if (output_distance_to_plane != nullptr)
        *output_distance_to_plane = parameter;
    if (output_nearest_point_in_plane != nullptr)
        *output_nearest_point_in_plane = point - parameter * plane_unit_normal;
}


bool  collision_ray_and_plane(
        vector3 const&  ray_origin,
        vector3 const&  ray_unit_direction,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal,
        float_32_bit*  const  output_cos_angle_between_plane_normal_and_ray_direction,
        float_32_bit*  const  output_distance_of_ray_origin_to_plane,
        float_32_bit*  const  output_distance_to_intersection_point,
        vector3* const  output_intersection_point
        )
{
    float_32_bit const  cos_angle = dot_product(plane_unit_normal,ray_unit_direction);
    if (output_cos_angle_between_plane_normal_and_ray_direction != nullptr)
        *output_cos_angle_between_plane_normal_and_ray_direction = cos_angle;

    float_32_bit const  normal_distance = dot_product(plane_unit_normal,ray_origin - plane_origin);
    if (output_distance_of_ray_origin_to_plane != nullptr)
        *output_distance_of_ray_origin_to_plane = normal_distance;

    if (absolute_value(cos_angle) < 1e-3f)
        return false;

    float_32_bit const  intersection_ditance = -normal_distance / cos_angle;
    if (output_distance_to_intersection_point != nullptr)
        *output_distance_to_intersection_point = intersection_ditance;
    if (output_intersection_point != nullptr)
        *output_intersection_point = ray_origin + intersection_ditance * ray_unit_direction;

    return true;
}


bool  clip_line_into_bbox(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        vector3* const  clipped_line_begin,
        vector3* const  clipped_line_end,
        float_32_bit* const  parameter_of_line_begin,
        float_32_bit* const  parameter_of_line_end
        )
{
    float_32_bit  tB = 0.0f;
    float_32_bit  tE = 1.0f;

    vector3 const  u = line_end - line_begin;

    for (auto  i = 0; i != 3; ++i)
    {
        float_32_bit const  B = line_begin(i) + tB * u(i);
        float_32_bit const  E = line_begin(i) + tE * u(i);

        if (u(i) < 0.0f)
        {
            if (B < bbox_low_corner(i) || E > bbox_high_corner(i))
                return false;

            if (B > bbox_high_corner(i))
            {
                if (u(i) > -1e-5f)
                    return false;

                tB = (bbox_high_corner(i) - line_begin(i)) / u(i);
            }

            if (E < bbox_low_corner(i))
            {
                if (u(i) > -1e-5f)
                    return false;

                tE = (bbox_low_corner(i) - line_begin(i)) / u(i);
            }
        }
        else
        {
            if (B > bbox_high_corner(i) || E < bbox_low_corner(i))
                return false;

            if (B < bbox_low_corner(i))
            {
                if (u(i) < 1e-5f)
                    return false;

                tB = (bbox_low_corner(i) - line_begin(i)) / u(i);
            }

            if (E > bbox_high_corner(i))
            {
                if (u(i) < 1e-5f)
                    return false;

                tE = (bbox_high_corner(i) - line_begin(i)) / u(i);
            }
        }
    }

    INVARIANT(tB >= 0.0f && tB <=1.0f && tE >= 0.0f && tE <=1.0f);

    if (tE < tB)
    {
        INVARIANT(tB >= tE + 1e-5f);
        tE = tB;
    }

    if (clipped_line_begin != nullptr)
        *clipped_line_begin = line_begin + tB * u;
    if (clipped_line_end != nullptr)
        *clipped_line_end = line_begin + tE * u;

    if (parameter_of_line_begin != nullptr)
        *parameter_of_line_begin = tB;
    if (parameter_of_line_end != nullptr)
        *parameter_of_line_end = tE;

    return true;
}


bool  collision_bbox_bbox(
        vector3 const&  bbox_0_low_corner,
        vector3 const&  bbox_0_high_corner,
        vector3 const&  bbox_1_low_corner,
        vector3 const&  bbox_1_high_corner,
        vector3&  intersection_bbox_low_corner,
        vector3&  intersection_bbox_high_corner
        )
{
    ASSUMPTION(&intersection_bbox_low_corner != &intersection_bbox_high_corner);
    for (int i = 0; i != 3; ++i)
    {
        intersection_bbox_low_corner(i) = std::max(bbox_0_low_corner(i), bbox_1_low_corner(i));
        intersection_bbox_high_corner(i) = std::min(bbox_0_high_corner(i), bbox_1_high_corner(i));
        if (intersection_bbox_low_corner(i) > intersection_bbox_high_corner(i))
            return false;
    }
    return true;
}


bool  collision_bbox_bbox(
        vector3 const&  bbox_0_low_corner,
        vector3 const&  bbox_0_high_corner,
        vector3 const&  bbox_1_low_corner,
        vector3 const&  bbox_1_high_corner
        )
{
    vector3  intersection_bbox_low_corner, intersection_bbox_high_corner;
    return collision_bbox_bbox(
                bbox_0_low_corner,
                bbox_0_high_corner,
                bbox_1_low_corner,
                bbox_1_high_corner,
                intersection_bbox_low_corner,
                intersection_bbox_high_corner
                );
}


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,
        vector2 const&  clip_origin,
        vector2 const&  clip_normal,
        clipped_polygon_description* const  description
        )
{
    INVARIANT(polygon_points.size() >= 4UL);    // The first point and the last one are the same.
                                                // So, at least 3 different points are required.

    // clip_line_normal * (X - clip_line_origin) = 0
    // X = A + t*(B-A)  ; A = polygon_points.at(i-1), B = polygon_points.at(i)
    // --------------------
    // clip_line_normal * (A + t*(B-A) - clip_line_origin) = 0
    // t * clip_line_normal * (B-A) + clip_line_normal * (A - clip_line_origin) = 0
    // t * (dot(normal,B) - dot(normal,A) + (dot(normal,A) - dot(normal,origin)) = 0

    scalar const  dot_normal_origin = dot_product_2d(clip_normal, clip_origin);
    scalar  dot_normal_A = dot_product_2d(clip_normal, polygon_points.at(0UL));
    scalar  dot_normal_old_A = dot_normal_origin;
    scalar  dot_normal_B;
    std::size_t  i = 1UL;

    if (dot_normal_A >= dot_normal_origin)
    {
        // The first point of the polygon is in FRONT space (i.e. not clipped away).

        while (true)
        {
            dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));

            if (dot_normal_B < dot_normal_origin)
                break;
            if (dot_normal_B >= dot_normal_A && dot_normal_old_A >= dot_normal_A)
                return POINT_SET_TYPE::FULL;

            ++i;
            if (i == polygon_points.size())
                return POINT_SET_TYPE::FULL;

            dot_normal_old_A = dot_normal_A;
            dot_normal_A = dot_normal_B;
        }
        if (description != nullptr)
        {
            description->index_end = i - 1UL;

            // param = (dot_normal_A - dot_normal_origin) / (dot_normal_A - dot_normal_B)

            scalar const  normal_distance_between_A_and_B = dot_normal_A - dot_normal_B;
            if (normal_distance_between_A_and_B < (scalar)1e-4) // i.e. < 0.1mm
            {
                description->param_start = (scalar)1.0;
                description->point_start = polygon_points.at(i);
            }
            else
            {
                scalar const  normal_distance_between_A_and_line_origin =  dot_normal_A - dot_normal_origin;
                description->param_start =
                        normal_distance_between_A_and_line_origin / normal_distance_between_A_and_B;
                description->point_start =
                        polygon_points.at(description->index_end) + description->param_start *
                                ( polygon_points.at(i) - polygon_points.at(description->index_end) );
            }
        }
        for(++i; true; ++i)
        {
            dot_normal_A = dot_normal_B;
            scalar const  dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));
            if (dot_normal_B >= dot_normal_origin)
                break;
        }
        if (description != nullptr)
        {
            description->index_start = i;

            // param = (dot_normal_origin - dot_normal_A) / (dot_normal_B - dot_normal_A)

            scalar const  normal_distance_between_A_and_B = dot_normal_B - dot_normal_A;
            if (normal_distance_between_A_and_B < (scalar)1e-4) // i.e. < 0.1mm
            {
                description->param_end = (scalar)0.0;
                description->point_end = polygon_points.at(i - 1UL);
            }
            else
            {
                scalar const  normal_distance_between_A_and_line_origin = dot_normal_origin - dot_normal_A;
                description->param_end =
                        normal_distance_between_A_and_line_origin / normal_distance_between_A_and_B;
                description->point_end =
                        polygon_points.at(i - 1UL) + description->param_end *
                                ( polygon_points.at(description->index_start) - polygon_points.at(i - 1UL) );
            }
        }

        return POINT_SET_TYPE::GENERAL;
    }
    else
    {
        // The first point of the polygon is in BACK space (i.e. clipped away).

        while (true)
        {
            dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));

            if (dot_normal_B >= dot_normal_origin)
                break;
            if (dot_normal_B <= dot_normal_A && dot_normal_old_A <= dot_normal_A)
                return POINT_SET_TYPE::EMPTY;

            ++i;
            if (i == polygon_points.size())
                return POINT_SET_TYPE::EMPTY;

            dot_normal_old_A = dot_normal_A;
            dot_normal_A = dot_normal_B;
        }
        if (description != nullptr)
        {
            description->index_start = i;

            // param = (dot_normal_origin - dot_normal_A) / (dot_normal_B - dot_normal_A)

            scalar const  normal_distance_between_A_and_B = dot_normal_B - dot_normal_A;
            if (normal_distance_between_A_and_B < (scalar)1e-4) // i.e. < 0.1mm
            {
                description->param_end = (scalar)0.0;
                description->point_end = polygon_points.at(i - 1UL);
            }
            else
            {
                scalar const  normal_distance_between_A_and_line_origin = dot_normal_origin - dot_normal_A;
                description->param_end =
                        normal_distance_between_A_and_line_origin / normal_distance_between_A_and_B;
                description->point_end =
                        polygon_points.at(i - 1UL) + description->param_end *
                                ( polygon_points.at(description->index_start) - polygon_points.at(i - 1UL) );
            }
        }
        for(++i; true; ++i)
        {
            dot_normal_A = dot_normal_B;
            scalar const  dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));
            if (dot_normal_B < dot_normal_origin)
                break;
        }
        if (description != nullptr)
        {
            description->index_end = i - 1UL;

            // param = (dot_normal_A - dot_normal_origin) / (dot_normal_A - dot_normal_B)

            scalar const  normal_distance_between_A_and_B = dot_normal_A - dot_normal_B;
            if (normal_distance_between_A_and_B < (scalar)1e-4) // i.e. < 0.1mm
            {
                description->param_start = (scalar)1.0;
                description->point_start = polygon_points.at(i);
            }
            else
            {
                scalar const  normal_distance_between_A_and_line_origin =  dot_normal_A - dot_normal_origin;
                description->param_start =
                        normal_distance_between_A_and_line_origin / normal_distance_between_A_and_B;
                description->point_start =
                        polygon_points.at(description->index_end) + description->param_start *
                                ( polygon_points.at(i) - polygon_points.at(description->index_end) );
            }
        }

        return POINT_SET_TYPE::GENERAL;
    }
}


POINT_SET_TYPE  instersection_of_plane_with_xy_coord_plane(
        vector3 const&  origin,
        vector3 const&  normal,
        vector2&  intersection_origin,
        vector2&  intersection_normal
        )
{
    if (normal(2) > scalar(1.0 - 1e-4))
    {
        if (origin(2) <= scalar(0.0))
            return POINT_SET_TYPE::FULL;
        else
            return POINT_SET_TYPE::EMPTY;
    }
    if (normal(2) < scalar(-1.0 + 1e-4))
    {
        if (origin(2) >= scalar(0.0))
            return POINT_SET_TYPE::FULL;
        else
            return POINT_SET_TYPE::EMPTY;
    }

    vector3 const  intersection_line_vector = cross_product(normal, vector3_unit_z()); // This could be optimised, in case of performance issues.
    vector3 const  down_hill_line_vector = cross_product(normal, intersection_line_vector); // This could be optimised, in case of performance issues.

    scalar const  down_hill_param = -origin(3) / down_hill_line_vector(3);

    intersection_origin = contract32(origin) + down_hill_param * contract32(down_hill_line_vector);
    intersection_normal = normalised_2d(orthogonal(contract32(intersection_line_vector)));

    return POINT_SET_TYPE::GENERAL;
}


POINT_SET_TYPE  clip_polygon(
        matrix44 const&  to_polygon_space_matrix,
        std::vector<vector2> const&  polygon_points,
        vector3 const&  clip_origin,
        vector3 const&  clip_normal,
        clipped_polygon_description* const  description
        )
{
    vector3 const  transformed_clip_origin = transform_point(clip_origin, to_polygon_space_matrix);
    vector3 const  transformed_clip_normal = transform_vector(clip_normal, to_polygon_space_matrix);

    vector2 origin2d;
    vector2 normal2d;
    POINT_SET_TYPE const  plane_intersection_type =
        instersection_of_plane_with_xy_coord_plane(
                transformed_clip_origin,
                transformed_clip_normal,
                origin2d,
                normal2d
                );

    return plane_intersection_type != POINT_SET_TYPE::GENERAL ?
                plane_intersection_type :
                clip_polygon(polygon_points,origin2d,normal2d,description);
}


bool  intersection_of_covex_polytopes(
    )
{
    return false;
}


}
