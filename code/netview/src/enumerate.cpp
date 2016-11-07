#include <netview/enumerate.hpp>
#include <angeo/collide.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
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
        std::vector< std::pair<vector3,vector3> > const&  clip_planes
        )
{
    TMPROF_BLOCK();

    for (auto const&  origin_normal : clip_planes)
        if (is_bbox_behind_plane(bbox_lo,bbox_hi,origin_normal.first,origin_normal.second))
            return true;
    return false;
}


void  enumerate_spiker_positions(
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!clip_planes.empty());

    std::vector< std::pair<vector3,vector3> >  work_list{
        { layer_props.low_corner_of_spikers(), layer_props.high_corner_of_spikers() }
    };
    do
    {
        vector3 const  lo = work_list.back().first;
        vector3 const  hi = work_list.back().second;
        work_list.pop_back();

        if (!is_bbox_behind_any_of_planes(lo,hi,clip_planes))
        {
            vector3 const  diagonal = hi - lo;
            if (diagonal(0) > layer_props.distance_of_spikers_along_x_axis_in_meters() + 0.001f ||
                diagonal(1) > layer_props.distance_of_spikers_along_y_axis_in_meters() + 0.001f ||
                diagonal(2) > layer_props.distance_of_spikers_along_c_axis_in_meters() + 0.001f )
            {
                // TODO: split bbox(lo,hi) into 8 sub-bboxes and push them to the work_list.
                NOT_IMPLEMENTED_YET();
            }
            else
            {
                netlab::sector_coordinate_type  x,y,c;
                layer_props.spiker_sector_coordinates(0.5f * (lo + hi),x,y,c);
                if (output_callback(layer_props.spiker_sector_centre(x,y,c)) == false)
                    break;
            }
        }
    }
    while (!work_list.empty());
}

void  enumerate_spiker_positions(
        std::vector<netlab::network_layer_props> const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    for (auto const&  props : layer_props)
        enumerate_spiker_positions(props,clip_planes,output_callback);
}


}
