#include <angeo/collide.hpp>
#include <angeo/tensor_hash.hpp>
#include <angeo/tensor_equal_to.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <unordered_set>
#include <unordered_map>
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
        natural_8_bit const  edges_ignore_mask,
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

    bool  edge_collider_output_state;
    auto const  edge_collider =
            [output_triangle_closest_point_ptr,
             output_triangle_shape_feature_id_ptr,
             output_parameter_ptr,
             &unit_normal_vector,
             edges_ignore_mask,
             &edge_collider_output_state](
                    vector3 const&  A,
                    vector3 const&  B,
                    natural_32_bit const  feature_index,
                    vector3 const&  P
                    )
            {
                vector3 const  AB = B - A;
                vector3 const  AP = P - A;
                vector3 const  binormal = cross_product(AB, unit_normal_vector);
                float_32_bit dot = dot_product(binormal, AP);
                if (dot < 0.0f)
                    return false;
                if (((1U << feature_index) & edges_ignore_mask) != 0U // Should be the passed edge ignored?
                    && dot > length(binormal) * 1e-4f) // Is the distance of the point to the edge greater then 1e-4f?
                {
                    edge_collider_output_state = false; // Yes, we indeed want to say there is no collison at all
                                                        // because the point lies behind ignored -> there must be
                                                        // another triangle sharing the edge which will report
                                                        // the collision.
                    return true;
                }
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
                edge_collider_output_state = true;
                return true;
            };

    if (edge_collider(triangle_vertex_1, triangle_vertex_2, 0U, point) == true)
        return edge_collider_output_state;
    if (edge_collider(triangle_vertex_2, triangle_vertex_3, 1U, point) == true)
        return edge_collider_output_state;
    if (edge_collider(triangle_vertex_3, triangle_vertex_1, 2U, point) == true)
        return edge_collider_output_state;

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

    auto const  ignore_edge =
        [triangle_edges_ignore_mask](
                natural_8_bit const  edge_mask,
                float_32_bit const  dot1,
                float_32_bit const  dot2,
                vector3 const&  binormal) -> bool {
            return (triangle_edges_ignore_mask & edge_mask) != 0U
                   && std::min(absolute_value(dot1), absolute_value(dot2)) > length(binormal) * 1e-4f;
        };

    vector3 const normal_T12 = cross_product(triangle_vertex_2 - triangle_vertex_1, triangle_unit_normal_vector);

    // Check that 'triangle_unit_normal_vector' agrees with the counter-clock-wise order of triangle's verices.
    ASSUMPTION(dot_product(normal_T12, triangle_vertex_3 - triangle_vertex_1) < 0.0f);

    float_32_bit const  dot_L1_T12 = dot_product(normal_T12, line_point_1 - triangle_vertex_1);
    float_32_bit const  dot_L2_T12 = dot_product(normal_T12, line_point_2 - triangle_vertex_1);

    bool const  L1_behind_T12 = dot_L1_T12 >= 0.0f;
    bool const  L2_behind_T12 = dot_L2_T12 >= 0.0f;

    if (L1_behind_T12 && L2_behind_T12)
    {
        if (ignore_edge(1U, dot_L1_T12, dot_L2_T12, normal_T12))
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

    float_32_bit const  dot_L1_T23 = dot_product(normal_T23, line_point_1 - triangle_vertex_2);
    float_32_bit const  dot_L2_T23 = dot_product(normal_T23, line_point_2 - triangle_vertex_2);

    bool const  L1_behind_T23 = dot_L1_T23 >= 0.0f;
    bool const  L2_behind_T23 = dot_L2_T23 >= 0.0f;

    if (L1_behind_T23 && L2_behind_T23)
    {
        if (ignore_edge(2U, dot_L1_T23, dot_L2_T23, normal_T23))
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

    vector3 const  normal_T31 = cross_product(triangle_vertex_1 - triangle_vertex_3, triangle_unit_normal_vector);

    float_32_bit const  dot_L1_T31 = dot_product(normal_T31, line_point_1 - triangle_vertex_3);
    float_32_bit const  dot_L2_T31 = dot_product(normal_T31, line_point_2 - triangle_vertex_3);

    bool const  L1_behind_T31 = dot_L1_T31 >= 0.0f;
    bool const  L2_behind_T31 = dot_L2_T31 >= 0.0f;

    if (L1_behind_T31 && L2_behind_T31)
    {
        if (ignore_edge(4U, dot_L1_T31, dot_L2_T31, normal_T31))
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

    if (!ignore_edge(1U, dot_L1_T12, dot_L2_T12, normal_T12)
        && ((L1_behind_T12 && !L2_below_L1) || (L2_behind_T12 && !L1_below_L2)))
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

    if (!ignore_edge(2U, dot_L1_T23, dot_L2_T23, normal_T23)
        && ((L1_behind_T23 && !L2_below_L1) || (L2_behind_T23 && !L1_below_L2)))
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

    if (!ignore_edge(4U, dot_L1_T31, dot_L2_T31, normal_T31)
        && ((L1_behind_T31 && !L2_below_L1) || (L2_behind_T31 && !L1_below_L2)))
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

        if (num_closest_point_pairs > 1U)
        {
            if (output_triangle_closest_point_2 != nullptr)
                *output_triangle_closest_point_2 = triangle_closest_points[index_2];
            if (output_triangle_shape_feature_id_2 != nullptr)
                *output_triangle_shape_feature_id_2 = triangle_closest_point_feature_ids[index_2];

            if (output_line_closest_point_2 != nullptr)
                *output_line_closest_point_2 = line_closest_points[index_2];
            if (output_line_shape_feature_id_2 != nullptr)
                *output_line_shape_feature_id_2 = line_closest_point_feature_ids[index_2];
        }

        return num_closest_point_pairs;
    }
    else
    {
        if (num_closest_point_pairs == 0U)
            return 0U;

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


float_32_bit  distance_from_center_of_capsule_to_surface_in_direction(
        float_32_bit const  half_distance_between_end_points, // end-points of the central line are aligned along the z-axis.
        float_32_bit const  thickness_from_central_line,
        vector3 const&  unit_direction
        )
{
    vector3 const  xy_projection(unit_direction(0), unit_direction(1), 0.0f);
    float_32_bit const  xy_projection_length = length(xy_projection);
    if (xy_projection_length < 1e-5f)
        return half_distance_between_end_points + thickness_from_central_line;
    vector3 const cylinder_surface_contact_vector = (thickness_from_central_line / xy_projection_length) * unit_direction;
    if (std::fabs(cylinder_surface_contact_vector(2)) < half_distance_between_end_points)
        return length(cylinder_surface_contact_vector);
    float_32_bit const  cos_alpha = std::fabs(unit_direction(2));
    float_32_bit const  D =
            thickness_from_central_line * thickness_from_central_line -
            half_distance_between_end_points * half_distance_between_end_points * (1.0f - cos_alpha * cos_alpha)
            ;
    float_32_bit const  D0 = std::max(0.0f, D);
    return  half_distance_between_end_points * cos_alpha + std::sqrt(D0);
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

    // clip_normal * (X - clip_origin) = 0
    // X = A + t*(B-A)  ; A = polygon_points.at(i-1), B = polygon_points.at(i)
    // --------------------
    // clip_normal * (A + t*(B-A) - clip_origin) = 0
    // t * clip_normal * (B-A) + clip_normal * (A - clip_origin) = 0
    // t * (dot(clip_normal,B) - dot(clip_normal,A)) + (dot(clip_normal,A) - dot(clip_normal,clip_origin)) = 0

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
            dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));
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
            dot_normal_B = dot_product_2d(clip_normal, polygon_points.at(i));
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

    vector3 const  intersection_line_vector = cross_product(normal, vector3_unit_z()); // This could be optimised, in case of performance issues.
    vector3 const  down_hill_line_vector = cross_product(normal, intersection_line_vector); // This could be optimised, in case of performance issues.

    if (std::fabs(down_hill_line_vector(2)) < 0.0001f)
        return (normal(2) >= 0.0f && origin(2) <= 0.0f) || (normal(2) <= 0.0f && origin(2) >= 0.0f) ?
                    POINT_SET_TYPE::FULL : POINT_SET_TYPE::EMPTY;

    scalar const  down_hill_param = -origin(2) / down_hill_line_vector(2);

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
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,  // First is origin, second unit normal
        std::vector<vector2>* const  output_clipped_polygon_points,
        std::vector<collision_shape_feature_id>* const  output_collision_shape_feature_ids,
        std::vector<natural_32_bit>* const  output_indices_of_intersection_points
        )
{
    TMPROF_BLOCK();

    auto const  build_intersection_feature_id =
        [](collision_shape_feature_id const&  feature_1, collision_shape_feature_id const&  feature_2) {
            COLLISION_SHAPE_FEATURE_TYPE const feature_type = as_collision_shape_feature_type(
                    feature_1.m_feature_type <= feature_2.m_feature_type ?
                            feature_1.m_feature_type :
                            feature_2.m_feature_type
                    );
            switch (feature_type)
            {
            case COLLISION_SHAPE_FEATURE_TYPE::VERTEX:
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::EDGE, feature_1.m_feature_index);
            case COLLISION_SHAPE_FEATURE_TYPE::EDGE:
                return make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::FACE, 0U);
            case COLLISION_SHAPE_FEATURE_TYPE::FACE:
                return make_collision_shape_feature_id(feature_type, 0U);
            default: UNREACHABLE(); break;
            }
        };

    natural_32_bit  free_buffer_index = ((clip_planes.size() & 1UL) == 0UL) ? 1U : 0U;

    std::array<std::vector<vector2>, 2U>  polygon_points_buffers;
    std::array<std::vector<vector2>*, 2U> const  polygon_points_buffer_ptr{
        output_clipped_polygon_points == nullptr ? &polygon_points_buffers.at(0U) : output_clipped_polygon_points,
        &polygon_points_buffers.at(1U)
    };
    polygon_points_buffer_ptr.at(0U)->reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);
    polygon_points_buffer_ptr.at(1U)->reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);

    std::vector<vector2> const*  polygon_points_ptr = &polygon_points;

    std::array<std::vector<collision_shape_feature_id>, 2U>  feature_id_buffers;
    std::array<std::vector<collision_shape_feature_id>*, 2U> const  feature_id_buffer_ptr{
        output_collision_shape_feature_ids == nullptr ? &feature_id_buffers.at(0U) : output_collision_shape_feature_ids,
        & feature_id_buffers.at(1U)
    };
    if (output_collision_shape_feature_ids != nullptr)
    {
        feature_id_buffer_ptr.at(0U)->reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);
        feature_id_buffer_ptr.at(1U)->reserve(polygon_points.size() + 2U * clip_planes.size() + 1U);

        std::vector<collision_shape_feature_id>& ids = *feature_id_buffer_ptr.at((free_buffer_index + 1U) % 2U);
        for (natural_32_bit  i = 0U; i != polygon_points.size(); ++i)
            ids.push_back(make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, i));
    }

    std::vector<collision_shape_feature_id> const*  feature_ids_ptr = feature_id_buffer_ptr.at((free_buffer_index + 1U) % 2U);

    std::array<std::vector<natural_32_bit>, 2U>  intersection_points_buffers;
    std::array<std::vector<natural_32_bit>*, 2U> const  intersection_points_buffer_ptr{
        output_indices_of_intersection_points == nullptr ? &intersection_points_buffers.at(0U) : output_indices_of_intersection_points,
        &intersection_points_buffers.at(1U)
    };
    if (output_indices_of_intersection_points != nullptr)
    {
        intersection_points_buffer_ptr.at(0U)->clear();
        intersection_points_buffer_ptr.at(0U)->reserve(2U * clip_planes.size());
        intersection_points_buffer_ptr.at(1U)->reserve(2U * clip_planes.size());
    }

    std::vector<natural_32_bit> const*  intersection_points_ptr = intersection_points_buffer_ptr.at((free_buffer_index + 1U) % 2U);

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

        std::vector<vector2>&  free_polygon_points_buffer_ref = *polygon_points_buffer_ptr.at(free_buffer_index);
        std::vector<collision_shape_feature_id>&  free_feature_id_buffer_ref = *feature_id_buffer_ptr.at(free_buffer_index);
        std::vector<natural_32_bit>&  free_intersection_points_buffer_ref = *intersection_points_buffer_ptr.at(free_buffer_index);
        free_polygon_points_buffer_ref.clear();
        if (output_collision_shape_feature_ids != nullptr)
            free_feature_id_buffer_ref.clear();
        if (output_indices_of_intersection_points != nullptr)
            free_intersection_points_buffer_ref.clear();
        if (description.index_start <= description.index_end)
        {
            for (std::size_t  i = description.index_start; i <= description.index_end ; ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));

            if (output_collision_shape_feature_ids != nullptr)
                for (std::size_t i = description.index_start; i <= description.index_end; ++i)
                    free_feature_id_buffer_ref.push_back(feature_ids_ptr->at(i));

            if (output_indices_of_intersection_points != nullptr)
                for (natural_32_bit  idx : *intersection_points_ptr)
                    if (idx >= description.index_start && idx <= description.index_end)
                        free_intersection_points_buffer_ref.push_back(idx - (natural_32_bit)description.index_start);
        }
        else
        {
            for (std::size_t i = description.index_start; i < polygon_points_ptr->size() - 1U; ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));
            for (std::size_t i = 0U; i <= description.index_end; ++i)
                free_polygon_points_buffer_ref.push_back(polygon_points_ptr->at(i));

            if (output_collision_shape_feature_ids != nullptr)
            {
                for (std::size_t i = description.index_start; i < polygon_points_ptr->size() - 1U; ++i)
                    free_feature_id_buffer_ref.push_back(feature_ids_ptr->at(i));
                for (std::size_t i = 0U; i <= description.index_end; ++i)
                    free_feature_id_buffer_ref.push_back(feature_ids_ptr->at(i));
            }

            if (output_indices_of_intersection_points != nullptr)
            {
                natural_32_bit const  end_shift = (natural_32_bit)(polygon_points_ptr->size() - 1U - description.index_start);
                for (natural_32_bit idx : *intersection_points_ptr)
                    if (idx >= description.index_start)
                        free_intersection_points_buffer_ref.push_back(idx - (natural_32_bit)description.index_start);
                    else if (idx <= description.index_end)
                        free_intersection_points_buffer_ref.push_back(idx + end_shift);
            }
        }

        if (output_indices_of_intersection_points != nullptr)
            free_intersection_points_buffer_ref.push_back((natural_32_bit)free_polygon_points_buffer_ref.size());
        if (output_collision_shape_feature_ids != nullptr)
            free_feature_id_buffer_ref.push_back(build_intersection_feature_id(
                    feature_ids_ptr->at(description.index_end),
                    feature_ids_ptr->at(description.index_end + 1U)
                    ));
        free_polygon_points_buffer_ref.push_back(description.point_start);

        if (output_indices_of_intersection_points != nullptr)
            free_intersection_points_buffer_ref.push_back((natural_32_bit)free_polygon_points_buffer_ref.size());
        if (output_collision_shape_feature_ids != nullptr)
            free_feature_id_buffer_ref.push_back(build_intersection_feature_id(
                    feature_ids_ptr->at(description.index_start - 1U),
                    feature_ids_ptr->at(description.index_start)
                    ));
        free_polygon_points_buffer_ref.push_back(description.point_end);

        free_polygon_points_buffer_ref.push_back(free_polygon_points_buffer_ref.front()); // Make the polygon cyclic.
        if (output_collision_shape_feature_ids != nullptr)
            free_feature_id_buffer_ref.push_back(free_feature_id_buffer_ref.front());   // Features must be cyclic too as they
                                                                                        // correspond to points.

        polygon_points_ptr = &free_polygon_points_buffer_ref;
        feature_ids_ptr = &free_feature_id_buffer_ref;
        intersection_points_ptr = &free_intersection_points_buffer_ref;

        free_buffer_index = (free_buffer_index + 1U) % 2U;

        was_clipped = true;
    }

    if (was_clipped == false)
        return POINT_SET_TYPE::FULL;

    if (free_buffer_index == 0U)
    {
        if (output_clipped_polygon_points != nullptr)
            output_clipped_polygon_points->swap(polygon_points_buffers.back());
        if (output_collision_shape_feature_ids != nullptr)
            output_collision_shape_feature_ids->swap(feature_id_buffers.back());
        if (output_indices_of_intersection_points != nullptr)
            output_indices_of_intersection_points->swap(intersection_points_buffers.back());
    }

    return POINT_SET_TYPE::GENERAL;
}


void  compute_polygons_of_box(
        coordinate_system_explicit const&  location,
        vector3 const&  half_sizes_along_axes,
        convex_polyhedron&  output_polygons
        )
{
    TMPROF_BLOCK();

    auto const  set_frame =
            [&output_polygons](natural_32_bit const  idx, vector3 const&  o, vector3 const&  x, vector3 const&  y, vector3 const&  z)
            {
                coordinate_system_explicit&  frame = output_polygons.polygon_frames.at(idx);
                frame.set_origin(o);
                frame.set_basis_vector_x(x);
                frame.set_basis_vector_y(y);
                frame.set_basis_vector_z(z);
            };
    auto const  set_polygon =
            [&output_polygons](natural_32_bit const  idx, float_32_bit const  half_x, float_32_bit const  half_y)
            {
                std::vector<vector2>&  points = output_polygons.polygons.at(idx);
                points.resize(5U);
                points.at(0UL) = vector2(-half_x, -half_y);
                points.at(1UL) = vector2( half_x, -half_y);
                points.at(2UL) = vector2( half_x,  half_y);
                points.at(3UL) = vector2(-half_x,  half_y);
                points.at(4UL) = points.front();
            };

    output_polygons.polygons.resize(6U);
    output_polygons.polygon_frames.resize(6U);

    // xy-face for z-min
    set_frame(0U, { 0.0f, 0.0f, -half_sizes_along_axes(2) }, vector3_unit_x(), vector3_unit_y(), vector3_unit_z());
    set_polygon(0U, half_sizes_along_axes(0), half_sizes_along_axes(1));

    // xy-face for z-max
    set_frame(1U, { 0.0f, 0.0f, half_sizes_along_axes(2) }, vector3_unit_x(), -vector3_unit_y(), -vector3_unit_z());
    set_polygon(1U, half_sizes_along_axes(0), half_sizes_along_axes(1));

    // xz-face for y-min
    set_frame(2U, { 0.0f, -half_sizes_along_axes(1), 0.0f }, vector3_unit_x(), -vector3_unit_z(), vector3_unit_y());
    set_polygon(2U, half_sizes_along_axes(0), half_sizes_along_axes(2));

    // xz-face for y-max
    set_frame(3U, { 0.0f, half_sizes_along_axes(1), 0.0f }, vector3_unit_x(), vector3_unit_z(), -vector3_unit_y());
    set_polygon(3U, half_sizes_along_axes(0), half_sizes_along_axes(2));

    // yz-face for x-min
    set_frame(4U, { -half_sizes_along_axes(0), 0.0f, 0.0f }, vector3_unit_y(), vector3_unit_z(), vector3_unit_x());
    set_polygon(4U, half_sizes_along_axes(1), half_sizes_along_axes(2));

    // yz-face for x-max
    set_frame(5U, { half_sizes_along_axes(0), 0.0f, 0.0f }, vector3_unit_y(), -vector3_unit_z(), -vector3_unit_x());
    set_polygon(5U, half_sizes_along_axes(1), half_sizes_along_axes(2));
}


void  compute_closest_box_feature_to_a_point(
        closest_box_feature_to_a_point&  output,
        vector3 const&  point_in_box_local_space,
        vector3 const&  box_half_sizes_along_axes,
        coordinate_system_explicit const&  box_location_in_word_space,
        float_32_bit const  max_edge_thickness
        )
{
    vector3 const  p{
            std::min(std::fabs(point_in_box_local_space(0) / box_half_sizes_along_axes(0)), 1.0f),
            std::min(std::fabs(point_in_box_local_space(1) / box_half_sizes_along_axes(1)), 1.0f),
            std::min(std::fabs(point_in_box_local_space(2) / box_half_sizes_along_axes(2)), 1.0f)
            };

    natural_32_bit  order[3] = {0, 1, 2};
    {
        // And we have to sort the 'order' array in the decreasing order.

        if (p(order[1]) > p(order[0]))
            std::swap(order[0], order[1]);
        if (p(order[2]) > p(order[1]))
        {
            std::swap(order[1], order[2]);
            if (p(order[1]) > p(order[0]))
                std::swap(order[0], order[1]);
        }
    }

    static_assert((natural_8_bit)COLLISION_SHAPE_FEATURE_TYPE::VERTEX == 0U, "");
    static_assert((natural_8_bit)COLLISION_SHAPE_FEATURE_TYPE::EDGE == 1U, "");
    static_assert((natural_8_bit)COLLISION_SHAPE_FEATURE_TYPE::FACE == 2U, "");

    natural_8_bit  feature_kind;    // 0=vertex, 1=edge, 2=face.
    natural_8_bit  coordinate;      // 0=vertex, 1=edge, 2=face.
    if (p(order[0]) < 0.0001f)
    {
        // A special case: The point is very close to the origin of the box.
        // So, we give up and return to FACE feature with the axis of the most thin side and
        // the impossiby bad sum of slopes.
        feature_kind = as_number(COLLISION_SHAPE_FEATURE_TYPE::FACE);
        coordinate = box_half_sizes_along_axes(0) < box_half_sizes_along_axes(1) ?
                        as_number(box_half_sizes_along_axes(0) < box_half_sizes_along_axes(2) ? COORDINATE::X : COORDINATE::Z) :
                        as_number(box_half_sizes_along_axes(1) < box_half_sizes_along_axes(2) ? COORDINATE::Y : COORDINATE::Z) ;
    }
    else
    {
        float_32_bit const  slope[2] = { p(order[1]) / p(order[0]), p(order[2]) / p(order[0]) };
        vector3 const  D{
                std::max(0.75f, 1.0f - max_edge_thickness / box_half_sizes_along_axes(0)),
                std::max(0.75f, 1.0f - max_edge_thickness / box_half_sizes_along_axes(1)),
                std::max(0.75f, 1.0f - max_edge_thickness / box_half_sizes_along_axes(2))
                };
        feature_kind = 0;
        coordinate = 0;
        if (slope[0] <= D(order[1]))
        {
            ++feature_kind;
            coordinate = 1;
        }
        if (slope[1] <= D(order[2]))
        {
            ++feature_kind;
            coordinate = 2;
        }
        if (feature_kind == 2U)
            coordinate = 0;
    }

    auto const get_feature_index =
        [](natural_8_bit const  coordinate, float_32_bit const  direction_sign, natural_8_bit const  shift) -> natural_16_bit {
            return ((1U + coordinate) | (direction_sign > 0.0f ? (1U << 2U) : 0U)) << (3U * shift);
        };

    output.feature_type = as_collision_shape_feature_type(feature_kind);
    switch (output.feature_type)
    {
    case COLLISION_SHAPE_FEATURE_TYPE::VERTEX:
        INVARIANT(coordinate == 0U); // sure, but does not matter as we do not use it here.
        output.feature_vector_in_world_space = vector3_zero(); // For this feature kind the vector is not used.
        output.distance_to_feature =
                length(
                    vector3 {
                        (point_in_box_local_space(order[0]) >= 0.0f ? 1.0f : -1.0f) * box_half_sizes_along_axes(order[0]),
                        (point_in_box_local_space(order[1]) >= 0.0f ? 1.0f : -1.0f) * box_half_sizes_along_axes(order[1]),
                        (point_in_box_local_space(order[2]) >= 0.0f ? 1.0f : -1.0f) * box_half_sizes_along_axes(order[2])
                        }
                    - point_in_box_local_space);
        output.feature_index = get_feature_index(order[0], point_in_box_local_space(order[0]), 0U) |
                               get_feature_index(order[1], point_in_box_local_space(order[1]), 1U) |
                               get_feature_index(order[2], point_in_box_local_space(order[2]), 2U) ;
        break;
    case COLLISION_SHAPE_FEATURE_TYPE::EDGE:
        INVARIANT(coordinate == 1U || coordinate == 2U);
        output.feature_vector_in_world_space = box_location_in_word_space.basis_vector(order[coordinate]);
        output.distance_to_feature =
                length_2d(vector2(box_half_sizes_along_axes(order[0]), box_half_sizes_along_axes(order[3U - coordinate])) -
                          vector2(point_in_box_local_space(order[0]), point_in_box_local_space(order[3U - coordinate])));
        output.feature_index = get_feature_index(order[0], point_in_box_local_space(order[0]), 0U) |
                               get_feature_index(order[3 - coordinate], point_in_box_local_space(order[3 - coordinate]), 1U) ;
        break;
    case COLLISION_SHAPE_FEATURE_TYPE::FACE:
        INVARIANT(coordinate == 0U);
        output.feature_vector_in_world_space =
                (point_in_box_local_space(order[0U]) >= 0.0f ? 1.0f : -1.0f) * box_location_in_word_space.basis_vector(order[0U]);
        output.distance_to_feature = box_half_sizes_along_axes(order[0U]) - std::fabs(point_in_box_local_space(order[0U]));
        output.feature_index = get_feature_index(order[0U], point_in_box_local_space(order[0U]), 0U);
        break;
    default: UNREACHABLE();
    }
    output.distance_to_feature = std::max(output.distance_to_feature, 0.0f);
}


vector3  compute_box_collision_unit_normal_and_penetration_depth_from_contact_point(
        vector3 const&  common_contact_point_in_world_space,

        coordinate_system_explicit const&  box_1_location,
        vector3 const&  box_1_half_sizes_along_axes,

        coordinate_system_explicit const&  box_2_location,
        vector3 const&  box_2_half_sizes_along_axes,

        float_32_bit&  output_penetration_depth,
        std::pair<collision_shape_feature_id, collision_shape_feature_id>* const  output_collision_shape_feature_id_ptr
        )
{
    vector3 const  point_1 =
            point3_to_orthonormal_base(
                    common_contact_point_in_world_space,
                    box_1_location.origin(),
                    box_1_location.basis_vector_x(),
                    box_1_location.basis_vector_y(),
                    box_1_location.basis_vector_z()
                    );
    closest_box_feature_to_a_point  props_1;
    compute_closest_box_feature_to_a_point(
            props_1,
            point_1,
            box_1_half_sizes_along_axes,
            box_1_location,
            0.005f
            );
    if (output_collision_shape_feature_id_ptr != nullptr)
    {
        output_collision_shape_feature_id_ptr->first.m_feature_type = as_number(props_1.feature_type);
        output_collision_shape_feature_id_ptr->first.m_feature_index = props_1.feature_index;
    }

    vector3 const  point_2 =
            point3_to_orthonormal_base(
                    common_contact_point_in_world_space,
                    box_2_location.origin(),
                    box_2_location.basis_vector_x(),
                    box_2_location.basis_vector_y(),
                    box_2_location.basis_vector_z()
                    );
    closest_box_feature_to_a_point  props_2;
    compute_closest_box_feature_to_a_point(
            props_2,
            point_2,
            box_2_half_sizes_along_axes,
            box_2_location,
            0.005f
            );
    if (output_collision_shape_feature_id_ptr != nullptr)
    {
        output_collision_shape_feature_id_ptr->second.m_feature_type = as_number(props_2.feature_type);
        output_collision_shape_feature_id_ptr->second.m_feature_index = props_2.feature_index;
    }

    if (props_1.feature_type == COLLISION_SHAPE_FEATURE_TYPE::FACE)
    {
        output_penetration_depth = props_1.distance_to_feature;
        return normalised(-props_1.feature_vector_in_world_space);
    }

    if (props_2.feature_type == COLLISION_SHAPE_FEATURE_TYPE::FACE)
    {
        output_penetration_depth = props_2.distance_to_feature;
        return normalised(props_2.feature_vector_in_world_space);
    }

    if (props_1.feature_type == COLLISION_SHAPE_FEATURE_TYPE::EDGE && props_2.feature_type == COLLISION_SHAPE_FEATURE_TYPE::EDGE)
    {
        output_penetration_depth = std::max(props_1.distance_to_feature, props_2.distance_to_feature);
        vector3  normal = cross_product(props_1.feature_vector_in_world_space, props_2.feature_vector_in_world_space);
        if (dot_product(normal, common_contact_point_in_world_space - box_1_location.origin()) < 0.0f)
            normal = -normal;
        return normalised(normal);
    }

    output_penetration_depth = std::max(props_1.distance_to_feature, props_2.distance_to_feature);
    vector3 const  origins_delta = box_2_location.origin() - box_1_location.origin();
    float_32_bit const  origins_distance = length(origins_delta);
    return origins_distance < 0.0001f ? vector3_unit_z() : (1.0f / origins_distance) * origins_delta;
}


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
        )
{
    matrix44  from_box_1_matrix, to_box_1_matrix;
    from_base_matrix(box_1_location, from_box_1_matrix);
    to_base_matrix(box_1_location, to_box_1_matrix);

    matrix44  from_box_2_matrix, to_box_2_matrix;
    from_base_matrix(box_2_location, from_box_2_matrix);
    to_base_matrix(box_2_location, to_box_2_matrix);

    matrix44 const  from_box_1_to_box_2_matrix = to_box_2_matrix * from_box_1_matrix;
    matrix44 const  from_box_2_to_box_1_matrix = to_box_1_matrix * from_box_2_matrix;

    std::vector< std::pair<vector3, vector3> >  box_1_clip_planes;  // First is origin, second unit normal
    for (natural_32_bit i = 0U, n = (natural_32_bit)box_1_polygons.polygons.size(); i < n; ++i)
        box_1_clip_planes.push_back({
                transform_point(box_1_polygons.polygon_frames.at(i).origin(), from_box_1_to_box_2_matrix),
                transform_vector(box_1_polygons.polygon_frames.at(i).basis_vector_z(), from_box_1_to_box_2_matrix),
                });

    std::vector< std::pair<vector3, vector3> >  box_2_clip_planes;  // First is origin, second unit normal
    for (natural_32_bit i = 0U, n = (natural_32_bit)box_2_polygons.polygons.size(); i < n; ++i)
        box_2_clip_planes.push_back({
                transform_point(box_2_polygons.polygon_frames.at(i).origin(), from_box_2_to_box_1_matrix),
                transform_vector(box_2_polygons.polygon_frames.at(i).basis_vector_z(), from_box_2_to_box_1_matrix),
                });

    using raw_collision_points_map =
            std::unordered_map<vector3, collision_shape_feature_id, tensor_hash<vector3, 1000U>, tensor_equal_to<vector3, 1000U> >;

    auto const  clip_polyhedron =
        [](convex_polyhedron const&  polyhedron,
            matrix44 const&  from_polyhedron_matrix,
           std::vector< std::pair<vector3, vector3> >  clip_planes,
           raw_collision_points_map&  output
           )
        {
            natural_32_bit const  n = (natural_32_bit)polyhedron.polygons.size();
            natural_32_bit  feature_id_bit_shift = 0U;
            while (1U << feature_id_bit_shift < n + 1U)
                ++feature_id_bit_shift;
            for (natural_32_bit  i = 0U; i < n; ++i)
            {
                matrix44  to_polygon_space_matrix;
                to_base_matrix(polyhedron.polygon_frames.at(i), to_polygon_space_matrix);

                std::vector<vector2>  clipped_polygon;
                std::vector<collision_shape_feature_id>  feature_ids;
                POINT_SET_TYPE const  type = clip_polygon(
                        polyhedron.polygons.at(i),
                        to_polygon_space_matrix,
                        clip_planes,
                        &clipped_polygon,
                        &feature_ids,
                        nullptr
                        );

                if (type == POINT_SET_TYPE::EMPTY)
                    continue;

                std::vector<vector2> const*  raw_collision_points_in_box_space;
                std::vector<collision_shape_feature_id> const*  raw_feature_ids = &feature_ids;
                if (type == POINT_SET_TYPE::FULL)
                {
                    raw_collision_points_in_box_space = &polyhedron.polygons.at(i);
                    feature_ids.resize(polyhedron.polygons.size());
                    for (natural_32_bit i = 0U; i != polyhedron.polygons.size(); ++i)
                        feature_ids.at(i) = make_collision_shape_feature_id(COLLISION_SHAPE_FEATURE_TYPE::VERTEX, i);
                }
                else
                    raw_collision_points_in_box_space = &clipped_polygon;

                for (collision_shape_feature_id&  id : feature_ids)
                    id.m_feature_index = (id.m_feature_index << feature_id_bit_shift) | i;

                INVARIANT(!raw_collision_points_in_box_space->empty());

                matrix44  from_polygon_space_matrix;
                from_base_matrix(polyhedron.polygon_frames.at(i), from_polygon_space_matrix);

                matrix44 const  from_polygon_to_world_matrix = from_polyhedron_matrix * from_polygon_space_matrix;
                for (natural_32_bit j = 0U, m = (natural_32_bit)raw_collision_points_in_box_space->size() - 1U; j < m; ++j)
                    output.insert({
                            transform_point(expand23(raw_collision_points_in_box_space->at(j), 0.0f), from_polygon_to_world_matrix),
                            feature_ids.at(j)
                            });
            }
        };

    raw_collision_points_map raw_collision_points_1;
    clip_polyhedron(box_1_polygons, from_box_1_matrix, box_2_clip_planes, raw_collision_points_1);

    raw_collision_points_map raw_collision_points_2;
    clip_polyhedron(box_2_polygons, from_box_2_matrix, box_1_clip_planes, raw_collision_points_2);

    if (raw_collision_points_1.size() + raw_collision_points_2.size() == 0UL)
        return false;

    vector3  mass_center_of_collision_points;
    float_32_bit  mass_centre_penetration_depth;
    std::pair<collision_shape_feature_id, collision_shape_feature_id>  mass_centre_collision_shape_feature_id;
    bool  is_face_collision_type;

    if (ouptut_collision_plane_unit_normal_in_world_space != nullptr)
    {
        mass_center_of_collision_points = vector3_zero();
        for (auto const&  point_and_feature : raw_collision_points_1)
            mass_center_of_collision_points += point_and_feature.first;
        for (auto const&  point_and_feature : raw_collision_points_2)
            mass_center_of_collision_points += point_and_feature.first;
        mass_center_of_collision_points /= (float_32_bit)(raw_collision_points_1.size() + raw_collision_points_2.size());

        *ouptut_collision_plane_unit_normal_in_world_space =
                compute_box_collision_unit_normal_and_penetration_depth_from_contact_point(
                        mass_center_of_collision_points,
                        box_1_location,
                        box_1_half_sizes_along_axes,
                        box_2_location,
                        box_2_half_sizes_along_axes,
                        mass_centre_penetration_depth,
                        &mass_centre_collision_shape_feature_id
                        );
        is_face_collision_type =
                mass_centre_collision_shape_feature_id.first.m_feature_type == as_number(COLLISION_SHAPE_FEATURE_TYPE::FACE) ||
                mass_centre_collision_shape_feature_id.second.m_feature_type == as_number(COLLISION_SHAPE_FEATURE_TYPE::FACE) ;

        INVARIANT(mass_centre_penetration_depth > -1e-5f);
    }

    if (output_collision_points_in_world_space != nullptr)
    {
        ASSUMPTION(ouptut_collision_plane_unit_normal_in_world_space != nullptr);

        if (is_face_collision_type)
        {
            struct  intersection_point_info
            {
                float_32_bit  depth;
                collision_shape_feature_id  feature_id_1;
                collision_shape_feature_id  feature_id_2;
            };

            std::unordered_map<vector3, intersection_point_info, tensor_hash<vector3, 1000U>, tensor_equal_to<vector3, 1000U> >  collision_points;
            for (auto  it_1 = raw_collision_points_1.cbegin(); it_1 != raw_collision_points_1.cend(); ++it_1)
            {
                auto const  it_2 = raw_collision_points_2.find(it_1->first);
                if (it_2 == raw_collision_points_2.end())
                    continue;

                vector3 const&  p = it_1->first;

                intersection_point_info  info;
                info.feature_id_1 = it_1->second;
                info.feature_id_2 = it_2->second;
                info.depth = mass_centre_penetration_depth;
                vector3 const  normal_shift =
                       project_to_unit_vector(
                               p - mass_center_of_collision_points,
                               *ouptut_collision_plane_unit_normal_in_world_space
                               );
                collision_points.insert({ p - normal_shift, info });
                //collision_points.insert({ p, info });
            }
            INVARIANT(!collision_points.empty());
            for (auto const&  point_and_info : collision_points)
            {
                output_collision_points_in_world_space->push_back(point_and_info.first);
                if (output_penetration_depths_of_collision_points != nullptr)
                    output_penetration_depths_of_collision_points->push_back(
                            mass_centre_penetration_depth + point_and_info.second.depth
                            );
                if (output_collision_shape_feature_ids != nullptr)
                    output_collision_shape_feature_ids->push_back(
                            { point_and_info.second.feature_id_1, point_and_info.second.feature_id_2 }
                            );
            }
        }
        else
        {
            output_collision_points_in_world_space->push_back(mass_center_of_collision_points);
            if (output_penetration_depths_of_collision_points != nullptr)
                output_penetration_depths_of_collision_points->push_back(mass_centre_penetration_depth);
            if (output_collision_shape_feature_ids != nullptr)
                output_collision_shape_feature_ids->push_back(mass_centre_collision_shape_feature_id);
        }
    }

    return true;
}


}
