#ifndef NETVIEW_UTILITY_HPP_INCLUDED
#   define NETVIEW_UTILITY_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <tuple>

namespace netview {


bool  is_bbox_behind_plane(
        vector3 const&  bbox_lo,
        vector3 const&  bbox_hi,
        vector3 const&  plane_origin,
        vector3 const&  plane_unit_normal
        );


bool  is_bbox_behind_any_of_planes(
        vector3 const&  bbox_lo,
        vector3 const&  bbox_hi,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes
        );


}

#endif
