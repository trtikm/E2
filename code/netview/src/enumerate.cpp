#include <netview/enumerate.hpp>
#include <netview/utility.hpp>
#include <angeo/collide.hpp>
#include <angeo/utility.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <array>

namespace netview { namespace detail {


struct  sector_id
{
    sector_id(
            netlab::sector_coordinate_type const  x,
            netlab::sector_coordinate_type const  y,
            netlab::sector_coordinate_type const  c
            )
        : m_x(x)
        , m_y(y)
        , m_c(c)
    {}

    netlab::sector_coordinate_type  x() const noexcept { return m_x; }
    netlab::sector_coordinate_type  y() const noexcept { return m_y; }
    netlab::sector_coordinate_type  c() const noexcept { return m_c; }

private:
    netlab::sector_coordinate_type  m_x;
    netlab::sector_coordinate_type  m_y;
    netlab::sector_coordinate_type  m_c;
};


void  compute_sector_sub_bboxes(
        sector_id const&  L,
        sector_id const&  H,
        std::vector< std::pair<sector_id,sector_id> >&  output
        )
{
    sector_id const  M { L.x() + (H.x() - L.x()) / 2U, L.y() + (H.y() - L.y()) / 2U, L.c() + (H.c() - L.c()) / 2U };
    output.push_back({ L, M });
    if (M.x()+1U <= H.x())
        output.push_back({ { M.x()+1U, L.y(), L.c() }, { H.x(), M.y(), M.c() } });
    if (M.y()+1U <= H.y())
        output.push_back({ { L.x(), M.y()+1U, L.c() }, { M.x(), H.y(), M.c() } });
    if (M.x()+1U <= H.x() && M.y()+1U <= H.y())
        output.push_back({ { M.x()+1U, M.y()+1U, L.c() }, { H.x(), H.y(), M.c() } });
    if (M.c()+1U <= H.c())
    {
        output.push_back({ { L.x(), L.y(), M.c()+1U }, { M.x(), M.y(), H.c() } });
        if (M.x()+1U <= H.x())
            output.push_back({ { M.x()+1U, L.y(), M.c()+1U }, { H.x(), M.y(), H.c() } });
        if (M.y()+1U <= H.y())
            output.push_back({ { L.x(), M.y()+1U, M.c()+1U }, { M.x(), H.y(), H.c() } });
        if (M.x()+1U <= H.x() && M.y()+1U <= H.y())
            output.push_back({ { M.x()+1U, M.y()+1U, M.c()+1U }, { H.x(), H.y(), H.c() } });
    }
}


natural_64_bit  enumerate_sector_positions(
        std::pair<sector_id,sector_id> const&  initial_bbox,
        vector3 const&  sector_half_shift,
        std::function<vector3(netlab::sector_coordinate_type,
                              netlab::sector_coordinate_type,
                              netlab::sector_coordinate_type)> const&  get_sector_centre,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(netlab::sector_coordinate_type,
                           netlab::sector_coordinate_type,
                           netlab::sector_coordinate_type)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!clip_planes.empty());

    natural_64_bit  count = 0UL;
    std::vector< std::pair<sector_id,sector_id> >  work_list{ initial_bbox };
    do
    {
        sector_id const  lo_id = work_list.back().first;
        sector_id const  hi_id = work_list.back().second;
        work_list.pop_back();

        vector3 const  lo = get_sector_centre(lo_id.x(),lo_id.y(),lo_id.c()) - sector_half_shift;
        vector3 const  hi = get_sector_centre(hi_id.x(),hi_id.y(),hi_id.c()) + sector_half_shift;
        if (!is_bbox_behind_any_of_planes(lo,hi,clip_planes))
            if (lo_id.x() == hi_id.x() && lo_id.y() == hi_id.y() && lo_id.c() == hi_id.c())
            {
                ++count;
                if (output_callback(lo_id.x(),lo_id.y(),lo_id.c()) == false)
                    break;
            }
            else
                compute_sector_sub_bboxes(lo_id,hi_id,work_list);
    }
    while (!work_list.empty());

    return count;
}


}}

namespace netview {


natural_64_bit  enumerate_spiker_positions(
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!clip_planes.empty());

    struct  local
    {
        static  bool  callback(std::function<bool(vector3 const&)> const&  output_callback,
                               netlab::network_layer_props const&  layer_props,
                               netlab::sector_coordinate_type const  x,
                               netlab::sector_coordinate_type const  y,
                               netlab::sector_coordinate_type const  c
                               )
        {
            return output_callback(layer_props.spiker_sector_centre(x,y,c));
        }
    };

    return detail::enumerate_sector_positions(
                { { 0U, 0U, 0U },
                  { layer_props.num_spikers_along_x_axis() - 1U,
                    layer_props.num_spikers_along_y_axis() - 1U,
                    layer_props.num_spikers_along_c_axis() - 1U, }
                },
                0.5f * vector3(layer_props.distance_of_spikers_along_x_axis_in_meters(),
                               layer_props.distance_of_spikers_along_y_axis_in_meters(),
                               layer_props.distance_of_spikers_along_c_axis_in_meters()),
                std::bind(&netlab::network_layer_props::spiker_sector_centre,std::cref(layer_props),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                clip_planes,
                std::bind(&local::callback,std::cref(output_callback),std::cref(layer_props),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
                );
}

natural_64_bit  enumerate_spiker_positions(
        std::vector<netlab::network_layer_props> const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    natural_64_bit  count = 0UL;
    for (auto const&  props : layer_props)
        count += enumerate_spiker_positions(props,clip_planes,output_callback);
    return count;
}


}

namespace netview {


natural_64_bit  enumerate_dock_positions(
        netlab::network_layer_props const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!clip_planes.empty());

    struct  local
    {
        static  bool  callback(std::function<bool(vector3 const&)> const&  output_callback,
                               netlab::network_layer_props const&  layer_props,
                               netlab::sector_coordinate_type const  x,
                               netlab::sector_coordinate_type const  y,
                               netlab::sector_coordinate_type const  c
                               )
        {
            return output_callback(layer_props.dock_sector_centre(x,y,c));
        }
    };

    return detail::enumerate_sector_positions(
                { { 0U, 0U, 0U },
                  { layer_props.num_docks_along_x_axis() - 1U,
                    layer_props.num_docks_along_y_axis() - 1U,
                    layer_props.num_docks_along_c_axis() - 1U, }
                },
                0.5f * vector3(layer_props.distance_of_docks_in_meters(),
                               layer_props.distance_of_docks_in_meters(),
                               layer_props.distance_of_docks_in_meters()),
                std::bind(&netlab::network_layer_props::dock_sector_centre,std::cref(layer_props),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                clip_planes,
                std::bind(&local::callback,std::cref(output_callback),std::cref(layer_props),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
                );
}

natural_64_bit  enumerate_dock_positions(
        std::vector<netlab::network_layer_props> const&  layer_props,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    natural_64_bit  count = 0UL;
    for (auto const&  props : layer_props)
        count += enumerate_dock_positions(props,clip_planes,output_callback);
    return count;
}


}

namespace netview {


natural_64_bit  enumerate_ship_positions(
        netlab::network const&  network,
        netlab::layer_index_type const  layer_index,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(!clip_planes.empty());

    natural_64_bit  count = 0UL;

    struct  local
    {
        static  bool  callback(std::function<bool(vector3 const&)> const&  output_callback,
                               netlab::network const&  network,
                               netlab::layer_index_type const  layer_index,
                               natural_64_bit&  count,
                               netlab::sector_coordinate_type const  x,
                               netlab::sector_coordinate_type const  y,
                               netlab::sector_coordinate_type const  c
                               )
        {
            netlab::network_layer_props const&  layer_props = network.properties()->layer_props().at(layer_index);
            for (auto const&  layer_and_object_indices :
                 network.get_indices_of_ships_in_dock_sector(layer_index,layer_props.dock_sector_index(x,y,c)))
            {
                vector3 const&  ship_pos = network.get_layer_of_ships(layer_and_object_indices.layer_index())
                                                  .position(layer_and_object_indices.object_index());
                ++count;
                if (output_callback(ship_pos) == false)
                    return false;
            }
            return true;
        }
    };

    netlab::network_layer_props const&  layer_props = network.properties()->layer_props().at(layer_index);

    detail::enumerate_sector_positions(
                { { 0U, 0U, 0U },
                  { layer_props.num_docks_along_x_axis() - 1U,
                    layer_props.num_docks_along_y_axis() - 1U,
                    layer_props.num_docks_along_c_axis() - 1U, }
                },
                0.5f * vector3(layer_props.distance_of_docks_in_meters(),
                               layer_props.distance_of_docks_in_meters(),
                               layer_props.distance_of_docks_in_meters()),
                std::bind(&netlab::network_layer_props::dock_sector_centre,std::cref(layer_props),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3),
                clip_planes,
                std::bind(&local::callback,std::cref(output_callback),std::cref(network),layer_index,std::ref(count),
                          std::placeholders::_1,std::placeholders::_2,std::placeholders::_3)
                );

    return count;
}

natural_64_bit  enumerate_ship_positions(
        netlab::network const&  network,
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::function<bool(vector3 const&)> const&  output_callback
        )
{
    TMPROF_BLOCK();

    natural_64_bit  count = 0UL;
    for (natural_8_bit  layer_index = 0U; layer_index != network.properties()->layer_props().size(); ++layer_index)
        count += enumerate_ship_positions(network,layer_index,clip_planes,output_callback);
    return count;
}


void  enumerate_sectors_intersecting_line(
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  bbox_low_corner,
        vector3 const&  bbox_high_corner,
        float_32_bit const  sector_size_x,
        float_32_bit const  sector_size_y,
        float_32_bit const  sector_size_c,
        std::function<bool(netlab::sector_coordinate_type,
                           netlab::sector_coordinate_type,
                           netlab::sector_coordinate_type)> const&
            enumeration_callback_fn
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(sector_size_x > 0.0001f && sector_size_y > 0.0001f && sector_size_c > 0.0001f);

    vector3 A,B;
    bool const  is_intersection_non_empty =
            angeo::clip_line_into_bbox(
                line_begin,
                line_end,
                bbox_low_corner,
                bbox_high_corner,
                &A,
                &B,
                nullptr,
                nullptr
                );
    if (is_intersection_non_empty == false)
        return;

    netlab::sector_coordinate_type  x0,y0,c0;
    {
        vector3 const  u = A - bbox_low_corner;
        x0 = u(0) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(0) / sector_size_x));
        y0 = u(1) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(1) / sector_size_y));
        c0 = u(2) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(2) / sector_size_c));

        if ((static_cast<float_32_bit>(x0) + 0.5f) * sector_size_x > bbox_high_corner(0) - bbox_low_corner(0))
            --x0;
        if ((static_cast<float_32_bit>(y0) + 0.5f) * sector_size_y > bbox_high_corner(1) - bbox_low_corner(1))
            --y0;
        if ((static_cast<float_32_bit>(c0) + 0.5f) * sector_size_c > bbox_high_corner(2) - bbox_low_corner(2))
            --c0;
    }

    netlab::sector_coordinate_type  x1,y1,c1;
    {
        vector3 const  u = B - bbox_low_corner;
        x1 = u(0) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(0) / sector_size_x));
        y1 = u(1) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(1) / sector_size_y));
        c1 = u(2) < 0.0f ? 0U : static_cast<netlab::sector_coordinate_type>(std::floor(u(2) / sector_size_c));

        if ((static_cast<float_32_bit>(x1) + 0.5f) * sector_size_x > bbox_high_corner(0) - bbox_low_corner(0))
            --x1;
        if ((static_cast<float_32_bit>(y1) + 0.5f) * sector_size_y > bbox_high_corner(1) - bbox_low_corner(1))
            --y1;
        if ((static_cast<float_32_bit>(c1) + 0.5f) * sector_size_c > bbox_high_corner(2) - bbox_low_corner(2))
            --c1;
    }

    vector3 const  u = B - A;

    if (std::fabsf(u(0)) < 1e-4f)
        x1 = x0;
    if (std::fabsf(u(1)) < 1e-4f)
        y1 = y0;
    if (std::fabsf(u(2)) < 1e-4f)
        c1 = c0;

    while (true)
    {
        if (enumeration_callback_fn(x0,y0,c0) == false)
            break;

        if (x0 == x1 && y0 == y1 && c0 == c1)
            break;

        int  dir = 0; // 0~nothing, 1~+x, 2~-x, 3~+y, 4~-y, 5~+c, 6~-c
        {
            float_32_bit  param = 1.0f;

            if (x0 < x1)
            {
                float_32_bit const  plane_x = bbox_low_corner(0) + static_cast<float_32_bit>(x0+1U) * sector_size_x;
                float_32_bit const  t = (plane_x - A(0)) / u(0);
                if (t < param)
                {
                    param = t;
                    dir = 1;
                }
            }
            else if (x1 < x0)
            {
                float_32_bit const  plane_x = bbox_low_corner(0) + static_cast<float_32_bit>(x0) * sector_size_x;
                float_32_bit const  t = (plane_x - A(0)) / u(0);
                if (t < param)
                {
                    param = t;
                    dir = 2;
                }
            }

            if (y0 < y1)
            {
                float_32_bit const  plane_y = bbox_low_corner(1) + static_cast<float_32_bit>(y0+1U) * sector_size_y;
                float_32_bit const  t = (plane_y - A(1)) / u(1);
                if (t < param)
                {
                    param = t;
                    dir = 3;
                }
            }
            else if (y1 < y0)
            {
                float_32_bit const  plane_y = bbox_low_corner(1) + static_cast<float_32_bit>(y0) * sector_size_y;
                float_32_bit const  t = (plane_y - A(1)) / u(1);
                if (t < param)
                {
                    param = t;
                    dir = 4;
                }
            }

            if (c0 < c1)
            {
                float_32_bit const  plane_c = bbox_low_corner(2) + static_cast<float_32_bit>(c0+1U) * sector_size_c;
                float_32_bit const  t = (plane_c - A(2)) / u(2);
                if (t < param)
                {
                    param = t;
                    dir = 5;
                }
            }
            else if (c1 < c0)
            {
                float_32_bit const  plane_c = bbox_low_corner(2) + static_cast<float_32_bit>(c0) * sector_size_c;
                float_32_bit const  t = (plane_c - A(2)) / u(2);
                if (t < param)
                {
                    param = t;
                    dir = 6;
                }
            }
        }

        if (dir == 0)
            break;

        switch(dir)
        {
        case 1: ++x0; break;
        case 2: --x0; break;
        case 3: ++y0; break;
        case 4: --y0; break;
        case 5: ++c0; break;
        case 6: --c0; break;
        default: UNREACHABLE();
        }
    }
}


}
