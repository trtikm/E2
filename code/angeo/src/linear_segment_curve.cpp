#include <angeo/linear_segment_curve.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <algorithm>

namespace angeo {


float_32_bit  linear_segment_curve::operator()(float_32_bit const  x) const
{
    if (points.empty())
        return 0.0f;
    if (points.size() == 1UL)
        return points.front()(1);

    auto  f = [](vector2 const&  A, vector2 const&  B, float_32_bit const  x) -> float_32_bit {
        vector2 const  delta = B - A;
        if (delta(0) < 0.0001f)
        {
            UNREACHABLE(); // ERROR: points either do not represent a function, or they are not sorted.
            return 0.5f * (A(1) + B(1));
        }
        return A(1) + (delta(1) / delta(0)) * (x - A(0));
    };

    auto const  it = std::upper_bound(points.cbegin(), points.cend(),
                                      vector2{ x, 0.0f },
                                      [](vector2 const& l, vector2 const& r) { return l(0) < r(0); });
    if (it == points.cbegin())
        return f(points.front(), points.at(1UL), x);
    else if (it == points.cend())
        return f(points.at(points.size() - 2UL), points.back(), x);
    else
        return f(*std::prev(it), *it, x);
}


void  linear_segment_curve::sort()
{
    std::sort(points.begin(), points.end(), [](vector2 const& l, vector2 const& r) { return l(0) < r(0); });
}


}
