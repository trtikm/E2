#include <netview/utility.hpp>
#include <angeo/collide.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <array>

namespace netview {


bool  is_bbox_behind_plane(
        vector3 const&  bbox_lo,
        vector3 const&  bbox_hi,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal
        )
{
    std::array<vector3,8>  corners;
    angeo::get_corner_points_of_bounding_box(bbox_lo,bbox_hi,corners.begin());
    for (auto const&  point : corners)
    {
        float_32_bit  dist;
        angeo::collision_point_and_plane(point,plane_origin,plane_unit_normal,&dist,nullptr);
        if (dist >= 0.0f)
            return false;
    }
    return true;
}


bool  is_bbox_behind_any_of_planes(
    vector3 const&  bbox_lo,
    vector3 const&  bbox_hi,
    std::vector< std::pair<vector3, vector3> > const&  clip_planes
    )
{
    TMPROF_BLOCK();

    for (auto const& origin_normal : clip_planes)
        if (is_bbox_behind_plane(bbox_lo, bbox_hi, origin_normal.first, origin_normal.second))
            return true;
    return false;
}


}
