#include <netviewer/simulator.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <netviewer/program_options.hpp>
#include <netlab/utility.hpp>
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <qtgl/camera_utils.hpp>
#include <netexp/experiment_factory.hpp>
#include <angeo/tensor_math.hpp>
#include <netview/enumerate.hpp>
#include <netview/raycast.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/random.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <atomic>
#include <thread>
#include <algorithm>
#include <cmath>

namespace {


std::shared_ptr<netlab::network>  g_constructed_network;
std::string  g_experiment_name;
enum struct NETWORK_CONSTRUCTION_STATE : natural_8_bit
{
    NOT_IN_CONSTRUCTION = 0U,
    PERFORMING_INITIALISATION_STEP = 1U,
    PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP = 2U,
};
std::atomic<NETWORK_CONSTRUCTION_STATE>  g_network_construction_state = NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION;
natural_64_bit  g_num_construction_steps = 0ULL;
std::thread  g_create_experiment_thread;
bool  g_pre_construction_pause_state = false;
bool  g_apply_pause_to_construction = false;

void  create_experiment_worker()
{
    TMPROF_BLOCK();

    ASSUMPTION(g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP);

    try
    {
        g_constructed_network =
                std::make_shared<netlab::network>(
                        netexp::experiment_factory::instance().create_network_props(g_experiment_name),
                        netexp::experiment_factory::instance().create_network_layers_factory(g_experiment_name)
                        );
        ASSUMPTION(g_constructed_network != nullptr);

        std::shared_ptr<netlab::initialiser_of_movement_area_centers> const centers_initialiser =
                netexp::experiment_factory::instance().create_initialiser_of_movement_area_centers(g_experiment_name);
        std::shared_ptr<netlab::initialiser_of_ships_in_movement_areas> const  ships_initialiser =
            netexp::experiment_factory::instance().create_initialiser_of_ships_in_movement_areas(g_experiment_name);

        g_network_construction_state = NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP;
        ++g_num_construction_steps;

        bool  done = false;
        while (true)
        {
            if (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP)
            {
                std::this_thread::yield();
                continue; // Wait with the next step till the simulator checks/draws the current state of the constructed network.
            }
            if (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION)
                return; // The network construction was force-terminated in the simulator's IDLE step.

            switch (g_constructed_network->get_state())
            {
            case netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_INITIALISATION:
                g_constructed_network->initialise_movement_area_centers(*centers_initialiser);
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STARTUP)
                {
                    g_constructed_network.reset();
                    done = true;
                }
                break;
            case netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STARTUP:
                g_constructed_network->prepare_for_movement_area_centers_migration(*centers_initialiser);
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP)
                {
                    g_constructed_network.reset();
                    done = true;
                }
                break;
            case netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP:
                g_constructed_network->do_movement_area_centers_migration_step(*centers_initialiser);
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP &&
                    g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_COMPUTATION_OF_SHIP_DENSITIES_IN_LAYERS)
                {
                    g_constructed_network.reset();
                    done = true;
                }
                break;
            case netlab::NETWORK_STATE::READY_FOR_COMPUTATION_OF_SHIP_DENSITIES_IN_LAYERS:
                g_constructed_network->compute_densities_of_ships_in_layers();
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_LUNCHING_SHIPS_INTO_MOVEMENT_AREAS)
                {
                    g_constructed_network.reset();
                    done = true;
                }
                break;
            case netlab::NETWORK_STATE::READY_FOR_LUNCHING_SHIPS_INTO_MOVEMENT_AREAS:
                g_constructed_network->lunch_ships_into_movement_areas(*ships_initialiser);
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_INITIALISATION_OF_MAP_FROM_DOCK_SECTORS_TO_SHIPS)
                {
                    g_constructed_network.reset();
                    done = true;
                }
                break;
            case netlab::NETWORK_STATE::READY_FOR_INITIALISATION_OF_MAP_FROM_DOCK_SECTORS_TO_SHIPS:
                g_constructed_network->initialise_map_from_dock_sectors_to_ships();
                if (g_constructed_network->get_state() != netlab::NETWORK_STATE::READY_FOR_SIMULATION_STEP)
                    g_constructed_network.reset();
                done = true;
                break;
            default:
                g_constructed_network.reset();
                done = true;
                break;
            }

            if (done)
            {
                g_network_construction_state = NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION;
                break;
            }

            g_network_construction_state = NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP;
            ++g_num_construction_steps;
        }
    }
    catch (std::exception const&)
    {
        g_constructed_network.reset();
        g_network_construction_state = NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION;
    }
}


}

namespace {


template<typename  numeric_type>
float_32_bit  percentage(numeric_type const  part, numeric_type const  base)
{
    ASSUMPTION(base != 0);
    return (float_32_bit)(float_64_bit)(100.0f * (float_64_bit)part / (float_64_bit)base);
}

template<typename  numeric_type>
std::string  percentage_string(numeric_type const  part, numeric_type const  base)
{
    return msgstream() << std::fixed << std::setprecision(3) << percentage(part,base);
};


}


simulator::simulator(
        vector3 const&  initial_clear_colour,
        bool const  paused,
        float_64_bit const  desired_network_to_real_time_ratio
        )
    : qtgl::real_time_simulator()

    , m_camera(
            qtgl::camera_perspective::create(
                    angeo::coordinate_system::create(
                            vector3(10.0f, 10.0f, 4.0f),
                            quaternion(0.293152988f, 0.245984003f, 0.593858004f, 0.707732975f)
                            ),
                    0.25f,
                    500.0f,
                    window_props()
                    )
            )
    , m_free_fly_config
            {
                {
                    false,
                    false,
                    2U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_W()),
                },
                {
                    false,
                    false,
                    2U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_S()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_A()),
                },
                {
                    false,
                    false,
                    0U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_D()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    -15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_Q()),
                },
                {
                    false,
                    false,
                    1U,
                    2U,
                    15.0f,
                    qtgl::free_fly_controler::keyboard_key_pressed(qtgl::KEY_E()),
                },
                {
                    true,
                    true,
                    2U,
                    0U,
                    -(10.0f * PI()) * (window_props().pixel_width_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
                {
                    true,
                    false,
                    0U,
                    1U,
                    -(10.0f * PI()) * (window_props().pixel_height_in_milimeters() / 1000.0f),
                    qtgl::free_fly_controler::mouse_button_pressed(qtgl::MIDDLE_MOUSE_BUTTON()),
                },
            }
    , m_batch_grid{ 
            qtgl::create_grid(
                    50.0f,
                    50.0f,
                    50.0f,
                    1.0f,
                    1.0f,
                    { 0.4f, 0.4f, 0.4f, 1.0f },
                    { 0.4f, 0.4f, 0.4f, 1.0f },
                    { 0.5f, 0.5f, 0.5f, 1.0f },
                    { 0.5f, 0.5f, 0.5f, 1.0f },
                    { 1.0f, 0.0f, 0.0f, 1.0f },
                    { 0.0f, 1.0f, 0.0f, 1.0f },
                    { 0.0f, 0.0f, 1.0f, 1.0f },
                    10U,
                    qtgl::GRID_MAIN_AXES_ORIENTATION_MARKER_TYPE::TRIANGLE,
                    qtgl::FOG_TYPE::NONE,
                    get_program_options()->dataRoot()
                    )
            }
    , m_do_show_grid(true)

    , m_network()
    , m_experiment_name()

    , m_paused(paused)
    , m_do_single_step(false)
    , m_spent_real_time(0.0)
    , m_spent_network_time(0.0)
    , m_num_network_updates(0UL)
    , m_desired_network_to_real_time_ratio(desired_network_to_real_time_ratio)

    , m_selected_object_stats()

    , m_effects_config(
            qtgl::effects_config::light_types{},
            qtgl::effects_config::lighting_data_types{
                { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            qtgl::effects_config::shader_output_types{ qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT },
            qtgl::FOG_TYPE::NONE
            )

    , m_batch_spiker{
        qtgl::batch(
            canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()}
                    / "shared" / "gfx" / "meshes" / "spiker"
                ),
            m_effects_config
            )
        }
    , m_batch_dock{
        qtgl::batch(
            canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()}
                    / "shared" / "gfx" / "meshes" / "dock"
                ),
            m_effects_config
            )
        }
    , m_batch_ship{
        qtgl::batch(
            canonical_path(
                boost::filesystem::path{get_program_options()->dataRoot()}
                    / "shared" / "gfx" / "meshes" / "ship"
                ),
                m_effects_config
                )
        }

    , m_batch_spiker_bbox{}
    , m_batch_dock_bbox{}
    , m_batch_ship_bbox{}

    , m_batch_spiker_bsphere{}
    , m_batch_dock_bsphere{}
    , m_batch_ship_bsphere{}

    , m_batches_selection{}

    , m_render_only_chosen_layer(false)
    , m_layer_index_of_chosen_layer_to_render(0U)

    , m_dbg_network_camera(m_camera->far_plane())
    , m_dbg_frustum_sector_enumeration()
    , m_dbg_raycast_sector_enumeration()
    , m_dbg_draw_movement_areas()
{
    TMPROF_BLOCK();
    set_clear_color(initial_clear_colour);
}

simulator::~simulator()
{
    TMPROF_BLOCK();

    if (is_network_being_constructed())
    {
        while (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP)
            std::this_thread::yield();
        g_constructed_network.reset();
        g_network_construction_state = NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION;
    }

    if (g_create_experiment_thread.joinable())
        g_create_experiment_thread.join();
}

void simulator::next_round(float_64_bit const  seconds_from_previous_call,
                           bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    if (!is_this_pure_redraw_request)
    {
        qtgl::adjust(*m_camera, window_props());
        auto const translated_rotated =
            qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                           seconds_from_previous_call, mouse_props(), keyboard_props());
        if (translated_rotated.first)
            call_listeners(simulator_notifications::camera_position_updated());
        if (translated_rotated.second)
            call_listeners(simulator_notifications::camera_orientation_updated());

        if (!is_network_being_constructed() && g_constructed_network != nullptr)
        {
            acquire_constructed_network();

            m_spent_real_time = 0.0;
            m_spent_network_time = 0.0;
            m_num_network_updates = 0UL;
            m_paused = g_pre_construction_pause_state;

            if (network()->properties()->layer_props().size() <= get_layer_index_of_chosen_layer_to_render())
            {
                m_render_only_chosen_layer = false;
                m_layer_index_of_chosen_layer_to_render = 0U;
            }
        }

        if (keyboard_props().was_just_released(qtgl::KEY_SPACE()))
        {
            if (paused())
            {
                m_paused = !m_paused;
                call_listeners(simulator_notifications::paused());
            }
            m_do_single_step = true;
        }

        if (!m_do_single_step && keyboard_props().was_just_released(qtgl::KEY_PAUSE()))
        {
            m_paused = !m_paused;
            call_listeners(simulator_notifications::paused());
        }

        if (network() != nullptr)
        {
            if (!paused())
                update_network(seconds_from_previous_call);

            update_selection_of_network_objects(seconds_from_previous_call);

            if (m_do_single_step)
            {
                m_paused = true;
                call_listeners(simulator_notifications::paused());
                m_do_single_step = false;
            }
        }
    }

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  matrix_from_world_to_camera;
    m_camera->to_camera_space_matrix(matrix_from_world_to_camera);
    matrix44  matrix_from_camera_to_clipspace;
    m_camera->projection_matrix(matrix_from_camera_to_clipspace);

    qtgl::draw_state  draw_state;
    if (m_do_show_grid)
        if (qtgl::make_current(m_batch_grid, draw_state))
        {
            qtgl::render_batch(m_batch_grid, matrix_from_world_to_camera, matrix_from_camera_to_clipspace);
            draw_state = m_batch_grid.get_draw_state();
        }

    if (network() != nullptr)
        render_network(matrix_from_world_to_camera, matrix_from_camera_to_clipspace,draw_state);
    else if (g_apply_pause_to_construction)
    {
        if (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP)
        {
            if (g_constructed_network->get_state() == netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP)
                render_constructed_network(matrix_from_world_to_camera, matrix_from_camera_to_clipspace,draw_state);
            if (!paused())
                g_network_construction_state = NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP;
        }
        if (!is_this_pure_redraw_request && m_do_single_step)
        {
            INVARIANT(!paused());
            m_paused = true;
            call_listeners(simulator_notifications::paused());
            m_do_single_step = false;
        }
    }
    else if (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP)
        g_network_construction_state = NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP;


    qtgl::swap_buffers();
}


void  simulator::update_network(float_64_bit const  seconds_from_previous_call)
{
    TMPROF_BLOCK();

    ASSUMPTION(network().operator bool());

    m_spent_real_time += seconds_from_previous_call;

    natural_64_bit  num_iterations = (natural_64_bit)
        std::max(
            0.0,
            std::round((desired_network_to_real_time_ratio() * spent_real_time() - spent_network_time())
                            / network()->properties()->update_time_step_in_seconds())
            );
    std::chrono::high_resolution_clock::time_point const  update_start_time = std::chrono::high_resolution_clock::now();
    for ( ; num_iterations != 0ULL || m_do_single_step; --num_iterations)
    {
        network()->do_simulation_step(
            true,true,true,
            m_selected_object_stats.operator bool() ? m_selected_object_stats.get() : nullptr
            );

        if (m_do_single_step)
            break;

        if (std::chrono::duration<float_64_bit>(std::chrono::high_resolution_clock::now() - update_start_time).count() > 1.0 / 60.0)
            break;
    }

    m_num_network_updates = network()->update_id();
    m_spent_network_time = network()->properties()->update_time_step_in_seconds() * m_num_network_updates;
}


void  simulator::update_selection_of_network_objects(float_64_bit const  seconds_from_previous_call)
{
    if (mouse_props().was_just_released(qtgl::LEFT_MOUSE_BUTTON()))
    {
        m_selected_object_stats.reset();
        m_batches_selection.clear();

        vector3  ray_begin, ray_end;
        {
            qtgl::camera_perspective const&  scene_camera =
                    *(m_dbg_network_camera.is_enabled() ? m_dbg_network_camera.get_camera() : m_camera);
            qtgl::cursor_line_begin(scene_camera, { mouse_props().x() ,mouse_props().y() }, window_props(), ray_begin);
            qtgl::cursor_line_end(scene_camera, ray_begin, ray_end);
        }

        natural_8_bit  object_kind = 0U; // 0-none, 1-spiker, 2-dock, 3-ship.
        netlab::compressed_layer_and_object_indices  object_indices(0U,0ULL);
        {
            float_32_bit  ray_param = 1.0f;

            if (m_batch_spiker.get_buffers_binding().loaded_successfully())
            {
                qtgl::spatial_boundary const  boundary = m_batch_spiker.get_buffers_binding().get_boundary();

                netlab::compressed_layer_and_object_indices  spiker_indices(0U, 0ULL);
                float_32_bit const  ray_spiker_param =
                    netview::find_first_spiker_on_line(
                            m_network->properties()->layer_props(),
                            ray_begin,
                            ray_end,
                            boundary.radius(),
                            spiker_indices
                            );
                if (ray_spiker_param < ray_param)
                {
                    ray_param = ray_spiker_param;
                    object_indices = spiker_indices;
                    object_kind = 1U;
                }
            }

            if (m_batch_dock.get_buffers_binding().loaded_successfully())
            {
                qtgl::spatial_boundary const  boundary = m_batch_dock.get_buffers_binding().get_boundary();

                netlab::compressed_layer_and_object_indices  dock_indices(0U, 0ULL);
                float_32_bit const  ray_dock_param =
                    netview::find_first_dock_on_line(
                            m_network->properties()->layer_props(),
                            ray_begin,
                            ray_end,
                            boundary.radius(),
                            dock_indices
                            );
                if (ray_dock_param < ray_param)
                {
                    ray_param = ray_dock_param;
                    object_indices = dock_indices;
                    object_kind = 2U;
                }
            }

            if (m_batch_ship.get_buffers_binding().loaded_successfully())
            {
                qtgl::spatial_boundary const  boundary = m_batch_ship.get_buffers_binding().get_boundary();

                netlab::compressed_layer_and_object_indices  ship_indices(0U, 0ULL);
                float_32_bit const  ray_ship_param =
                    netview::find_first_ship_on_line(
                            *m_network,
                            ray_begin,
                            ray_end,
                            boundary.radius(),
                            ship_indices
                            );
                if (ray_ship_param < ray_param)
                {
                    ray_param = ray_ship_param;
                    object_indices = ship_indices;
                    object_kind = 3U;
                }
                else if (ray_ship_param < 1.0f && ray_param < 1.0f)
                {
                    vector3 const ship_pos = ray_begin + ray_ship_param * (ray_end - ray_begin);
                    vector3 const other_pos = ray_begin + ray_param * (ray_end - ray_begin);
                    float_32_bit const  dist = length(ship_pos - other_pos);
                    if (dist <= boundary.radius())
                    {
                        ray_param = ray_ship_param;
                        object_indices = ship_indices;
                        object_kind = 3U;
                    }
                }
            }

            INVARIANT(object_kind < 4U);
            INVARIANT(object_kind == 0U || (ray_param >= 0.0f && ray_param < 1.0f));

            m_selected_object_stats =
                object_kind == 1U ? 
                        netexp::experiment_factory::instance().create_tracked_spiker_stats(get_experiment_name(), object_indices) :
                object_kind == 2U ?
                        netexp::experiment_factory::instance().create_tracked_dock_stats(get_experiment_name(), object_indices) :
                object_kind == 3U ?
                        netexp::experiment_factory::instance().create_tracked_ship_stats(get_experiment_name(), object_indices) :
                        std::shared_ptr<netlab::tracked_network_object_stats>{}
                        ;
        }

        //call_listeners(simulator_notifications::selection_changed());

        if (m_dbg_raycast_sector_enumeration.is_enabled())
            m_dbg_raycast_sector_enumeration.enumerate(ray_begin,ray_end,network()->properties()->layer_props());
    }
}


void  simulator::render_network(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (m_dbg_network_camera.is_enabled() && window_props().just_resized())
        m_dbg_network_camera.on_window_resized(window_props());

    std::vector< std::pair<vector3,vector3> >  clip_planes;
    qtgl::compute_clip_planes(
                *(m_dbg_network_camera.is_enabled() ? m_dbg_network_camera.get_camera() : m_camera),
                clip_planes
                );
    if (renders_only_chosen_layer() && get_layer_index_of_chosen_layer_to_render() < network()->properties()->layer_props().size())
    {
        auto const&  props = network()->properties()->layer_props().at(get_layer_index_of_chosen_layer_to_render());
        clip_planes.push_back({props.low_corner_of_ships(),vector3_unit_z()});
        clip_planes.push_back({props.high_corner_of_ships(),-vector3_unit_z()});
    }

    render_network_spikers(matrix_from_world_to_camera,matrix_from_camera_to_clipspace,clip_planes,draw_state);
    render_network_docks(matrix_from_world_to_camera, matrix_from_camera_to_clipspace,clip_planes,draw_state);
    render_network_ships(matrix_from_world_to_camera, matrix_from_camera_to_clipspace,clip_planes,draw_state);

    render_selected_network_object(matrix_from_world_to_camera, matrix_from_camera_to_clipspace,draw_state);

    if (m_dbg_network_camera.is_enabled())
        m_dbg_network_camera.render_camera_frustum(matrix_from_world_to_camera,matrix_from_camera_to_clipspace,draw_state);
    if (m_dbg_frustum_sector_enumeration.is_enabled())
    {
        if (m_dbg_frustum_sector_enumeration.is_invalidated() ||
                !m_dbg_network_camera.is_enabled() ||
                window_props().just_resized())
            m_dbg_frustum_sector_enumeration.enumerate(clip_planes,network()->properties()->layer_props());
        m_dbg_frustum_sector_enumeration.render(matrix_from_world_to_camera,matrix_from_camera_to_clipspace,draw_state);
    }
    if (m_dbg_raycast_sector_enumeration.is_enabled())
        m_dbg_raycast_sector_enumeration.render(matrix_from_world_to_camera,matrix_from_camera_to_clipspace,draw_state);
    if (m_dbg_draw_movement_areas.is_enabled())
    {
        if (m_dbg_draw_movement_areas.is_invalidated() ||
                !m_dbg_network_camera.is_enabled() ||
                window_props().just_resized())
            m_dbg_draw_movement_areas.collect_visible_areas(clip_planes,network());
        m_dbg_draw_movement_areas.render(matrix_from_world_to_camera,matrix_from_camera_to_clipspace,draw_state);
    }
}


void  simulator::render_network_spikers(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(m_batch_spiker, draw_state))
    {
        if (m_batch_spiker_bbox.empty())
        {
            INVARIANT(m_batch_spiker_bsphere.empty());
            qtgl::spatial_boundary const  boundary = m_batch_spiker.get_buffers_binding().get_boundary();

            m_batch_spiker_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                             vector4(0.5f, 0.5f, 0.5f, 1.0f),qtgl::FOG_TYPE::NONE,
                                                             "/netviewer/spiker_bbox");
            m_batch_spiker_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U, vector4(0.5f, 0.5f, 0.5f, 1.0f),
                                                                   qtgl::FOG_TYPE::NONE, "/netviewer/spiker_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch const  batch_spiker,
                        matrix44 const&  matrix_from_world_to_camera,
                        matrix44 const&  matrix_from_camera_to_clipspace,
                        vector3 const&  spiker_position
                        )
            {
                qtgl::render_batch(
                    batch_spiker,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(spiker_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_spiker_positions(
                    network()->properties()->layer_props(),
                    clip_planes,
                    std::bind(
                        &local::callback,
                        m_batch_spiker,
                        std::cref(matrix_from_world_to_camera),
                        std::cref(matrix_from_camera_to_clipspace),
                        std::placeholders::_1
                        )
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_spiker.get_draw_state();
    }
}


void  simulator::render_network_docks(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state&  draw_state)
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(m_batch_dock, draw_state))
    {
        if (m_batch_dock_bbox.empty())
        {
            INVARIANT(m_batch_dock_bsphere.empty());
            qtgl::spatial_boundary const  boundary = m_batch_dock.get_buffers_binding().get_boundary();

            m_batch_dock_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                           vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                           "/netviewer/dock_bbox");
            m_batch_dock_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U, vector4(0.5f, 0.5f, 0.5f, 1.0f),
                                                                 qtgl::FOG_TYPE::NONE,"/netviewer/dock_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch const  batch_dock,
                        matrix44 const&  matrix_from_world_to_camera,
                        matrix44 const&  matrix_from_camera_to_clipspace,
                        vector3 const&  dock_position
                        )
            {
                qtgl::render_batch(
                    batch_dock,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(dock_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_dock_positions(
                    network()->properties()->layer_props(),
                    clip_planes,
                    std::bind(
                        &local::callback,
                        m_batch_dock,
                        std::cref(matrix_from_world_to_camera),
                        std::cref(matrix_from_camera_to_clipspace),
                        std::placeholders::_1
                        )
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_dock.get_draw_state();
    }
}


void  simulator::render_network_ships(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(m_batch_ship, draw_state))
    {
        if (m_batch_ship_bbox.empty())
        {
            INVARIANT(m_batch_ship_bsphere.empty());
            qtgl::spatial_boundary const  boundary = m_batch_ship.get_buffers_binding().get_boundary();

            m_batch_ship_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                           vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                           "/netviewer/ship_bbox");
            m_batch_ship_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,vector4(0.5f, 0.5f, 0.5f, 1.0f),
                                                                 qtgl::FOG_TYPE::NONE,"/netviewer/ship_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch const  batch_ship,
                        matrix44 const&  matrix_from_world_to_camera,
                        matrix44 const&  matrix_from_camera_to_clipspace,
                        vector3 const&  ship_position
                        )
            {
                qtgl::render_batch(
                    batch_ship,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(ship_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_ship_positions(
                    *network(),
                    clip_planes,
                    std::bind(
                        &local::callback,
                        m_batch_ship,
                        std::cref(matrix_from_world_to_camera),
                        std::cref(matrix_from_camera_to_clipspace),
                        std::placeholders::_1
                        )
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_ship.get_draw_state();
    }
}


void  simulator::render_selected_network_object(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    if (!network().operator bool())
        return;

    if (std::dynamic_pointer_cast<netlab::tracked_spiker_stats>(m_selected_object_stats) != nullptr)
    {
        if (m_batch_spiker_bsphere.empty())
        {
            if (qtgl::make_current(m_batch_spiker_bsphere, draw_state))
            {
                netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
                netlab::sector_coordinate_type  x, y, c;
                props.spiker_sector_coordinates(m_selected_object_stats->indices().object_index(), x, y, c);
                vector3 const&  pos = props.spiker_sector_centre(x, y, c);
                qtgl::render_batch(
                    m_batch_spiker_bsphere,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(pos, quaternion_identity()),
                    { 1.0f, 1.0f, 1.0f, 1.0f }
                );
                draw_state = m_batch_spiker_bsphere.get_draw_state();
            }
        }
        if (m_batches_selection.empty())
        {
            {
                vector3 const  sector_center = netlab::spiker_sector_centre(network()->properties()->layer_props(),
                                                                            m_selected_object_stats->indices());
                vector3 const  sector_shift = netlab::shift_from_low_corner_of_spiker_sector_to_center(
                                                        network()->properties()->layer_props(),
                                                        m_selected_object_stats->indices());
                m_batches_selection.push_back(
                        qtgl::create_wireframe_box(sector_center - sector_shift,sector_center + sector_shift,
                                                   vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                   "/netviewer/selected_spiker_sector_bbox")
                        );
            }
            {
                vector3 const&  area_center =
                        network()->get_layer_of_spikers(m_selected_object_stats->indices().layer_index())
                                  .get_movement_area_center(m_selected_object_stats->indices().object_index());
                netlab::layer_index_type const  area_layer_index =
                        network()->properties()->find_layer_index(area_center(2));
                vector3 const  area_shift =
                        0.5f * network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index())
                                                                     .size_of_ship_movement_area_in_meters(area_layer_index);
                m_batches_selection.push_back(
                        qtgl::create_wireframe_box(area_center - area_shift,area_center + area_shift,
                                                   vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                   "/netviewer/selected_spiker_ship_movement_area")
                        );
                m_batches_selection.push_back(
                        qtgl::create_lines3d(
                                {
                                    std::pair<vector3,vector3>(
                                        area_center - vector3( 1.0f,0.0f,0.0f),
                                        area_center + vector3( 1.0f,0.0f,0.0f)
                                        ),
                                    std::pair<vector3,vector3>(
                                        area_center - vector3{0.0f, 1.0f,0.0f},
                                        area_center + vector3{0.0f, 1.0f,0.0f}
                                        ),
                                    std::pair<vector3,vector3>(
                                        area_center - vector3{0.0f,0.0f, 1.0f},
                                        area_center + vector3{0.0f,0.0f, 1.0f}
                                        )
                                },
                                vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
                                qtgl::FOG_TYPE::NONE,
                                "/netviewer/selected_spiker_ship_movement_area_center"
                                )
                        );
            }
        }
        static std::vector<vector4> const colours = {
            vector4{ 0.5f, 0.5f, 0.5f, 1.0f },
            vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
            vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
        };
        INVARIANT(m_batches_selection.size() == colours.size());
        for (std::size_t  i = 0ULL; i != colours.size(); ++i)
        {
            if (qtgl::make_current(m_batches_selection.at(i), draw_state))
            {
                qtgl::render_batch(
                    m_batches_selection.at(i),
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(vector3_zero(), quaternion_identity()),
                    colours.at(i)
                );
                draw_state = m_batches_selection.at(i).get_draw_state();
            }
        }
        {
            netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
            vector3 const  spiker_sector_centre =
                netlab::spiker_sector_centre(props, m_selected_object_stats->indices().object_index());
            netlab::object_index_type const  ships_begin_index =
                props.ships_begin_index_of_spiker(m_selected_object_stats->indices().object_index());
            std::vector< std::pair<vector3, vector3> >  lines;
            for (natural_32_bit i = 0U; i < props.num_ships_per_spiker(); ++i)
                lines.push_back({
                        spiker_sector_centre,
                        network()->get_layer_of_ships(m_selected_object_stats->indices().layer_index()).position(ships_begin_index + i)
                        });
            qtgl::batch const  batch =
                    qtgl::create_lines3d(lines, { 0.5f, 0.5f, 0.5f, 1.0f }, qtgl::FOG_TYPE::NONE,
                                         "/netviewer/selected_spiker_lines_to_ships");
            if (qtgl::make_current(batch, draw_state))
            {
                qtgl::render_batch(
                    batch,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(vector3_zero(), quaternion_identity())
                    );
                draw_state = batch.get_draw_state();
            }
        }
    }
    else if (std::dynamic_pointer_cast<netlab::tracked_dock_stats>(m_selected_object_stats) != nullptr)
    {
        if (!m_batch_dock_bsphere.empty())
        {
            if (qtgl::make_current(m_batch_dock_bsphere, draw_state))
            {
                netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
                netlab::sector_coordinate_type  x,y,c;
                props.dock_sector_coordinates(m_selected_object_stats->indices().object_index(),x,y,c);
                vector3 const&  pos = props.dock_sector_centre(x,y,c);
                qtgl::render_batch(
                    m_batch_dock_bsphere,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(pos,quaternion_identity()),
                    { 1.0f, 1.0f, 1.0f, 1.0f }
                    );
                draw_state = m_batch_dock_bsphere.get_draw_state();
            }
        }
        if (m_batches_selection.empty())
        {
            {
                vector3 const  sector_center = netlab::dock_sector_centre(network()->properties()->layer_props(),
                                                                          m_selected_object_stats->indices());
                vector3 const  sector_shift = netlab::shift_from_low_corner_of_dock_sector_to_center(
                                                        network()->properties()->layer_props(),
                                                        m_selected_object_stats->indices());
                m_batches_selection.push_back(
                        qtgl::create_wireframe_box(sector_center - sector_shift,sector_center + sector_shift,
                                                   vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                   "/netviewer/selected_dock_sector_bbox")
                        );
            }
            {
                netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
                netlab::sector_coordinate_type  dock_x, dock_y, dock_c;
                props.dock_sector_coordinates(m_selected_object_stats->indices().object_index(), dock_x, dock_y, dock_c);
                netlab::sector_coordinate_type  x,y,c;
                props.spiker_sector_coordinates_from_dock_sector_coordinates(dock_x, dock_y, dock_c, x, y, c);
                vector3 const  sector_center = props.spiker_sector_centre(x,y,c);
                vector3 const  sector_shift = netlab::shift_from_low_corner_of_spiker_sector_to_center(props);
                m_batches_selection.push_back(
                    qtgl::create_wireframe_box(sector_center - sector_shift, sector_center + sector_shift,
                                               vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                               "/netviewer/selected_spiker_sector_bbox")
                                               );
                m_batches_selection.push_back(
                        qtgl::create_lines3d(
                                { { sector_center, netlab::dock_sector_centre(props,m_selected_object_stats->indices().object_index()) } },
                                { 0.5f, 0.5f, 0.5f, 1.0f },
                                qtgl::FOG_TYPE::NONE,
                                "/netviewer/selected_dock_line_to_spiker"
                                )
                        );
            }
        }
        static std::vector<vector4> const colours = {
            vector4{ 0.5f, 0.5f, 0.5f, 1.0f },
            vector4{ 0.5f, 0.5f, 0.5f, 1.0f },
            vector4{ 0.5f, 0.5f, 0.5f, 1.0f },
        };
        INVARIANT(m_batches_selection.size() == colours.size());
        for (std::size_t  i = 0ULL; i != colours.size(); ++i)
        {
            if (qtgl::make_current(m_batches_selection.at(i), draw_state))
            {
                qtgl::render_batch(
                    m_batches_selection.at(i),
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(vector3_zero(), quaternion_identity()),
                    colours.at(i)
                );
                draw_state = m_batches_selection.at(i).get_draw_state();
            }
        }
    }
    else if (std::dynamic_pointer_cast<netlab::tracked_ship_stats>(m_selected_object_stats) != nullptr)
    {
        if (!m_batch_ship_bsphere.empty())
        {
            if (qtgl::make_current(m_batch_ship_bsphere, draw_state))
            {
                vector3 const&  pos =
                    network()->get_layer_of_ships(m_selected_object_stats->indices().layer_index())
                              .position(m_selected_object_stats->indices().object_index());
                qtgl::render_batch(
                    m_batch_ship_bsphere,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(pos,quaternion_identity()),
                    { 1.0f, 1.0f, 1.0f, 1.0f }
                    );
                draw_state = m_batch_ship_bsphere.get_draw_state();
            }
        }
        {
            netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
            netlab::object_index_type const  spiker_index =
                    props.spiker_index_from_ship_index(m_selected_object_stats->indices().object_index());
            qtgl::batch const  batch =
                qtgl::create_lines3d(
                        {{
                            network()->get_layer_of_ships(m_selected_object_stats->indices().layer_index())
                                      .position(m_selected_object_stats->indices().object_index()),
                            netlab::spiker_sector_centre(props,spiker_index)
                        }},
                        { 0.5f, 0.5f, 0.5f, 1.0f },
                        qtgl::FOG_TYPE::NONE,
                        "/netviewer/selected_ship_line_to_spiker"
                        );
            if (qtgl::make_current(batch, draw_state))
            {
                qtgl::render_batch(
                    batch,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(vector3_zero(), quaternion_identity())
                    );
                draw_state = batch.get_draw_state();
            }
        }
        if (m_batches_selection.empty())
        {
            netlab::network_layer_props const&  props =
                    network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
            vector3 const&  area_center =
                    network()->get_layer_of_spikers(m_selected_object_stats->indices().layer_index())
                              .get_movement_area_center(
                                    props.spiker_index_from_ship_index(m_selected_object_stats->indices().object_index()));
            netlab::layer_index_type const  area_layer_index =
                    network()->properties()->find_layer_index(area_center(2));
            vector3 const  area_shift = 0.5f * props.size_of_ship_movement_area_in_meters(area_layer_index);
            m_batches_selection.push_back(
                    qtgl::create_wireframe_box(area_center - area_shift,area_center + area_shift,
                                               vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                               "/netviewer/selected_ship_movement_area")
                    );
            m_batches_selection.push_back(
                    qtgl::create_lines3d(
                            {
                                std::pair<vector3,vector3>(
                                    area_center - vector3( 1.0f,0.0f,0.0f),
                                    area_center + vector3( 1.0f,0.0f,0.0f)
                                    ),
                                std::pair<vector3,vector3>(
                                    area_center - vector3{0.0f, 1.0f,0.0f},
                                    area_center + vector3{0.0f, 1.0f,0.0f}
                                    ),
                                std::pair<vector3,vector3>(
                                    area_center - vector3{0.0f,0.0f, 1.0f},
                                    area_center + vector3{0.0f,0.0f, 1.0f}
                                    )
                            },
                            vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
                            qtgl::FOG_TYPE::NONE,
                            "/netviewer/selected_ship_movement_area_center"
                            )
                    );
        }
        static std::vector<vector4> const colours = {
            vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
            vector4{ 0.5f, 0.25f, 0.5f, 1.0f },
        };
        INVARIANT(m_batches_selection.size() == colours.size());
        for (std::size_t  i = 0ULL; i != colours.size(); ++i)
        {
            if (qtgl::make_current(m_batches_selection.at(i), draw_state))
            {
                qtgl::render_batch(
                    m_batches_selection.at(i),
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(vector3_zero(), quaternion_identity()),
                    colours.at(i)
                );
                draw_state = m_batches_selection.at(i).get_draw_state();
            }
        }
    }
}

void  simulator::initiate_network_construction(std::string const&  experiment_name, bool const  pause_applies_to_network_open)
{
    TMPROF_BLOCK();

    ASSUMPTION(is_network_being_constructed() == false);

    if (g_create_experiment_thread.joinable())
        g_create_experiment_thread.join();

    m_network.reset();
    g_constructed_network.reset();
    g_experiment_name = experiment_name;
    g_num_construction_steps = 0ULL;
    g_pre_construction_pause_state = paused();
    g_apply_pause_to_construction = pause_applies_to_network_open;

    g_network_construction_state = NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP;
    g_create_experiment_thread = std::thread(&create_experiment_worker);
}

bool  simulator::is_network_being_constructed() const
{
    return g_network_construction_state != NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION;
}

std::string  simulator::get_constructed_network_progress_text() const
{
    switch (g_network_construction_state)
    {
    case NETWORK_CONSTRUCTION_STATE::NOT_IN_CONSTRUCTION:
        return "ERROR: not in construction";
    case NETWORK_CONSTRUCTION_STATE::PERFORMING_INITIALISATION_STEP:
        return "performing step ...";
    case NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP:
        return to_string(g_constructed_network->get_state());
    default:
        UNREACHABLE();
    }
}

natural_64_bit  simulator::get_num_network_construction_steps() const
{
    return (g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP) ?
                g_num_construction_steps :
                0ULL;
}

void  simulator::acquire_constructed_network()
{
    m_network = g_constructed_network;
    m_experiment_name = g_experiment_name;
    g_constructed_network.reset();
    g_experiment_name.clear();
    g_num_construction_steps = 0ULL;
}

void  simulator::destroy_network()
{
    m_network.reset();
    m_experiment_name.clear();
    m_selected_object_stats.reset();
    m_batches_selection.clear();
}

natural_64_bit  simulator::get_network_update_id() const
{
    ASSUMPTION(has_network());
    return network()->update_id();
}

void  simulator::enable_usage_of_queues_in_update_of_ships_in_network(bool const  enable_state)
{
    ASSUMPTION(has_network());
    network()->enable_usage_of_queues_in_update_of_ships(enable_state);
}


void  simulator::render_constructed_network(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network() == nullptr);
    INVARIANT(is_network_being_constructed());
    INVARIANT(g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP);
    INVARIANT(g_constructed_network != nullptr);

    INVARIANT(g_constructed_network->get_state() == netlab::NETWORK_STATE::READY_FOR_MOVEMENT_AREA_CENTERS_MIGRATION_STEP);

    if (m_dbg_network_camera.is_enabled() && window_props().just_resized())
        m_dbg_network_camera.on_window_resized(window_props());

    std::vector< std::pair<vector3, vector3> >  clip_planes;
    qtgl::compute_clip_planes(
        *(m_dbg_network_camera.is_enabled() ? m_dbg_network_camera.get_camera() : m_camera),
        clip_planes
        );
    if (renders_only_chosen_layer() &&
        get_layer_index_of_chosen_layer_to_render() < g_constructed_network->properties()->layer_props().size())
    {
        auto const&  props = g_constructed_network->properties()->layer_props().at(get_layer_index_of_chosen_layer_to_render());
        clip_planes.push_back({ props.low_corner_of_ships(),vector3_unit_z() });
        clip_planes.push_back({ props.high_corner_of_ships(),-vector3_unit_z() });
    }

    render_spikers_of_constructed_network(
            matrix_from_world_to_camera,
            matrix_from_camera_to_clipspace,
            clip_planes,
            draw_state
            );

    if (m_dbg_network_camera.is_enabled())
        m_dbg_network_camera.render_camera_frustum(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    if (m_dbg_frustum_sector_enumeration.is_enabled())
    {
        if (m_dbg_frustum_sector_enumeration.is_invalidated() ||
            !m_dbg_network_camera.is_enabled() ||
            window_props().just_resized())
            m_dbg_frustum_sector_enumeration.enumerate(clip_planes, g_constructed_network->properties()->layer_props());
        m_dbg_frustum_sector_enumeration.render(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    }
    if (m_dbg_raycast_sector_enumeration.is_enabled())
        m_dbg_raycast_sector_enumeration.render(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    if (m_dbg_draw_movement_areas.is_enabled())
    {
        if (m_dbg_draw_movement_areas.is_invalidated() ||
            !m_dbg_network_camera.is_enabled() ||
            window_props().just_resized())
            m_dbg_draw_movement_areas.collect_visible_areas(clip_planes, g_constructed_network);
        m_dbg_draw_movement_areas.render(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    }
}

void  simulator::render_spikers_of_constructed_network(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        std::vector< std::pair<vector3, vector3> > const&  clip_planes,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(g_network_construction_state == NETWORK_CONSTRUCTION_STATE::PERFORMING_IDLE_BETWEEN_INITIALISATION_STEP);
    INVARIANT(g_constructed_network != nullptr);

    if (qtgl::make_current(m_batch_spiker, draw_state))
    {
        if (m_batch_spiker_bbox.empty())
        {
            INVARIANT(m_batch_spiker_bsphere.empty());
            qtgl::spatial_boundary const  boundary = m_batch_spiker.get_buffers_binding().get_boundary();

            m_batch_spiker_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                             vector4(0.5f, 0.5f, 0.5f, 1.0f), qtgl::FOG_TYPE::NONE,
                                                             "/netviewer/spiker_bbox");
            m_batch_spiker_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,vector4(0.5f, 0.5f, 0.5f, 1.0f),
                                                                   qtgl::FOG_TYPE::NONE,"/netviewer/spiker_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch const  batch_spiker,
                        matrix44 const&  matrix_from_world_to_camera,
                        matrix44 const&  matrix_from_camera_to_clipspace,
                        vector3 const&  spiker_position
                        )
            {
                qtgl::render_batch(
                    batch_spiker,
                    matrix_from_world_to_camera,
                    matrix_from_camera_to_clipspace,
                    angeo::coordinate_system(spiker_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_spiker_positions(
                    g_constructed_network->properties()->layer_props(),
                    clip_planes,
                    std::bind(
                        &local::callback,
                        m_batch_spiker,
                        std::cref(matrix_from_world_to_camera),
                        std::cref(matrix_from_camera_to_clipspace),
                        std::placeholders::_1)
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_spiker.get_draw_state();
    }
}


void  simulator::set_camera_speed(float_32_bit const  speed)
{
    ASSUMPTION(speed > 0.001f);
    m_free_fly_config.at(0UL).set_action_value(-speed);
    m_free_fly_config.at(1UL).set_action_value(speed);
    m_free_fly_config.at(2UL).set_action_value(-speed);
    m_free_fly_config.at(3UL).set_action_value(speed);
    m_free_fly_config.at(4UL).set_action_value(-speed);
    m_free_fly_config.at(5UL).set_action_value(speed);
}

void  simulator::on_look_at_selected()
{
    if (!m_selected_object_stats.operator bool())
        return;

    vector3  pos;
    {
        netlab::network_layer_props const&  props =
                network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
        if (std::dynamic_pointer_cast<netlab::tracked_spiker_stats>(m_selected_object_stats) != nullptr)
        {
            netlab::sector_coordinate_type  x, y, c;
            props.spiker_sector_coordinates(m_selected_object_stats->indices().object_index(), x, y, c);
            pos = props.spiker_sector_centre(x, y, c);
        }
        else if (std::dynamic_pointer_cast<netlab::tracked_dock_stats>(m_selected_object_stats) != nullptr)
        {
            netlab::sector_coordinate_type  x, y, c;
            props.dock_sector_coordinates(m_selected_object_stats->indices().object_index(), x, y, c);
            pos = props.dock_sector_centre(x, y, c);
        }
        else if (std::dynamic_pointer_cast<netlab::tracked_ship_stats>(m_selected_object_stats) != nullptr)
            pos = network()->get_layer_of_ships(m_selected_object_stats->indices().layer_index())
                            .position(m_selected_object_stats->indices().object_index());
        else
            return;
    }

    qtgl::look_at(*m_camera->coordinate_system(),pos,2.5f);
}

std::string const&  simulator::get_experiment_name() const
{
    return is_network_being_constructed() ? g_experiment_name : m_experiment_name;
}

void simulator::set_desired_network_to_real_time_ratio(float_64_bit const  value)
{
    ASSUMPTION(value > 1e-5f);
    m_desired_network_to_real_time_ratio = value;
    m_spent_real_time = spent_network_time() / desired_network_to_real_time_ratio();
}


std::string  simulator::get_network_info_text() const
{
    if (!network().operator bool())
        return "No network is loaded.";

    std::ostringstream  ostr;

    netlab::network_props const&  props = *network()->properties();

    natural_64_bit  total_memory_spikers = 0ULL;
    natural_64_bit  total_memory_docks = 0ULL;
    natural_64_bit  total_memory_ships = 0ULL;
    natural_64_bit  total_memory_movement_area_centers = 0ULL;
    natural_64_bit  total_memory_index_of_ships_in_sectors = 0ULL;
    natural_64_bit  total_max_memory_of_update_queues_of_ships =
            network()->max_size_of_update_queue_of_ships() * sizeof(netlab::network::element_type_in_update_queue_of_ships);
    for (netlab::layer_index_type layer_index = 0U; layer_index != props.layer_props().size(); ++layer_index)
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(layer_index);

        total_memory_spikers += layer_props.num_spikers() * (
                network()->get_layer_of_spikers(layer_index).num_bytes_per_spiker() +
                network()->get_layer_of_spikers(layer_index).num_extra_bytes_per_spiker()
                );
        total_memory_docks += layer_props.num_docks() * (
                network()->get_layer_of_docks(layer_index).num_bytes_per_dock() +
                network()->get_layer_of_docks(layer_index).num_extra_bytes_per_dock()
                );
        total_memory_ships += layer_props.num_ships() * (
                network()->get_layer_of_ships(layer_index).num_bytes_per_ship() +
                network()->get_layer_of_ships(layer_index).num_extra_bytes_per_ship()
                );
        total_memory_movement_area_centers += layer_props.num_spikers() * sizeof(vector3);
        total_memory_index_of_ships_in_sectors += layer_props.num_docks() *
                                                  3ULL * sizeof(netlab::compressed_layer_and_object_indices);

    }
    natural_64_bit  total_memory =
            total_memory_spikers +
            total_memory_docks +
            total_memory_ships +
            total_memory_movement_area_centers +
            total_memory_index_of_ships_in_sectors +
            total_max_memory_of_update_queues_of_ships
            ;

    std::vector< std::vector<natural_64_bit> >  counts_of_area_centers(props.layer_props().size(),
        std::vector<natural_64_bit>(props.layer_props().size()));
    for (netlab::layer_index_type layer_index = 0U; layer_index != props.layer_props().size(); ++layer_index)
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(layer_index);
        for (netlab::object_index_type spiker_index = 0ULL; spiker_index != layer_props.num_spikers(); ++spiker_index)
        {
            netlab::layer_index_type const  center_layer_index =
                props.find_layer_index(network()->get_layer_of_spikers(layer_index).get_movement_area_center(spiker_index)(2));

            ++counts_of_area_centers.at(layer_index).at(center_layer_index);
        }
    }

    ostr << "Experiment: " << get_experiment_name() << "\n"
            "Description: " << netexp::experiment_factory::instance().get_experiment_description(get_experiment_name()) << "\n\n"
            "Network properties:\n"
            "  num spikers: " << props.num_spikers() << "\n"
            "  num docks: " << props.num_docks() << "\n"
            "  num ships: " << props.num_ships() << "\n"
            "  total memory size: " << total_memory << "B\n"
            "  memory size of spikers: " << total_memory_spikers << "B (~"
                                         << percentage_string(total_memory_spikers,total_memory)
                                         << "%)\n"
            "  memory size of docks: " << total_memory_docks << "B (~"
                                       << percentage_string(total_memory_docks,total_memory)
                                       << "%)\n"
            "  memory size of ships: " << total_memory_ships << "B (~"
                                       << percentage_string(total_memory_ships,total_memory)
                                       << "%)\n"
            "  memory size of movement area centers: " << total_memory_movement_area_centers << "B (~"
                                                       << percentage_string(total_memory_movement_area_centers,total_memory)
                                                       << "%)\n"
            "  memory size of index of ships in sectors: " << total_memory_index_of_ships_in_sectors << "B (~"
                                                           << percentage_string(total_memory_index_of_ships_in_sectors,total_memory)
                                                           << "%)\n"
            "  max memory size of update queues of ships: " << total_max_memory_of_update_queues_of_ships << "B (~"
                                                           << percentage_string(total_max_memory_of_update_queues_of_ships,total_memory)
                                                           << "%)\n"
            "  simulation time step: " << props.update_time_step_in_seconds() << "s\n"
            "  max connection distance: " << props.max_connection_distance_in_meters() << "m\n"
            "  spiking potential magnitude: " << props.spiking_potential_magnitude() << "\n"
            "  mini spiking potential magnitude: " << props.mini_spiking_potential_magnitude() << "\n"
            "  average mini spiking period: " << props.average_mini_spiking_period_in_seconds() << "s\n"
            "  number of mini spikes generated per simulation step: " << props.num_mini_spikes_to_generate_per_simulation_step() << "\n"
            "  max num treads to use: " << props.num_threads_to_use() << "\n"
            "  number of layers: " << props.layer_props().size() << "\n"
         ;

    for (netlab::layer_index_type layer_index = 0U; layer_index != props.layer_props().size(); ++layer_index)
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(layer_index);

        ostr << "\n  layer[" << (int)layer_index << "]:\n"

             << "    num spikers: " << layer_props.num_spikers() << "\n"
             << "    excitatory: " << std::boolalpha << layer_props.are_spikers_excitatory() << "\n"
             << "    num spikers along axes [xyc]: "
                        << layer_props.num_spikers_along_x_axis() << ", "
                        << layer_props.num_spikers_along_y_axis() << ", "
                        << layer_props.num_spikers_along_c_axis() << "\n"
             << "    distance of spikers along axes [xyc]: "
                        << layer_props.distance_of_spikers_along_x_axis_in_meters() << "m, "
                        << layer_props.distance_of_spikers_along_y_axis_in_meters() << "m, "
                        << layer_props.distance_of_spikers_along_c_axis_in_meters() << "m\n"
             << "    low corner of spikers: [ "
                        << layer_props.low_corner_of_spikers()(0) << "m, "
                        << layer_props.low_corner_of_spikers()(1) << "m, "
                        << layer_props.low_corner_of_spikers()(2) << "m ]\n"
             << "    high corner of spikers: [ "
                        << layer_props.high_corner_of_spikers()(0) << "m, "
                        << layer_props.high_corner_of_spikers()(1) << "m, "
                        << layer_props.high_corner_of_spikers()(2) << "m ]\n\n"

             << "    num docks: " << layer_props.num_docks() << "\n"
             << "    num docks per spiker: " << layer_props.num_docks_per_spiker() << "\n"
             << "    num docks along axes [xyc]: "
                        << layer_props.num_docks_along_x_axis() << ", "
                        << layer_props.num_docks_along_y_axis() << ", "
                        << layer_props.num_docks_along_c_axis() << "\n"
             << "    num docks per spiker along axes [xyc]: "
                        << layer_props.num_docks_along_x_axis_per_spiker() << ", "
                        << layer_props.num_docks_along_y_axis_per_spiker() << ", "
                        << layer_props.num_docks_along_c_axis_per_spiker() << "\n"
             << "    distance of docks [same for all axes]: " << layer_props.distance_of_docks_in_meters() << "m\n"
             << "    low corner of docks: [ "
                        << layer_props.low_corner_of_docks()(0) << "m, "
                        << layer_props.low_corner_of_docks()(1) << "m, "
                        << layer_props.low_corner_of_docks()(2) << "m ]\n"
             << "    high corner of docks: [ "
                        << layer_props.high_corner_of_docks()(0) << "m, "
                        << layer_props.high_corner_of_docks()(1) << "m, "
                        << layer_props.high_corner_of_docks()(2) << "m ]\n\n"

             << "    num ships: " << layer_props.num_ships() << "\n"
             << "    num ships per spiker: " << layer_props.num_ships_per_spiker() << "\n"
             << "    low corner of ships: [ "
                        << layer_props.low_corner_of_ships()(0) << "m, "
                        << layer_props.low_corner_of_ships()(1) << "m, "
                        << layer_props.low_corner_of_ships()(2) << "m ]\n"
             << "    high corner of ships: [ "
                        << layer_props.high_corner_of_ships()(0) << "m, "
                        << layer_props.high_corner_of_ships()(1) << "m, "
                        << layer_props.high_corner_of_ships()(2) << "m ]\n\n"
             ;

        ostr << "    sizes of ship movement areas [xyc]:\n";
        for (netlab::layer_index_type  other_layer_index = 0U; other_layer_index != props.layer_props().size(); ++other_layer_index)
            ostr << "      layer[" << (int)other_layer_index << "]: "
                 << layer_props.size_of_ship_movement_area_along_x_axis_in_meters(other_layer_index) << "m, "
                 << layer_props.size_of_ship_movement_area_along_y_axis_in_meters(other_layer_index) << "m, "
                 << layer_props.size_of_ship_movement_area_along_c_axis_in_meters(other_layer_index) << "m\n"
                 ;
        ostr << "\n    speed limits of ships [min,max]:\n";
        for (netlab::layer_index_type  other_layer_index = 0U; other_layer_index != props.layer_props().size(); ++other_layer_index)
            ostr << "      layer[" << (int)other_layer_index << "]: "
                 << layer_props.min_speed_of_ship_in_meters_per_second(other_layer_index) << "m/s, "
                 << layer_props.max_speed_of_ship_in_meters_per_second(other_layer_index) << "m/s\n"
                 ;

        ostr << "\n    Counts area centers and ships from this layer [centers / ships]:\n";
        {
            natural_64_bit  total_centers = 0ULL;
            natural_64_bit  total_ships = 0ULL;
            natural_64_bit  total_inhibitory_centers = 0ULL;
            natural_64_bit  total_inhibitory_ships = 0ULL;
            for (netlab::layer_index_type i = 0U; i != props.layer_props().size(); ++i)
            {
                ostr << "        to layer[" << (natural_32_bit)i << "]: "
                     << counts_of_area_centers.at(layer_index).at(i)
                     << " / "
                     << counts_of_area_centers.at(layer_index).at(i) * props.layer_props().at(layer_index).num_ships_per_spiker()
                     << (props.layer_props().at(i).are_spikers_excitatory() ? "" : " (inhibitory)")
                     << "\n"
                     ;
                total_centers += counts_of_area_centers.at(layer_index).at(i);
                total_ships += counts_of_area_centers.at(layer_index).at(i) * props.layer_props().at(layer_index).num_ships_per_spiker();
                if (!props.layer_props().at(i).are_spikers_excitatory())
                {
                    total_inhibitory_centers += counts_of_area_centers.at(layer_index).at(i);
                    total_inhibitory_ships += counts_of_area_centers.at(layer_index).at(i) *
                                              props.layer_props().at(layer_index).num_ships_per_spiker();
                }
            }
            ostr << "        TOTAL: " << total_centers << " / " << total_ships << "\n";
            ostr << "        TOTAL to excitatory: " << total_centers - total_inhibitory_centers << " / " 
                                                    << total_ships - total_inhibitory_ships << "\n";
            ostr << "        TOTAL to inhibitory: " << total_inhibitory_centers << " / " << total_inhibitory_ships << "\n";
        }

        ostr << "\n    Counts area centers and ships into this layer [centers / ships]:\n";
        {
            natural_64_bit  total_centers = 0ULL;
            natural_64_bit  total_ships = 0ULL;
            natural_64_bit  total_inhibitory_centers = 0ULL;
            natural_64_bit  total_inhibitory_ships = 0ULL;
            for (netlab::layer_index_type i = 0U; i != props.layer_props().size(); ++i)
            {
                ostr << "        from layer[" << (natural_32_bit)i << "]: "
                     << counts_of_area_centers.at(i).at(layer_index)
                     << " / "
                     << counts_of_area_centers.at(i).at(layer_index) * props.layer_props().at(i).num_ships_per_spiker()
                     << (props.layer_props().at(i).are_spikers_excitatory() ? "" : " (inhibitory)")
                     << "\n"
                     ;
                total_centers += counts_of_area_centers.at(i).at(layer_index);
                total_ships += counts_of_area_centers.at(i).at(layer_index) * props.layer_props().at(i).num_ships_per_spiker();
                if (!props.layer_props().at(i).are_spikers_excitatory())
                {
                    total_inhibitory_centers += counts_of_area_centers.at(i).at(layer_index);
                    total_inhibitory_ships += counts_of_area_centers.at(i).at(layer_index) *
                                              props.layer_props().at(i).num_ships_per_spiker();
                }
            }
            ostr << "        TOTAL: " << total_centers << " / " << total_ships
                 << " (>= " << layer_props.num_docks() << ")"
                 << "\n";
            ostr << "        TOTAL from excitatory: " << total_centers - total_inhibitory_centers << " / " 
                                                      << total_ships - total_inhibitory_ships
                 << " (>= " << (natural_64_bit)(layer_props.num_docks() * 3.0/4.0) << ")"
                 << "\n";
            ostr << "        TOTAL from inhibitory: " << total_inhibitory_centers << " / " << total_inhibitory_ships
                 << " (>= " << layer_props.num_docks() - (natural_64_bit)(layer_props.num_docks() * 3.0/4.0) << ")"
                 << "\n";
        }

        ostr << "\n    Densities of ships in this layer:\n";
        {
            netlab::statistics_of_densities_of_ships_in_layers const&  densities = network()->densities_of_ships();
            ostr << "        Ideal density  : " << std::fixed << std::setprecision(3)
                                                << densities.ideal_densities().at(layer_index) << "\n"
                 << "        Minimal density: " << std::fixed << std::setprecision(3)
                                                << densities.minimal_densities().at(layer_index) << "\n"
                 << "        Average density: " << std::fixed << std::setprecision(3)
                                                << densities.average_densities().at(layer_index) << "\n"
                 << "        Maximal density: " << std::fixed << std::setprecision(3)
                                                << densities.maximal_densities().at(layer_index) << "\n"
                 ;

            float_32_bit const  max_density = 2.0f * densities.ideal_densities().at(layer_index);
            for (natural_64_bit j = 0ULL, n = netlab::distribution_of_spikers_by_density_of_ships().size(); j != n; ++j)
            {
                ostr << "        Num spikers in density range ["
                     << std::fixed << std::setprecision(3)
                     << (float_32_bit)j * (max_density / (float_32_bit)n)
                     << ","
                     ;
                if (j + 1ULL != n || densities.maximal_densities().at(layer_index) < max_density)
                    ostr << std::fixed << std::setprecision(3)
                         << (float_32_bit)(j + 1ULL) * (max_density / (float_32_bit)n)
                         << "): "
                         ;
                else
                    ostr << std::fixed << std::setprecision(3)
                         << densities.maximal_densities().at(layer_index)
                         << "]: "
                         ;
                ostr << densities.distribution_of_spikers_by_densities_of_ships().at(layer_index).at(j)
                     << " (~ "
                     << std::fixed << std::setprecision(1)
                     << 100.0f * (float_32_bit)densities.distribution_of_spikers_by_densities_of_ships().at(layer_index).at(j)
                               / (float_32_bit)layer_props.num_spikers()
                     << "%)"
                     << "\n"
                     ;
            }
        }
    }

    return ostr.str();
}


std::string  simulator::get_selected_info_text() const
{
    if (!m_selected_object_stats.operator bool())
        return "No network object is selected.";

    std::ostringstream  ostr;

    netlab::network_props const&  props = *network()->properties();

    if (auto ptr = std::dynamic_pointer_cast<netlab::tracked_spiker_stats>(m_selected_object_stats))
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(ptr->indices().layer_index());

        //netlab::spiker const&  spiker_ref = network()->get_spiker(ptr->indices().layer_index(),ptr->indices().object_index());

        netlab::sector_coordinate_type  x,y,c;
        layer_props.spiker_sector_coordinates(ptr->indices().object_index(),x,y,c);

        vector3 const  sector_center = layer_props.spiker_sector_centre(x,y,c);

        vector3 const&  area_center =
                network()->get_layer_of_spikers(ptr->indices().layer_index()).get_movement_area_center(ptr->indices().object_index());

        netlab::layer_index_type const  area_layer_index = props.find_layer_index(area_center(2));

        netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

        netlab::object_index_type const  ships_begin_index =
                layer_props.ships_begin_index_of_spiker(ptr->indices().object_index());

        netlab::object_index_type  num_connected_ships = 0UL;
        for (netlab::object_index_type  i = 0UL; i < layer_props.num_ships_per_spiker(); ++i)
        {
            vector3 const&  ship_pos = network()->get_layer_of_ships(ptr->indices().layer_index()).position(ships_begin_index + i);

            netlab::sector_coordinate_type  dock_x,dock_y,dock_c;
            area_layer_props.dock_sector_coordinates(ship_pos,dock_x,dock_y,dock_c);

            vector3 const  dock_pos = area_layer_props.dock_sector_centre(dock_x,dock_y,dock_c);

            if (netlab::are_ship_and_dock_connected(ship_pos,dock_pos,props.max_connection_distance_in_meters()))
                ++num_connected_ships;
        }

        netlab::sector_coordinate_type  dock_low_x,dock_low_y,dock_low_c;
        layer_props.dock_low_sector_coordinates_from_spiker_sector_coordinates(x,y,c,dock_low_x,dock_low_y,dock_low_c);

        netlab::object_index_type  num_connected_docks = 0UL;
        for (netlab::sector_coordinate_type  i = 0UL; i != layer_props.num_docks_along_x_axis_per_spiker(); ++i)
            for (netlab::sector_coordinate_type  j = 0UL; j != layer_props.num_docks_along_y_axis_per_spiker(); ++j)
                for (netlab::sector_coordinate_type  k = 0UL; k != layer_props.num_docks_along_c_axis_per_spiker(); ++k)
                {
                    vector3 const  dock_pos = layer_props.dock_sector_centre(dock_low_x+i,dock_low_y+j,dock_low_c+k);
                    for (auto  indices :
                         network()->get_indices_of_ships_in_dock_sector(ptr->indices().layer_index(),
                                                                        layer_props.dock_sector_index(dock_low_x+i,
                                                                                                      dock_low_y+j,
                                                                                                      dock_low_c+k)))
                    {
                        vector3 const&  ship_pos = network()->get_layer_of_ships(indices.layer_index()).position(indices.object_index());
                        if (netlab::are_ship_and_dock_connected(ship_pos,dock_pos,props.max_connection_distance_in_meters()))
                            ++num_connected_docks;
                    }
                }

        ptr->get_info_text(ostr);
        network()->get_layer_of_spikers(ptr->indices().layer_index()).get_info_text(ptr->indices().object_index(),ostr);

        ostr << "Position: [ " << sector_center(0) << "m, "
                               << sector_center(1) << "m, "
                               << sector_center(2) << "m ]\n"
             << "Sector coords: [" << x << ", " << y << ", " << c << "]\n"
             << "Layer index of movement area: " << (natural_64_bit)area_layer_index << "\n"
             << "Number of connected docks: " << (natural_64_bit)num_connected_docks << "\n"
             << "Number of disconnected docks: "
                    << (natural_64_bit)layer_props.num_docks_per_spiker() - num_connected_docks << "\n"
             << "Number of connected ships: " << (natural_64_bit)num_connected_ships << "\n"
             << "Number of disconnected ships: "
                    << (natural_64_bit)layer_props.num_ships_per_spiker() - num_connected_ships << "\n"
             << "Excitatory: " << std::boolalpha << layer_props.are_spikers_excitatory() << "\n"
             ;
    }
    else if (auto ptr = std::dynamic_pointer_cast<netlab::tracked_dock_stats>(m_selected_object_stats))
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(ptr->indices().layer_index());

        netlab::sector_coordinate_type  x,y,c;
        layer_props.dock_sector_coordinates(ptr->indices().object_index(),x,y,c);

        vector3 const  sector_center = layer_props.dock_sector_centre(x,y,c);

        std::size_t const  num_ships_in_dock_sector =
                network()->get_indices_of_ships_in_dock_sector(ptr->indices().layer_index(),
                                                               layer_props.dock_sector_index(x,y,c)).size();

        bool  is_connected = false;
        bool  is_occupied = false;
        float_32_bit  dist_to_nearest_ship = std::numeric_limits<float_32_bit>::max();
        for (auto  indices : network()->get_indices_of_ships_in_dock_sector(ptr->indices().layer_index(),
                                                                            layer_props.dock_sector_index(x,y,c)))
        {
            vector3 const&  ship_pos = network()->get_layer_of_ships(indices.layer_index()).position(indices.object_index());
            if (netlab::are_ship_and_dock_connected(ship_pos,sector_center,props.max_connection_distance_in_meters()))
                is_connected = true;
            vector3 const&  ship_velocity = network()->get_layer_of_ships(indices.layer_index()).velocity(indices.object_index());
            if (layer_props.ship_controller_ptr()->is_ship_docked(ship_pos,ship_velocity,ptr->indices().layer_index(),props))
                is_occupied = true;
            float_32_bit const  dist = length(ship_pos - sector_center);
            if (dist < dist_to_nearest_ship)
                dist_to_nearest_ship = dist;
        }

        ptr->get_info_text(ostr);
        network()->get_layer_of_docks(ptr->indices().layer_index()).get_info_text(ptr->indices().object_index(),ostr);

        ostr << "Position: [ " << sector_center(0) << "m, "
                               << sector_center(1) << "m, "
                               << sector_center(2) << "m ]\n"
             << "Sector coords: [" << x << ", " << y << ", " << c << "]\n"
             << "Num ships in the sector: " << num_ships_in_dock_sector << "\n"
             ;
        if (num_ships_in_dock_sector != 0ULL)
            ostr  << "Distance to nearest ship in the sector: " << dist_to_nearest_ship << "m\n";
        ostr << "Connected: " << std::boolalpha << is_connected << "\n"
             << "Occupied by a ship: " << std::boolalpha << is_occupied << "\n"
             ;
    }
    else if (auto ptr = std::dynamic_pointer_cast<netlab::tracked_ship_stats>(m_selected_object_stats))
    {
        netlab::network_layer_props const&  layer_props = props.layer_props().at(ptr->indices().layer_index());

        vector3 const&  ship_pos = network()->get_layer_of_ships(ptr->indices().layer_index()).position(ptr->indices().object_index());
        vector3 const&  ship_velocity = network()->get_layer_of_ships(ptr->indices().layer_index()).velocity(ptr->indices().object_index());

        netlab::object_index_type const  spiker_index = layer_props.spiker_index_from_ship_index(ptr->indices().object_index());

        vector3 const&  area_center = network()->get_layer_of_spikers(ptr->indices().layer_index()).get_movement_area_center(spiker_index);

        netlab::layer_index_type const  area_layer_index = network()->properties()->find_layer_index(area_center(2));

        netlab::network_layer_props const&  area_layer_props = props.layer_props().at(area_layer_index);

        netlab::sector_coordinate_type  dock_x,dock_y,dock_c;
        area_layer_props.dock_sector_coordinates(ship_pos,dock_x,dock_y,dock_c);

        vector3 const  dock_pos = area_layer_props.dock_sector_centre(dock_x,dock_y,dock_c);

        std::size_t const  num_ships_in_dock_sector =
                network()->get_indices_of_ships_in_dock_sector(area_layer_index,
                                                               area_layer_props.dock_sector_index(dock_x,dock_y,dock_c)
                                                               ).size();

            
        bool const  is_connected = netlab::are_ship_and_dock_connected(ship_pos,dock_pos,props.max_connection_distance_in_meters());
        bool const  is_docked = area_layer_props.ship_controller_ptr()->is_ship_docked(ship_pos,ship_velocity,area_layer_index,props);

        ptr->get_info_text(ostr);
        network()->get_layer_of_ships(ptr->indices().layer_index()).get_info_text(ptr->indices().object_index(),ostr);

        ostr << "Layer index of movement area: " << (natural_64_bit)area_layer_index << "\n"
             << "Center of movement area: [ " << area_center(0) << "m, "
                                              << area_center(1) << "m, "
                                              << area_center(2) << "m ]\n"
             << "Nearest dock position: [ " << dock_pos(0) << "m, "
                                            << dock_pos(1) << "m, "
                                            << dock_pos(2) << "m ]\n"
             << "Nearest dock sector coords: [" << dock_x << ", " << dock_y << ", " << dock_c << "]\n"
             << "Distance to the nearest dock: " << length(dock_pos - ship_pos) << "m\n"
             << "Num ships in nearest dock sector: " << num_ships_in_dock_sector << "\n"
             << "Connected: " << std::boolalpha << is_connected << "\n"
             << "Docked: " << std::boolalpha << is_docked << "\n"
             << "Excitatory: " << std::boolalpha << layer_props.are_spikers_excitatory() << "\n"
             ;
    }
    else
        ostr << "Selected object of UNKNOWN kind!";

    return ostr.str();
}


std::string  simulator::get_network_performance_text() const
{
    if (!network().operator bool())
        return "No network is loaded.";

    netlab::network_props const&  props = *network()->properties();

    std::ostringstream  ostr;

    if (network()->is_update_queue_of_ships_used())
    {
        ostr << "Update queue of ships: ";
        if (network()->is_update_queue_of_ships_overloaded())
            ostr << "OVERLOADED! (=> all ships are simulated)";
        else
            ostr << network()->size_of_update_queue_of_ships() << " / "
                    << network()->max_size_of_update_queue_of_ships() << " (~"
                    << percentage_string(network()->size_of_update_queue_of_ships(),
                                        network()->max_size_of_update_queue_of_ships())
                    << "%)"
                    ;
        ostr << "\n";
    }
    else
        ostr << "Update queue of ships is NOT used.\n"
                "  => updating all ships in each simulation step.\n";

    return ostr.str();
}


void  simulator::on_select_owner_spiker()
{
    if (!m_selected_object_stats.operator bool())
        return;
    if (get_experiment_name().empty())
        return;

    netlab::layer_index_type  layer_index = m_selected_object_stats->indices().layer_index();
    netlab::object_index_type  object_index;
    {
        netlab::network_layer_props const&  props = network()->properties()->layer_props().at(layer_index);

        if (std::dynamic_pointer_cast<netlab::tracked_dock_stats>(m_selected_object_stats) != nullptr)
        {
            netlab::sector_coordinate_type  x, y, c;
            props.dock_sector_coordinates(m_selected_object_stats->indices().object_index(), x, y, c);
            netlab::sector_coordinate_type  spiker_x, spiker_y, spiker_c;
            props.spiker_sector_coordinates_from_dock_sector_coordinates(x,y,c,spiker_x, spiker_y, spiker_c);
            object_index = props.spiker_sector_index(spiker_x, spiker_y, spiker_c);
        }
        else if (std::dynamic_pointer_cast<netlab::tracked_ship_stats>(m_selected_object_stats) != nullptr)
            object_index = props.spiker_index_from_ship_index(m_selected_object_stats->indices().object_index());
        else
            return;
    }

    m_selected_object_stats =
        netexp::experiment_factory::instance().create_tracked_spiker_stats(get_experiment_name(), {layer_index,object_index});
    m_batches_selection.clear();
}


bool  simulator::set_layer_index_of_chosen_layer_to_render(netlab::layer_index_type const  layer_index)
{
    if (network().operator bool() && network()->properties()->layer_props().size() <= layer_index)
    {
        call_listeners(simulator_notifications::on_wrong_index_of_layer_to_render());
        return false;
    }
    m_layer_index_of_chosen_layer_to_render = layer_index;
    return true;
}
