#include <netviewer/dbg/dbg_raycast_sector_enumeration.hpp>
#include <netviewer/program_options.hpp>
#include <netview/enumerate.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/draw.hpp>
#include <utility/msgstream.hpp>


namespace {


void  enumerate_sectors(
        std::string const&  id,
        vector3 const&  line_begin,
        vector3 const&  line_end,
        vector3 const&  low_corner,
        vector3 const&  high_corner,
        float_32_bit const  sector_size_x,
        float_32_bit const  sector_size_y,
        float_32_bit const  sector_size_c,
        std::vector< std::pair<vector3,qtgl::batch> >&  batches
        )
{
    qtgl::batch const  batch =
            qtgl::create_wireframe_box(
                    -0.5f * vector3{ sector_size_x, sector_size_y, sector_size_c },
                    +0.5f * vector3{ sector_size_x, sector_size_y, sector_size_c },
                    vector4(0.5f, 0.5f, 0.5f, 1.0f),
                    qtgl::FOG_TYPE::NONE,
                    id
                    );
    netview::enumerate_sectors_intersecting_line(
            line_begin,
            line_end,
            low_corner,
            high_corner,
            sector_size_x,
            sector_size_y,
            sector_size_c,
            [&low_corner,sector_size_x,sector_size_y,sector_size_c,batch,&batches](
                    netlab::sector_coordinate_type const  x,
                    netlab::sector_coordinate_type const  y,
                    netlab::sector_coordinate_type const  c
                    ) -> bool
            {
                vector3 const  shift {
                    (static_cast<float_32_bit>(x) + 0.5f) * sector_size_x,
                    (static_cast<float_32_bit>(y) + 0.5f) * sector_size_y,
                    (static_cast<float_32_bit>(c) + 0.5f) * sector_size_c,
                };
                batches.push_back({low_corner + shift,batch});
                return true;
            }
            );
}


}


dbg_raycast_sector_enumeration::dbg_raycast_sector_enumeration()
    : m_enabled(false)
    , m_batch_line()
    , m_batches()
{}


void  dbg_raycast_sector_enumeration::disable()
{
    m_enabled = false;
    m_batch_line.release();
    m_batches.clear();
}


void  dbg_raycast_sector_enumeration::enumerate(
        vector3 const&  line_begin, vector3 const&  line_end,
        std::vector<netlab::network_layer_props> const&  layer_props
        )
{
    if (!is_enabled())
        return;

    m_batch_line = qtgl::create_lines3d({{line_begin,line_end}},{0.0f,0.8f,0.9f,1.0f});

    m_batches.clear();
    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        auto const&  props = layer_props.at(layer_index);
        enumerate_sectors(
                msgstream() << "netviewer/spiker_sector_of_layer_" << layer_index << msgstream::end(),
                line_begin,
                line_end,
                props.low_corner_of_ships(),
                props.high_corner_of_ships(),
                props.distance_of_spikers_along_x_axis_in_meters(),
                props.distance_of_spikers_along_y_axis_in_meters(),
                props.distance_of_spikers_along_c_axis_in_meters(),
                m_batches
                );
        enumerate_sectors(
                msgstream() << "netviewer/dock_sector_of_layer_" << layer_index << msgstream::end(),
                line_begin,
                line_end,
                props.low_corner_of_ships(),
                props.high_corner_of_ships(),
                props.distance_of_docks_in_meters(),
                props.distance_of_docks_in_meters(),
                props.distance_of_docks_in_meters(),
                m_batches
                );
    }
}


void  dbg_raycast_sector_enumeration::render(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        ) const
{
    if (!is_enabled())
        return;

    if (!m_batch_line.empty())
        if (qtgl::make_current(m_batch_line, draw_state))
        {
            qtgl::render_batch(m_batch_line, matrix_from_world_to_camera, matrix_from_camera_to_clipspace);
            draw_state = m_batch_line.get_draw_state();
        }

    for (auto const&  pos_batch : m_batches)
        if (qtgl::make_current(pos_batch.second, draw_state))
        {
            qtgl::render_batch(
                pos_batch.second,
                matrix_from_world_to_camera,
                matrix_from_camera_to_clipspace,
                angeo::coordinate_system(pos_batch.first,quaternion_identity()),
                vector4(0.5f, 0.5f, 0.5f, 1.0f)
                );

            draw_state = pos_batch.second.get_draw_state();
        }
}
