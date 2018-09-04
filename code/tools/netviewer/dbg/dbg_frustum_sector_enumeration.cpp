#include <netviewer/dbg/dbg_frustum_sector_enumeration.hpp>
#include <netviewer/program_options.hpp>
#include <netview/enumerate.hpp>
#include <qtgl/batch_generators.hpp>
#include <utility/msgstream.hpp>


dbg_frustum_sector_enumeration::dbg_frustum_sector_enumeration()
    : m_enabled(false)
    , m_invalidated(false)
    , m_batches()
{}


void  dbg_frustum_sector_enumeration::disable()
{
    m_enabled = false;
    m_invalidated = false;
    m_batches.clear();
}


void  dbg_frustum_sector_enumeration::enumerate(
        std::vector< std::pair<vector3,vector3> > const&  clip_planes,
        std::vector<netlab::network_layer_props> const&  layer_props
        )
{
    if (!is_enabled())
        return;

    m_batches.clear();
    m_invalidated = false;
    std::vector< std::pair<vector3,qtgl::batch> >&  batches = m_batches;

    for (netlab::layer_index_type  layer_index = 0U; layer_index != layer_props.size(); ++layer_index)
    {
        auto const&  props = layer_props.at(layer_index);

        qtgl::batch  batch =
                qtgl::create_wireframe_box(
                        -0.5f * vector3{ props.distance_of_spikers_along_x_axis_in_meters(),
                                         props.distance_of_spikers_along_y_axis_in_meters(),
                                         props.distance_of_spikers_along_c_axis_in_meters() },
                        +0.5f * vector3{ props.distance_of_spikers_along_x_axis_in_meters(),
                                         props.distance_of_spikers_along_y_axis_in_meters(),
                                         props.distance_of_spikers_along_c_axis_in_meters() },
                        vector4(0.5f, 0.5f, 0.5f, 1.0f),
                        qtgl::FOG_TYPE::NONE,
                        msgstream() << "netviewer/spiker_sector_of_layer_" << layer_index
                        );
        netview::enumerate_spiker_positions(
                    props,
                    clip_planes,
                    [batch,&batches](vector3 const&  pos) -> bool
                    {
                        batches.push_back({pos,batch});
                        return true;
                    }
                    );

        batch = qtgl::create_wireframe_box(
                        -0.5f * vector3{ props.distance_of_docks_in_meters(),
                                         props.distance_of_docks_in_meters(),
                                         props.distance_of_docks_in_meters() },
                        +0.5f * vector3{ props.distance_of_docks_in_meters(),
                                         props.distance_of_docks_in_meters(),
                                         props.distance_of_docks_in_meters() },
                        vector4(0.5f, 0.5f, 0.5f, 1.0f),
                        qtgl::FOG_TYPE::NONE,
                        msgstream() << "netviewer/dock_sector_of_layer_" << layer_index
                        );
        netview::enumerate_dock_positions(
                    props,
                    clip_planes,
                    [batch,&batches](vector3 const&  pos) -> bool
                    {
                        batches.push_back({pos,batch});
                        return true;
                    }
                    );
    }
}


void  dbg_frustum_sector_enumeration::render(
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
