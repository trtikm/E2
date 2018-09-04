#include <netviewer/dbg/dbg_draw_movement_areas.hpp>
#include <netviewer/program_options.hpp>
#include <netview/utility.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/draw.hpp>
#include <utility/msgstream.hpp>


dbg_draw_movement_areas::dbg_draw_movement_areas()
    : m_enabled(false)
    , m_invalidated(false)
    , m_batches()
{}


void  dbg_draw_movement_areas::disable()
{
    m_enabled = false;
    m_invalidated = false;
    m_batches.clear();
}


void  dbg_draw_movement_areas::collect_visible_areas(
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::shared_ptr<netlab::network const> const  network
        )
{
    if (!is_enabled())
        return;

    m_batches.clear();
    m_invalidated = false;
    std::vector< std::pair<vector3,qtgl::batch> >&  batches = m_batches;

    std::vector<netlab::network_layer_props> const&  layer_props = network->properties()->layer_props();

    std::vector< std::vector< qtgl::batch > >  area_batches;
    {
        area_batches.resize(layer_props.size());
        for (netlab::layer_index_type layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
        {
            area_batches.at(layer_index).resize(layer_props.size());
            auto const&  props = layer_props.at(layer_index);
            for (netlab::layer_index_type area_layer_index = 0U; area_layer_index != layer_props.size(); ++area_layer_index)
                area_batches.at(layer_index).at(area_layer_index) =
                        qtgl::create_wireframe_box(
                                -0.5f * props.size_of_ship_movement_area_in_meters(area_layer_index),
                                +0.5f * props.size_of_ship_movement_area_in_meters(area_layer_index),
                                vector4(0.5f, 0.5f, 0.5f, 1.0f),
                                qtgl::FOG_TYPE::NONE,
                                msgstream() << "netviewer/movement_area_of_layer_" << layer_index << "_to_layer_" << area_layer_index
                                );
        }
    }

    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        auto const&  props = layer_props.at(layer_index);
        for (netlab::object_index_type spiker_index = 0ULL; spiker_index != props.num_spikers(); ++spiker_index)
        {
            auto const&  props = layer_props.at(layer_index);
            vector3 const&  area_center = network->get_layer_of_spikers(layer_index).get_movement_area_center(spiker_index);
            netlab::layer_index_type const  area_layer_index = network->properties()->find_layer_index(area_center(2));
            if (netview::is_bbox_behind_any_of_planes(
                        area_center - 0.5f * props.size_of_ship_movement_area_in_meters(area_layer_index),
                        area_center + 0.5f * props.size_of_ship_movement_area_in_meters(area_layer_index),
                        clip_planes))
                continue;
            batches.push_back({ area_center,area_batches.at(layer_index).at(area_layer_index) });
        }
    }
}


void  dbg_draw_movement_areas::render(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        ) const
{
    if (!is_enabled())
        return;

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
