#include <angeo/collide.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <array>
#include <tuple>

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


natural_32_bit  closest_points_of_two_lines(
        vector3 const&  line_1_begin,
        vector3 const&  line_1_end,
        vector3 const&  line_2_begin,
        vector3 const&  line_2_end,
        vector3*  output_line_1_closest_point_1,
        float_32_bit*  output_line_1_closest_point_param_1,
        vector3*  output_line_2_closest_point_1,
        float_32_bit*  output_line_2_closest_point_param_1,
        vector3*  output_line_1_closest_point_2,
        float_32_bit*  output_line_1_closest_point_param_2,
        vector3*  output_line_2_closest_point_2,
        float_32_bit*  output_line_2_closest_point_param_2
        )
{
    vector3 const  u1 = line_1_end - line_1_begin;

    float_32_bit const  u1_dot_u1 = dot_product(u1, u1);

    vector3 const  v1 = line_2_begin - line_1_begin;
    vector3 const  v2 = line_2_end - line_1_begin;

    float_32_bit const  v1_dot_u1 = dot_product(v1, u1);
    float_32_bit const  v2_dot_u1 = dot_product(v2, u1);

    if (v1_dot_u1 < 0.0f && v2_dot_u1 < 0.0f)
    {
        if (output_line_1_closest_point_1 != nullptr)
            *output_line_1_closest_point_1 = line_1_begin;
        if (output_line_1_closest_point_param_1 != nullptr)
            *output_line_1_closest_point_param_1 = 0.0f;
        float_32_bit const t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);
        if (output_line_2_closest_point_param_1 != nullptr)
            *output_line_2_closest_point_param_1 = t;
        return 1U;
    }

    if (v1_dot_u1 > u1_dot_u1 && v2_dot_u1 > u1_dot_u1)
    {
        if (output_line_1_closest_point_1 != nullptr)
            *output_line_1_closest_point_1 = line_1_end;
        if (output_line_1_closest_point_param_1 != nullptr)
            *output_line_1_closest_point_param_1 = 1.0f;
        float_32_bit const t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_1);
        if (output_line_2_closest_point_param_1 != nullptr)
            *output_line_2_closest_point_param_1 = t;
        return 1U;
    }

    vector3 const  u2 = line_2_end - line_2_begin;

    vector3 const  n = cross_product(u1, u2);

    //float_32_bit constexpr  sin_1_degree_squared = 3.0458649e-4f;
    //float_32_bit const  sin_of_angle_between_u1_and_u2_squared = dot_product(n, n) / (u1_dot_u1 * dot_product(u2, u2));
    //if (sin_of_angle_between_u1_and_u2_squared < sin_1_degree_squared)
    if (dot_product(n, n) < 0.0001f)
    {
        // Lines are (almost) parallel.

        float_32_bit const  u1u2_dot = dot_product(u1, u2);

        // Solve for k2_begin:  u2 * (line_1_begin + k2_begin * u1 - line_2_begin) = 0
        float_32_bit const  k2_begin = dot_product(u2, v1) / u1u2_dot;

        // Solve for k2_end:  u2 * (line_1_begin + k2_end * u1 - line_2_end) = 0
        float_32_bit const  k2_end = -dot_product(u2, line_1_begin - line_2_end) / u1u2_dot;

        if (k2_begin <= k2_end)
        {
            if (k2_end <= 0.0f)
            {
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 0.0f;
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_begin;

                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 1.0f;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_end;

                return 1U;  // There is only 1 pair of closest points (1 point on each line)
            }

            if (k2_begin >= 1.0f)
            {
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 1.0f;
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_end;

                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 0.0f;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_begin;

                return 1U;  // There is only 1 pair of closest points (1 point on each line)
            }

            float_32_bit  dummy;

            if (k2_begin <= 0.0f)
                if (k2_end <= 1.0f)
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = 0.0f;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin;

                    *((output_line_2_closest_point_param_1 != nullptr) ? output_line_2_closest_point_param_1 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = k2_end;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_begin + k2_end * u1;

                    if (output_line_2_closest_point_param_2 != nullptr)
                        *output_line_2_closest_point_param_2 = 1.0f;
                    if (output_line_2_closest_point_2 != nullptr)
                        *output_line_2_closest_point_2 = line_2_end;
                }
                else
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = 0.0f;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin;

                    *((output_line_2_closest_point_param_1 != nullptr) ? output_line_2_closest_point_param_1 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = 1.0f;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_end;

                    *((output_line_2_closest_point_param_2 != nullptr) ? output_line_2_closest_point_param_2 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_2);

                }
            else // k2_begin < 1.0f
                if (k2_end <= 1.0f)
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = k2_begin;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin + k2_begin * u1;

                    if (output_line_2_closest_point_param_1 != nullptr)
                        *output_line_2_closest_point_param_1 = 0.0f;
                    if (output_line_2_closest_point_1 != nullptr)
                        *output_line_2_closest_point_1 = line_2_begin;

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = k2_end;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_begin + k2_end * u1;

                    if (output_line_2_closest_point_param_2 != nullptr)
                        *output_line_2_closest_point_param_2 = 1.0f;
                    if (output_line_2_closest_point_2 != nullptr)
                        *output_line_2_closest_point_2 = line_2_end;
                }
                else
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = k2_begin;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin + k2_begin * u1;

                    if (output_line_2_closest_point_param_1 != nullptr)
                        *output_line_2_closest_point_param_1 = 0.0f;
                    if (output_line_2_closest_point_1 != nullptr)
                        *output_line_2_closest_point_1 = line_2_begin;

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = 1.0f;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_end;

                    *((output_line_2_closest_point_param_2 != nullptr) ? output_line_2_closest_point_param_2 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_2);
                }
        }
        else
        {
            if (k2_begin <= 0.0f)
            {
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 0.0f;
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_begin;

                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 0.0f;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_begin;

                return 1U;  // There is only 1 pair of closest points (1 point on each line)
            }

            if (k2_end >= 1.0f)
            {
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 1.0f;
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_end;

                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 1.0f;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_end;

                return 1U;  // There is only 1 pair of closest points (1 point on each line)
            }

            float_32_bit  dummy;

            if (k2_end <= 0.0f)
                if (k2_begin <= 1.0f)
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = 0.0f;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin;

                    *((output_line_2_closest_point_param_1 != nullptr) ? output_line_2_closest_point_param_1 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = k2_begin;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_begin + k2_begin * u1;

                    if (output_line_2_closest_point_param_2 != nullptr)
                        *output_line_2_closest_point_param_2 = 0.0f;
                    if (output_line_2_closest_point_2 != nullptr)
                        *output_line_2_closest_point_2 = line_2_begin;
                }
                else
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = 0.0f;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin;

                    *((output_line_2_closest_point_param_1 != nullptr) ? output_line_2_closest_point_param_1 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = 1.0f;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_end;

                    *((output_line_2_closest_point_param_2 != nullptr) ? output_line_2_closest_point_param_2 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_2);

                }
            else // k2_end < 1.0f
                if (k2_begin <= 1.0f)
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = k2_end;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin + k2_end * u1;

                    if (output_line_2_closest_point_param_1 != nullptr)
                        *output_line_2_closest_point_param_1 = 1.0f;
                    if (output_line_2_closest_point_1 != nullptr)
                        *output_line_2_closest_point_1 = line_2_end;

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = k2_begin;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_begin + k2_begin * u1;

                    if (output_line_2_closest_point_param_2 != nullptr)
                        *output_line_2_closest_point_param_2 = 0.0f;
                    if (output_line_2_closest_point_2 != nullptr)
                        *output_line_2_closest_point_2 = line_2_begin;
                }
                else
                {
                    // Pair 1

                    if (output_line_1_closest_point_param_1 != nullptr)
                        *output_line_1_closest_point_param_1 = k2_end;
                    if (output_line_1_closest_point_1 != nullptr)
                        *output_line_1_closest_point_1 = line_1_begin + k2_end * u1;

                    if (output_line_2_closest_point_param_1 != nullptr)
                        *output_line_2_closest_point_param_1 = 1.0f;
                    if (output_line_2_closest_point_1 != nullptr)
                        *output_line_2_closest_point_1 = line_2_end;

                    // Pair 2

                    if (output_line_1_closest_point_param_2 != nullptr)
                        *output_line_1_closest_point_param_2 = 1.0f;
                    if (output_line_1_closest_point_2 != nullptr)
                        *output_line_1_closest_point_2 = line_1_end;

                    *((output_line_2_closest_point_param_2 != nullptr) ? output_line_2_closest_point_param_2 : &dummy) =
                        closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_2);
                }
        }

        return 2U;  // There are infinitelly many pairs of closest points. We return only 2 pairs: most distant ones.
    }

    // General case: lines are not parallel.

    vector3 const  m = cross_product(n, u1);

    float_32_bit const  m_dot_v1 = dot_product(m, v1);
    float_32_bit const  m_dot_v2 = dot_product(m, v2);

    if (m_dot_v1 * m_dot_v2 >= 0.0f)
    {
        // Both end points 'line_2_begin' and 'line_2_end' are on one side of the other line (either above or bellow).

        if (std::fabsf(m_dot_v1) < std::fabsf(m_dot_v2))
        {
            if (v1_dot_u1 < 0.0f)
            {
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_begin;
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 0.0f;
                float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = t;
            }
            else if (v1_dot_u1 > u1_dot_u1)
            {
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_end;
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 1.0f;
                float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_1);
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = t;
            }
            else
            {
                float_32_bit const  t = closest_point_on_line_to_point(line_1_begin, line_1_end, line_2_begin, output_line_1_closest_point_1);
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = t;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_begin;
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 0.0f;
            }
        }
        else
        {
            if (v2_dot_u1 < 0.0f)
            {
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_begin;
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 0.0f;
                float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = t;
            }
            else if (v2_dot_u1 > u1_dot_u1)
            {
                if (output_line_1_closest_point_1 != nullptr)
                    *output_line_1_closest_point_1 = line_1_end;
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = 1.0f;
                float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_1);
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = t;
            }
            else
            {
                float_32_bit const  t = closest_point_on_line_to_point(line_1_begin, line_1_end, line_2_end, output_line_1_closest_point_1);
                if (output_line_1_closest_point_param_1 != nullptr)
                    *output_line_1_closest_point_param_1 = t;
                if (output_line_2_closest_point_1 != nullptr)
                    *output_line_2_closest_point_1 = line_2_end;
                if (output_line_2_closest_point_param_1 != nullptr)
                    *output_line_2_closest_point_param_1 = 1.0f;
            }
        }

        return 1U;
    }

    // The end points 'line_2_begin' and 'line_2_end' are on oposite sides of the other line (one above the other bellow).

    // Solve for 'k':  m * (line_2_begin + k * u2 - line_1_begin) = 0
    float_32_bit const  k = -m_dot_v1 / dot_product(m, u2);

    vector3 const  C2 = line_2_begin + k * u2;

    vector3 const  w = C2 - line_1_begin;
    float_32_bit const  w_dot_u1 = dot_product(w, u1);

    if (w_dot_u1 < 0.0f)
    {
        if (output_line_1_closest_point_1 != nullptr)
            *output_line_1_closest_point_1 = line_1_begin;
        if (output_line_1_closest_point_param_1 != nullptr)
            *output_line_1_closest_point_param_1 = 0.0f;
        float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_begin, output_line_2_closest_point_1);
        if (output_line_2_closest_point_param_1 != nullptr)
            *output_line_2_closest_point_param_1 = t;
    }
    if (w_dot_u1 > u1_dot_u1)
    {
        if (output_line_1_closest_point_1 != nullptr)
            *output_line_1_closest_point_1 = line_1_end;
        if (output_line_1_closest_point_param_1 != nullptr)
            *output_line_1_closest_point_param_1 = 1.0f;
        float_32_bit const  t = closest_point_on_line_to_point(line_2_begin, line_2_end, line_1_end, output_line_2_closest_point_1);
        if (output_line_2_closest_point_param_1 != nullptr)
            *output_line_2_closest_point_param_1 = t;
    }
    else
    {
        // Solve for 'h':  w = h*u1 => w*u1 = h*(u1*u1)
        float_32_bit const  h = w_dot_u1 / u1_dot_u1;
        if (output_line_1_closest_point_1 != nullptr)
            *output_line_1_closest_point_1 = line_1_begin + h * u1;
        if (output_line_1_closest_point_param_1 != nullptr)
            *output_line_1_closest_point_param_1 = h;

        if (output_line_2_closest_point_1 != nullptr)
            *output_line_2_closest_point_1 = C2;
        if (output_line_2_closest_point_param_1 != nullptr)
            *output_line_2_closest_point_param_1 = k;
    }

    return 1U;
}


collision_shape_feature_id  closest_point_on_cylinder_to_point(
        vector3 const&  cylinder_axis_line_begin,
        vector3 const&  cylinder_axis_line_end,
        float_32_bit const  cylinder_radius,
        vector3 const&  point,
        vector3&  output_closest_point
        )
{
    vector3 const  n = cylinder_axis_line_end - cylinder_axis_line_begin;
    float_32_bit const  dot_nn = dot_product(n, n);

    vector3 const  u = point - cylinder_axis_line_begin;
    float_32_bit const  dot_nu = dot_product(n, u);

    vector3 const  v = (dot_nu / dot_nn) * n;
    vector3 const  w = u - v;
    float_32_bit const  dot_ww = dot_product(w, w);
    bool const  is_within_radius = dot_ww <= cylinder_radius * cylinder_radius;

    if (dot_nu <= 0.0f)
    {
        if (is_within_radius)
        {
            output_closest_point = cylinder_axis_line_begin + w;
            return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);
        }
        output_closest_point = cylinder_axis_line_begin + (cylinder_radius / std::sqrtf(dot_ww)) * w;
        return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, 0U);
    }

    if (dot_nu >= dot_nn)
    {
        if (is_within_radius)
        {
            output_closest_point = cylinder_axis_line_begin + w;
            return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 1U);
        }
        output_closest_point = cylinder_axis_line_begin + (cylinder_radius / std::sqrtf(dot_ww)) * w;
        return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, 1U);
    }

    if (is_within_radius)
        output_closest_point = point;
    else
        output_closest_point = cylinder_axis_line_begin + v + (cylinder_radius / std::sqrtf(dot_ww)) * w;
    return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 2U);
}


bool  closest_point_of_triangle_to_point(
        vector3 const&  triangle_vertex_1,
        vector3 const&  triangle_vertex_2,
        vector3 const&  triangle_vertex_3,
        vector3 const&  unit_normal_vector,
        bool const  cull_triangle_back_side,
        vector3 const&  point,
        vector3*  output_triangle_closest_point_ptr,
        collision_shape_feature_id*  output_triangle_shape_feature_id_ptr,
        float_32_bit*  output_parameter_ptr
        )
{
    vector3  w = point - triangle_vertex_1;
    float_32_bit const  dot_normal_w = dot_product(unit_normal_vector, w);

    if (cull_triangle_back_side == true && dot_normal_w < 0.0f)
        return false;

    vector3  closest_point;

    auto const  edge_collider =
            [output_triangle_closest_point_ptr,
             output_triangle_shape_feature_id_ptr,
             output_parameter_ptr,
             &unit_normal_vector](
                    vector3 const&  A,
                    vector3 const&  B,
                    natural_32_bit const  feature_index,
                    vector3 const&  P
                    )
            {
                if (dot_product(cross_product(B - A, unit_normal_vector), P - A) < 0.0f)
                    return false;
                float_32_bit const  line_param = closest_point_on_line_to_point(A, B, P, output_triangle_closest_point_ptr);
                if (output_triangle_shape_feature_id_ptr != nullptr)
                {
                    if (line_param < 0.001f)
                        *output_triangle_shape_feature_id_ptr = make_collision_shape_feature_id(
                                COLLISION_SHAPE_FEATURE_TYPE::VERTEX,
                                feature_index
                                );
                    else if (line_param > 0.999f)
                        *output_triangle_shape_feature_id_ptr = make_collision_shape_feature_id(
                                COLLISION_SHAPE_FEATURE_TYPE::VERTEX,
                                feature_index == 2U ? 0U : feature_index + 1U
                                );
                    else
                        *output_triangle_shape_feature_id_ptr = make_collision_shape_feature_id(
                                COLLISION_SHAPE_FEATURE_TYPE::EDGE,
                                feature_index
                                );
                }
                if (output_parameter_ptr != nullptr)
                    *output_parameter_ptr = line_param;
                return true;
            };

    if (edge_collider(triangle_vertex_1, triangle_vertex_2, 0U, point) == true)
        return true;
    if (edge_collider(triangle_vertex_2, triangle_vertex_3, 1U, point) == true)
        return true;
    if (edge_collider(triangle_vertex_3, triangle_vertex_1, 2U, point) == true)
        return true;

    collision_point_and_plane(
            point,
            triangle_vertex_1,
            unit_normal_vector,
            output_parameter_ptr,
            output_triangle_closest_point_ptr
            );
    if (output_triangle_shape_feature_id_ptr != nullptr)
        *output_triangle_shape_feature_id_ptr = make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);

    return true;
}


/**
 * Ensure that 'triangle_unit_normal_vector' agrees with the counter-clock-wise order of triangle's verices.
 *   ASSUMPTION(dot_product(normal_T12, triangle_vertex_3 - triangle_vertex_1) < 0.0f);
 */
natural_32_bit  closest_points_of_triangle_and_line(
    vector3 const&  triangle_vertex_1,
    vector3 const&  triangle_vertex_2,
    vector3 const&  triangle_vertex_3,
    vector3 const&  triangle_unit_normal_vector,
    vector3  line_point_1,
    vector3  line_point_2,

    vector3*  output_triangle_closest_point_1,
    collision_shape_feature_id*  output_triangle_shape_feature_id_1,
    vector3*  output_line_closest_point_1,
    collision_shape_feature_id*  output_line_shape_feature_id_1,

    vector3*  output_triangle_closest_point_2,
    collision_shape_feature_id*  output_triangle_shape_feature_id_2,
    vector3*  output_line_closest_point_2,
    collision_shape_feature_id*  output_line_shape_feature_id_2,

    natural_8_bit const  triangle_edges_ignore_mask
    )
{
    float_32_bit constexpr  distance_epsilon = 0.0001f;
    float_32_bit constexpr  param_epsilon = 0.001f;

    scalar  dot_Tnormal_T1L1 = dot_product(triangle_unit_normal_vector, line_point_1 - triangle_vertex_1);
    scalar  dot_Tnormal_T1L2 = dot_product(triangle_unit_normal_vector, line_point_2 - triangle_vertex_1);

    bool  is_dot_Tnormal_T1L1_outdated = false;
    bool  is_dot_Tnormal_T1L2_outdated = false;
    scalar  min_line_param = 0.0f;
    scalar  max_line_param = 1.0f;
    if (dot_Tnormal_T1L1 < distance_epsilon)
    {
        if (dot_Tnormal_T1L2 < distance_epsilon)
            return 0U;

        scalar const  L1_distance_to_triangle_plane = std::fabsf(dot_Tnormal_T1L1);
        scalar const  param = L1_distance_to_triangle_plane / (L1_distance_to_triangle_plane + dot_Tnormal_T1L2);
        line_point_1 += param * (line_point_2 - line_point_1);
        is_dot_Tnormal_T1L1_outdated = true;
    }
    else if (dot_Tnormal_T1L2 < distance_epsilon)
    {
        scalar const  param = dot_Tnormal_T1L1 / (dot_Tnormal_T1L1 + std::fabsf(dot_Tnormal_T1L2));
        line_point_2 = line_point_1 + param * (line_point_2 - line_point_1);
        is_dot_Tnormal_T1L2_outdated = true;
    }

    auto const  build_triangle_edge_feature_id =
        [param_epsilon](float_32_bit const  param, natural_32_bit const  index_1, natural_32_bit const  index_2) -> collision_shape_feature_id {
            if (param < param_epsilon)
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, index_1);
            if (param > 1.0f - param_epsilon)
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, index_2);
            else
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, index_1);
        };
    auto const  build_line_feature_id =
        [param_epsilon](float_32_bit const  param) -> collision_shape_feature_id {
            if (param < param_epsilon)
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U);
            if (param > 1.0f - param_epsilon)
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 1U);
            else
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, 0U);
        };

    auto const  build_triangle_line_and_line_both_feature_ids =
        [output_triangle_shape_feature_id_1, output_triangle_shape_feature_id_2,
         output_line_shape_feature_id_1, output_line_shape_feature_id_2,
         &build_triangle_edge_feature_id, &build_line_feature_id](
                natural_32_bit const  num_collisions,
                float_32_bit const  triangle_param_1, float_32_bit const  triangle_param_2,
                natural_32_bit const  index_1, natural_32_bit const  index_2,
                float_32_bit const  line_param_1, float_32_bit const  line_param_2) -> void {
            if (output_triangle_shape_feature_id_1 != nullptr)
                *output_triangle_shape_feature_id_1 = build_triangle_edge_feature_id(triangle_param_1, 0U, 1U);
            if (output_line_shape_feature_id_1 != nullptr)
                *output_line_shape_feature_id_1 = build_line_feature_id(line_param_1);
            if (num_collisions == 2U)
            {
                if (output_triangle_shape_feature_id_2 != nullptr)
                    *output_triangle_shape_feature_id_2 = build_triangle_edge_feature_id(triangle_param_2, 0U, 1U);
                if (output_line_shape_feature_id_2 != nullptr)
                    *output_line_shape_feature_id_2 = build_line_feature_id(line_param_2);
            }
        };

    vector3 const normal_T12 = cross_product(triangle_vertex_2 - triangle_vertex_1, triangle_unit_normal_vector);

    // Check that 'triangle_unit_normal_vector' agrees with the counter-clock-wise order of triangle's verices.
    ASSUMPTION(dot_product(normal_T12, triangle_vertex_3 - triangle_vertex_1) < 0.0f);

    bool const  L1_behind_T12 = dot_product(normal_T12, line_point_1 - triangle_vertex_1) >= 0.0f;
    bool const  L2_behind_T12 = dot_product(normal_T12, line_point_2 - triangle_vertex_1) >= 0.0f;

    if (L1_behind_T12 && L2_behind_T12)
    {
        if ((triangle_edges_ignore_mask & 1U) != 0U)
            return 0U;

        float_32_bit  triangle_param_1, triangle_param_2, line_param_1, line_param_2;
        natural_32_bit const  num_collisions = closest_points_of_two_lines(
                        triangle_vertex_1,
                        triangle_vertex_2,
                        line_point_1,
                        line_point_2,
                        output_triangle_closest_point_1,
                        &triangle_param_1,
                        output_line_closest_point_1,
                        &line_param_1,
                        output_triangle_closest_point_2,
                        &triangle_param_2,
                        output_line_closest_point_2,
                        &line_param_2
                        );
        build_triangle_line_and_line_both_feature_ids(
                num_collisions,
                triangle_param_1,
                triangle_param_2,
                0U,
                1U,
                line_param_1,
                line_param_2
                );
        return num_collisions;
    }

    vector3 const normal_T23 = cross_product(triangle_vertex_3 - triangle_vertex_2, triangle_unit_normal_vector);

    bool const  L1_behind_T23 = dot_product(normal_T23, line_point_1 - triangle_vertex_2) >= 0.0f;
    bool const  L2_behind_T23 = dot_product(normal_T23, line_point_2 - triangle_vertex_2) >= 0.0f;

    if (L1_behind_T23 && L2_behind_T23)
    {
        if ((triangle_edges_ignore_mask & 2U) != 0U)
            return 0U;

        float_32_bit  triangle_param_1, triangle_param_2, line_param_1, line_param_2;
        natural_32_bit const  num_collisions = closest_points_of_two_lines(
                        triangle_vertex_2,
                        triangle_vertex_3,
                        line_point_1,
                        line_point_2,
                        output_triangle_closest_point_1,
                        &triangle_param_1,
                        output_line_closest_point_1,
                        &line_param_1,
                        output_triangle_closest_point_2,
                        &triangle_param_2,
                        output_line_closest_point_2,
                        &line_param_2
                        );
        build_triangle_line_and_line_both_feature_ids(
                num_collisions,
                triangle_param_1,
                triangle_param_2,
                1U,
                2U,
                line_param_1,
                line_param_2
                );
        return num_collisions;
    }

    vector3 const normal_T31 = cross_product(triangle_vertex_1 - triangle_vertex_3, triangle_unit_normal_vector);

    bool const  L1_behind_T31 = dot_product(normal_T31, line_point_1 - triangle_vertex_3) >= 0.0f;
    bool const  L2_behind_T31 = dot_product(normal_T31, line_point_2 - triangle_vertex_3) >= 0.0f;

    if (L1_behind_T31 && L2_behind_T31)
    {
        if ((triangle_edges_ignore_mask & 4U) != 0U)
            return 0U;

        float_32_bit  triangle_param_1, triangle_param_2, line_param_1, line_param_2;
        natural_32_bit const  num_collisions = closest_points_of_two_lines(
                        triangle_vertex_3,
                        triangle_vertex_1,
                        line_point_1,
                        line_point_2,
                        output_triangle_closest_point_1,
                        &triangle_param_1,
                        output_line_closest_point_1,
                        &line_param_1,
                        output_triangle_closest_point_2,
                        &triangle_param_2,
                        output_line_closest_point_2,
                        &line_param_2
                        );
        build_triangle_line_and_line_both_feature_ids(
                num_collisions,
                triangle_param_1,
                triangle_param_2,
                2U,
                0U,
                line_param_1,
                line_param_2
                );
        return num_collisions;
    }

    if (is_dot_Tnormal_T1L1_outdated)
        dot_Tnormal_T1L1 = dot_product(triangle_unit_normal_vector, line_point_1 - triangle_vertex_1);
    if (is_dot_Tnormal_T1L2_outdated)
        dot_Tnormal_T1L2 = dot_product(triangle_unit_normal_vector, line_point_2 - triangle_vertex_1);

    bool const  L1_inside_triagle = !L1_behind_T12 && !L1_behind_T23 && !L1_behind_T31;
    bool const  L2_inside_triagle = !L2_behind_T12 && !L2_behind_T23 && !L2_behind_T31;
    bool const  L1_below_L2 = dot_Tnormal_T1L1 < dot_Tnormal_T1L2 - 10.0f * distance_epsilon;
    bool const  L2_below_L1 = dot_Tnormal_T1L2 < dot_Tnormal_T1L1 - 10.0f * distance_epsilon;

    auto const  write_output_for_one_line_corner_point =
        [&triangle_unit_normal_vector](
                vector3 const  line_point,
                float_32_bit const  distance_to_plane,
                natural_32_bit const  point_index,
                vector3*  output_triangle_closest_point,
                collision_shape_feature_id*  output_triangle_shape_feature_id,
                vector3*  output_line_closest_point,
                collision_shape_feature_id*  output_line_shape_feature_id
                ) -> natural_32_bit {
            if (output_triangle_closest_point != nullptr)
                *output_triangle_closest_point = line_point - distance_to_plane * triangle_unit_normal_vector;
            if (output_triangle_shape_feature_id != nullptr)
                *output_triangle_shape_feature_id = make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);

            if (output_line_closest_point != nullptr)
                *output_line_closest_point = line_point;
            if (output_line_shape_feature_id != nullptr)
                *output_line_shape_feature_id = make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, point_index);

            return 1U;
        };

    if (L1_inside_triagle && L1_below_L2)
        return write_output_for_one_line_corner_point(
                    line_point_1,
                    dot_Tnormal_T1L1,
                    0U,
                    output_triangle_closest_point_1,
                    output_triangle_shape_feature_id_1,
                    output_line_closest_point_1,
                    output_line_shape_feature_id_1
                    );
    if (L2_inside_triagle && L2_below_L1)
        return write_output_for_one_line_corner_point(
                    line_point_2,
                    dot_Tnormal_T1L2,
                    1U,
                    output_triangle_closest_point_1,
                    output_triangle_shape_feature_id_1,
                    output_line_closest_point_1,
                    output_line_shape_feature_id_1
                    );
    if (L1_inside_triagle && L2_inside_triagle)
        return write_output_for_one_line_corner_point(
                    line_point_1,
                    dot_Tnormal_T1L1,
                    0U,
                    output_triangle_closest_point_1,
                    output_triangle_shape_feature_id_1,
                    output_line_closest_point_1,
                    output_line_shape_feature_id_1
                    ) +
                write_output_for_one_line_corner_point(
                    line_point_2,
                    dot_Tnormal_T1L2,
                    1U,
                    output_triangle_closest_point_2,
                    output_triangle_shape_feature_id_2,
                    output_line_closest_point_2,
                    output_line_shape_feature_id_2
                    );

    vector3  triangle_closest_points[3];
    float_32_bit  triangle_closest_point_params[3];
    collision_shape_feature_id  triangle_closest_point_feature_ids[3];

    vector3  line_closest_points[3];
    float_32_bit  line_closest_point_params[3];
    collision_shape_feature_id  line_closest_point_feature_ids[3];

    natural_32_bit  num_closest_point_pairs = 0U;

    bool const  is_line_parallel_with_plane_of_triangle = !L1_below_L2 && !L2_below_L1;

    if (is_line_parallel_with_plane_of_triangle)
    {
        if (L1_inside_triagle)
        {
            triangle_closest_points[num_closest_point_pairs] = line_point_1 - dot_Tnormal_T1L1 * triangle_unit_normal_vector;
            triangle_closest_point_params[num_closest_point_pairs] = 0.0f;
            triangle_closest_point_feature_ids[num_closest_point_pairs] =
                    make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);

            line_closest_points[num_closest_point_pairs] = line_point_1;
            line_closest_point_params[num_closest_point_pairs] = 0.0f;
            line_closest_point_feature_ids[num_closest_point_pairs] =
                    make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 0U);

            ++num_closest_point_pairs;
        }
        else if (L2_inside_triagle)
        {
            triangle_closest_points[num_closest_point_pairs] = line_point_2 - dot_Tnormal_T1L2 * triangle_unit_normal_vector;
            triangle_closest_point_params[num_closest_point_pairs] = 1.0f;
            triangle_closest_point_feature_ids[num_closest_point_pairs] =
                    make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);

            line_closest_points[num_closest_point_pairs] = line_point_2;
            line_closest_point_params[num_closest_point_pairs] = 1.0f;
            line_closest_point_feature_ids[num_closest_point_pairs] =
                    make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, 1U);

            ++num_closest_point_pairs;
        }
    }

    if ((triangle_edges_ignore_mask & 1U) == 0U && ((L1_behind_T12 && !L2_below_L1) || (L2_behind_T12 && !L1_below_L2)))
    {
        closest_points_of_two_lines(
                triangle_vertex_1,
                triangle_vertex_2,
                line_point_1,
                line_point_2,
                &triangle_closest_points[num_closest_point_pairs],
                &triangle_closest_point_params[num_closest_point_pairs],
                &line_closest_points[num_closest_point_pairs],
                &line_closest_point_params[num_closest_point_pairs],
                nullptr,
                nullptr,
                nullptr,
                nullptr
                );
        triangle_closest_point_feature_ids[num_closest_point_pairs] =
                build_triangle_edge_feature_id(triangle_closest_point_params[num_closest_point_pairs], 0U, 1U);
        line_closest_point_feature_ids[num_closest_point_pairs] =
                build_line_feature_id(line_closest_point_params[num_closest_point_pairs]);
        ++num_closest_point_pairs;
    }

    if ((triangle_edges_ignore_mask & 2U) == 0U && ((L1_behind_T23 && !L2_below_L1) || (L2_behind_T23 && !L1_below_L2)))
    {
        closest_points_of_two_lines(
                triangle_vertex_2,
                triangle_vertex_3,
                line_point_1,
                line_point_2,
                &triangle_closest_points[num_closest_point_pairs],
                &triangle_closest_point_params[num_closest_point_pairs],
                &line_closest_points[num_closest_point_pairs],
                &line_closest_point_params[num_closest_point_pairs],
                nullptr,
                nullptr,
                nullptr,
                nullptr
                );
        triangle_closest_point_feature_ids[num_closest_point_pairs] =
                build_triangle_edge_feature_id(triangle_closest_point_params[num_closest_point_pairs], 2U, 3U);
        line_closest_point_feature_ids[num_closest_point_pairs] =
                build_line_feature_id(line_closest_point_params[num_closest_point_pairs]);
        ++num_closest_point_pairs;
    }

    if ((triangle_edges_ignore_mask & 4U) == 0U && ((L1_behind_T31 && !L2_below_L1) || (L2_behind_T31 && !L1_below_L2)))
    {
        closest_points_of_two_lines(
                triangle_vertex_3,
                triangle_vertex_1,
                line_point_1,
                line_point_2,
                &triangle_closest_points[num_closest_point_pairs],
                &triangle_closest_point_params[num_closest_point_pairs],
                &line_closest_points[num_closest_point_pairs],
                &line_closest_point_params[num_closest_point_pairs],
                nullptr,
                nullptr,
                nullptr,
                nullptr
                );
        triangle_closest_point_feature_ids[num_closest_point_pairs] =
                build_triangle_edge_feature_id(triangle_closest_point_params[num_closest_point_pairs], 3U, 1U);
        line_closest_point_feature_ids[num_closest_point_pairs] =
                build_line_feature_id(line_closest_point_params[num_closest_point_pairs]);
        ++num_closest_point_pairs;
    }

    if (is_line_parallel_with_plane_of_triangle)
    {
        INVARIANT(num_closest_point_pairs > 1U);

        natural_32_bit  index_1 = 0U, index_2 = 1U;
        if (num_closest_point_pairs == 3U)
        {
            float_32_bit const  distance2_1 = length_squared(triangle_closest_points[0] - line_closest_points[0]);
            float_32_bit const  distance2_2 = length_squared(triangle_closest_points[1] - line_closest_points[1]);
            float_32_bit const  distance2_3 = length_squared(triangle_closest_points[2] - line_closest_points[2]);

            if (distance2_1 < distance2_2)
            {
                if (distance2_3 < distance2_2)
                    index_2 = 2;
            }
            else
            {
                if (distance2_3 < distance2_1)
                    index_1 = 2;
            }
        }

        if (output_triangle_closest_point_1 != nullptr)
            *output_triangle_closest_point_1 = triangle_closest_points[index_1];
        if (output_triangle_shape_feature_id_1 != nullptr)
            *output_triangle_shape_feature_id_1 = triangle_closest_point_feature_ids[index_1];

        if (output_line_closest_point_1 != nullptr)
            *output_line_closest_point_1 = line_closest_points[index_1];
        if (output_line_shape_feature_id_1 != nullptr)
            *output_line_shape_feature_id_1 = line_closest_point_feature_ids[index_1];

        if (output_triangle_closest_point_2 != nullptr)
            *output_triangle_closest_point_2 = triangle_closest_points[index_2];
        if (output_triangle_shape_feature_id_2 != nullptr)
            *output_triangle_shape_feature_id_2 = triangle_closest_point_feature_ids[index_2];

        if (output_line_closest_point_2 != nullptr)
            *output_line_closest_point_2 = line_closest_points[index_2];
        if (output_line_shape_feature_id_2 != nullptr)
            *output_line_shape_feature_id_2 = line_closest_point_feature_ids[index_2];

        return 2U;
    }
    else
    {
        INVARIANT(num_closest_point_pairs > 0U);

        natural_32_bit  index = 0U;
        float_32_bit  distance2 = length_squared(triangle_closest_points[index] - line_closest_points[index]);
        for (natural_32_bit  i = 1U; i != num_closest_point_pairs; ++i)
        {
            float_32_bit const  i_distance2 = length_squared(triangle_closest_points[i] - line_closest_points[i]);
            if (i_distance2 < distance2)
                index = i;
        }

        if (output_triangle_closest_point_1 != nullptr)
            *output_triangle_closest_point_1 = triangle_closest_points[index];
        if (output_triangle_shape_feature_id_1 != nullptr)
            *output_triangle_shape_feature_id_1 = triangle_closest_point_feature_ids[index];

        if (output_line_closest_point_1 != nullptr)
            *output_line_closest_point_1 = line_closest_points[index];
        if (output_line_shape_feature_id_1 != nullptr)
            *output_line_shape_feature_id_1 = line_closest_point_feature_ids[index];

        return 1U;
    }
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


bool  is_point_inside_capsule(
        vector3 const&  point_in_sphere_local_space,
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line
        )
{
    vector3  central_line_closest_point;
    closest_point_on_line_to_point(
            { 0.0f, 0.0f, -half_distance_between_end_points},
            { 0.0f, 0.0f, +half_distance_between_end_points},
            point_in_sphere_local_space,
            &central_line_closest_point
            );
    return length_squared(point_in_sphere_local_space - central_line_closest_point)
                <= thickness_from_central_line *thickness_from_central_line;
}


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,
        vector2 const&  clip_origin,
        vector2 const&  clip_normal,
        clipped_polygon_description* const  description
        )
{
    TMPROF_BLOCK();

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
    TMPROF_BLOCK();

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
    intersection_normal = normalised_2d(orthogonal_2d(contract32(intersection_line_vector)));

    return POINT_SET_TYPE::GENERAL;
}


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,
        matrix44 const&  to_polygon_space_matrix,
        vector3 const&  clip_origin,
        vector3 const&  clip_normal,
        clipped_polygon_description* const  description
        )
{
    TMPROF_BLOCK();

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


POINT_SET_TYPE  clip_polygon(
        std::vector<vector2> const&  polygon_points,
        matrix44 const&  to_polygon_space_matrix,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::vector<vector2>* const  output_clipped_polygon_points,
        std::vector<natural_32_bit>* const  output_indices_of_intersection_points
        )
{
    TMPROF_BLOCK();

    natural_32_bit  free_buffer_index = ((clip_planes.size() & 1UL) == 0UL) ? 0U : 1U;

    std::array<std::vector<vector2>, 2U>  polygon_points_buffers;
    polygon_points_buffers.at(0U).reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);
    polygon_points_buffers.at(1U).reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);
    std::array<std::vector<vector2>*, 2U> const  polygon_points_buffer_ptr{
        output_clipped_polygon_points == nullptr ? &polygon_points_buffers.at(0U) : output_clipped_polygon_points,
        &polygon_points_buffers.at(1U)
    };
    std::vector<vector2> const*  polygon_points_ptr = &polygon_points;

    std::array<std::vector<natural_32_bit>, 2U>  intersection_points_buffers;
    intersection_points_buffers.at(0U).reserve(2U * clip_planes.size());
    intersection_points_buffers.at(1U).reserve(2U * clip_planes.size());
    std::array<std::vector<natural_32_bit>*, 2U> const  intersection_points_buffer_ptr{
        output_indices_of_intersection_points == nullptr ? &intersection_points_buffers.at(0U) : output_indices_of_intersection_points,
        &intersection_points_buffers.at(1U)
    };
    std::vector<natural_32_bit> const*  intersection_points_ptr = intersection_points_buffer_ptr.at(free_buffer_index);

    bool  was_clipped = false;
    for (std::pair<vector3, vector3> const&  origin_and_normal : clip_planes)
    {
        clipped_polygon_description  description;
        POINT_SET_TYPE const  set_type = angeo::clip_polygon(
                                                *polygon_points_ptr,
                                                to_polygon_space_matrix,
                                                origin_and_normal.first,
                                                origin_and_normal.second,
                                                &description
                                                );
        if (set_type == POINT_SET_TYPE::EMPTY)
            return POINT_SET_TYPE::EMPTY;
        if (set_type == POINT_SET_TYPE::FULL)
            continue;

        ++free_buffer_index;
        if (free_buffer_index == 2U)
            free_buffer_index = 0U;

        std::vector<vector2>&  free_polygon_points_buffer_ref = polygon_points_buffers.at(free_buffer_index);
        std::vector<natural_32_bit>&  free_intersection_points_buffer_ref = intersection_points_buffers.at(free_buffer_index);
        free_polygon_points_buffer_ref.clear();
        free_intersection_points_buffer_ref.clear();
        if (description.index_start <= description.index_end)
        {
            for (std::size_t  i = description.index_start; i != description.index_end; ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));

            for (natural_32_bit  idx : *intersection_points_ptr)
                if (idx >= description.index_start && idx < description.index_end)
                    free_intersection_points_buffer_ref.push_back(idx - (natural_32_bit)description.index_start);
        }
        else
        {
            for (std::size_t i = description.index_start; i != polygon_points_ptr->size(); ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));
            for (std::size_t i = 0U; i != description.index_end; ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));

            natural_32_bit const  end_shift = (natural_32_bit)(polygon_points_ptr->size() - description.index_start);
            for (natural_32_bit idx : *intersection_points_ptr)
                if (idx >= description.index_start)
                    free_intersection_points_buffer_ref.push_back(idx - (natural_32_bit)description.index_start);
                else if (idx < description.index_end)
                    free_intersection_points_buffer_ref.push_back(idx + end_shift);
        }
        free_intersection_points_buffer_ref.push_back((natural_32_bit)free_polygon_points_buffer_ref.size());
        free_polygon_points_buffer_ref.push_back(description.point_start);

        free_intersection_points_buffer_ref.push_back((natural_32_bit)free_polygon_points_buffer_ref.size());
        free_polygon_points_buffer_ref.push_back(description.point_end);

        free_polygon_points_buffer_ref.push_back(free_polygon_points_buffer_ref.front()); // Make the polygon cyclic.

        polygon_points_ptr = &free_polygon_points_buffer_ref;
        intersection_points_ptr = &free_intersection_points_buffer_ref;

        was_clipped = true;
    }

    if (was_clipped == false)
        return POINT_SET_TYPE::FULL;

    if (free_buffer_index == 0U)
    {
        if (output_clipped_polygon_points != nullptr)
            output_clipped_polygon_points->swap(polygon_points_buffers.front());
        if (output_indices_of_intersection_points != nullptr)
            output_indices_of_intersection_points->swap(intersection_points_buffers.front());
    }

    return POINT_SET_TYPE::GENERAL;
}


void  collision_box_box(
        vector3 const&  box_1_origin_in_world_space,
        vector3 const&  box_1_basis_x_vector_in_world_space,
        vector3 const&  box_1_basis_y_vector_in_world_space,
        vector3 const&  box_1_basis_z_vector_in_world_space,
        vector3 const&  box_1_min_corner_in_box_space,
        vector3 const&  box_1_max_corner_in_box_space,

        vector3 const&  box_2_origin_in_world_space,
        vector3 const&  box_2_basis_x_vector_in_world_space,
        vector3 const&  box_2_basis_y_vector_in_world_space,
        vector3 const&  box_2_basis_z_vector_in_world_space,
        vector3 const&  box_2_min_corner_in_box_space,
        vector3 const&  box_2_max_corner_in_box_space,

        std::vector<vector3>  output_collision_plane_points,
        vector3  ouptut_collision_plane_normal
        )
{
    // Build clip planes from faces of box 2.

    matrix44  box_2_from_space_matrix;
    compose_from_base_matrix(
            box_2_origin_in_world_space,
            box_2_basis_x_vector_in_world_space,
            box_2_basis_y_vector_in_world_space,
            box_2_basis_z_vector_in_world_space,
            box_2_from_space_matrix
            );
    vector3 const  box_2_min_corners_in_world = transform_point(box_2_min_corner_in_box_space, box_2_from_space_matrix);
    vector3 const  box_2_max_corners_in_world = transform_point(box_2_max_corner_in_box_space, box_2_from_space_matrix);
    std::vector< std::pair<vector3, vector3> > const  clip_planes {
        std::pair<vector3, vector3>{ box_2_min_corners_in_world, box_2_basis_x_vector_in_world_space },
        std::pair<vector3, vector3>{ box_2_min_corners_in_world, box_2_basis_y_vector_in_world_space },
        std::pair<vector3, vector3>{ box_2_min_corners_in_world, box_2_basis_z_vector_in_world_space },

        std::pair<vector3, vector3>{ box_2_max_corners_in_world, -box_2_basis_x_vector_in_world_space },
        std::pair<vector3, vector3>{ box_2_max_corners_in_world, -box_2_basis_y_vector_in_world_space },
        std::pair<vector3, vector3>{ box_2_max_corners_in_world, -box_2_basis_z_vector_in_world_space },
    };

    // Prepare data for cliping faces of box 1.

    matrix44  box_1_from_space_matrix;
    compose_from_base_matrix(
            box_1_origin_in_world_space,
            box_1_basis_x_vector_in_world_space,
            box_1_basis_y_vector_in_world_space,
            box_1_basis_z_vector_in_world_space,
            box_1_from_space_matrix
            );
    vector3 const  box_1_min_corners_in_world = transform_point(box_1_min_corner_in_box_space, box_1_from_space_matrix);
    vector3 const  box_1_max_corners_in_world = transform_point(box_1_max_corner_in_box_space, box_1_from_space_matrix);

    matrix44  to_polygon_space_matrix;
    std::vector<vector2>  polygon_points(5UL);

    {
        // Build polygon and to-space-matrix of face 1 of box 1 (xy-face-low).

        compose_to_base_matrix(
                box_1_min_corners_in_world,
                box_1_basis_y_vector_in_world_space,
                box_1_basis_x_vector_in_world_space,
                -box_1_basis_z_vector_in_world_space,
                to_polygon_space_matrix
                );
        polygon_points.at(0UL) = vector2(box_1_min_corner_in_box_space(1), box_1_min_corner_in_box_space(0));
        polygon_points.at(1UL) = vector2(box_1_max_corner_in_box_space(1), box_1_min_corner_in_box_space(0));
        polygon_points.at(2UL) = vector2(box_1_max_corner_in_box_space(1), box_1_max_corner_in_box_space(0));
        polygon_points.at(3UL) = vector2(box_1_min_corner_in_box_space(1), box_1_max_corner_in_box_space(0));
        polygon_points.at(4UL) = polygon_points.at(0UL);

        // Clip face 1 by all clip planes

        std::vector<vector2>  clipped_polygon_points;
        std::vector<natural_32_bit>  indices_of_intersection_points;
        POINT_SET_TYPE const  set_type = clip_polygon(
                                                polygon_points,
                                                to_polygon_space_matrix,
                                                clip_planes,
                                                &clipped_polygon_points,
                                                &indices_of_intersection_points
                                                );
    }
    {
        // Build polygon and to-space-matrix of face 2 of box 1 (xz-face-low).

        compose_to_base_matrix(
                box_1_min_corners_in_world,
                box_1_basis_x_vector_in_world_space,
                box_1_basis_z_vector_in_world_space,
                -box_1_basis_y_vector_in_world_space,
                to_polygon_space_matrix
                );
        polygon_points.at(0UL) = vector2(box_1_min_corner_in_box_space(0), box_1_min_corner_in_box_space(2));
        polygon_points.at(1UL) = vector2(box_1_max_corner_in_box_space(0), box_1_min_corner_in_box_space(2));
        polygon_points.at(2UL) = vector2(box_1_max_corner_in_box_space(0), box_1_max_corner_in_box_space(2));
        polygon_points.at(3UL) = vector2(box_1_min_corner_in_box_space(0), box_1_max_corner_in_box_space(2));
        polygon_points.at(4UL) = polygon_points.at(0UL);

        // Clip face 2 by all clip planes

        std::vector<vector2>  clipped_polygon_points;
        std::vector<natural_32_bit>  indices_of_intersection_points;
        POINT_SET_TYPE const  set_type = clip_polygon(
                                                polygon_points,
                                                to_polygon_space_matrix,
                                                clip_planes,
                                                &clipped_polygon_points,
                                                &indices_of_intersection_points
                                                );
    }





}


bool  intersection_of_covex_polytopes(
    )
{
    return false;
}


}
