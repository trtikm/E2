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

    float_32_bit const  intersection_ditance = normal_distance / cos_angle;
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
    for (int i = 0; i != 3; ++i)
    {
        intersection_bbox_low_corner(i) = std::max(bbox_0_low_corner(i), bbox_1_low_corner(i));
        intersection_bbox_high_corner(i) = std::min(bbox_0_high_corner(i), bbox_1_high_corner(i));
        if (intersection_bbox_low_corner(i) >= intersection_bbox_high_corner(i))
            return false;
    }
    return true;
}


}
