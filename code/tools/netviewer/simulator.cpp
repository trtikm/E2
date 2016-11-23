#include <netviewer/simulator.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <netviewer/program_options.hpp>
#include <netviewer/draw_utils.hpp>
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
std::atomic<bool>  g_is_network_being_constructed = false;
std::thread  g_create_experiment_thread;

void  create_experiment_worker()
{
    TMPROF_BLOCK();
    g_constructed_network = netexp::experiment_factory::instance().instance().create_network(g_experiment_name);
    g_is_network_being_constructed = false;
}


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
                    { 0.4f, 0.4f, 0.4f },
                    { 0.4f, 0.4f, 0.4f },
                    { 0.5f, 0.5f, 0.5f },
                    { 0.5f, 0.5f, 0.5f },
                    { 1.0f, 0.0f, 0.0f },
                    { 0.0f, 1.0f, 0.0f },
                    { 0.0f, 0.0f, 1.0f },
                    10U,
                    true,
                    get_program_options()->dataRoot()
                    )
            }

    , m_network()
    , m_experiment_name()

    , m_paused(paused)
    , m_do_single_step(false)
    , m_spent_real_time(0.0)
    , m_spent_network_time(0.0)
    , m_num_network_updates(0UL)
    , m_desired_network_to_real_time_ratio(desired_network_to_real_time_ratio)

    , m_selected_object_stats()

    , m_batch_spiker{ qtgl::batch::create(canonical_path(
            boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/models/neuron/body.txt"
            )) }
    , m_batch_dock{ qtgl::batch::create(canonical_path(
            boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/models/input_spot/input_spot.txt"
            )) }
    , m_batch_ship{ qtgl::batch::create(canonical_path(
            boost::filesystem::path{get_program_options()->dataRoot()} / "shared/gfx/models/output_terminal/output_terminal.txt"
            )) }

    , m_batch_spiker_bbox{}
    , m_batch_dock_bbox{}
    , m_batch_ship_bbox{}

    , m_batch_spiker_bsphere{}
    , m_batch_dock_bsphere{}
    , m_batch_ship_bsphere{}

    , m_dbg_network_camera(m_camera->far_plane())
    , m_dbg_frustum_sector_enumeration()
    , m_dbg_raycast_sector_enumeration()

//    , m_selected_cell_input_spot_lines()
//    , m_selected_cell_output_terminal_lines()
{
    TMPROF_BLOCK();
    set_clear_color(initial_clear_colour);
}

simulator::~simulator()
{
    TMPROF_BLOCK();
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

        if (!is_network_being_constructed() && g_constructed_network.operator bool())
        {
            m_network = g_constructed_network;
            m_experiment_name = g_experiment_name;
            g_constructed_network.reset();
            g_experiment_name.clear();

            m_spent_real_time = 0.0;
            m_spent_network_time = 0.0;
            m_num_network_updates = 0UL;
        }

        if (network().operator bool())
        {
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

            if (!paused())
                update_network(seconds_from_previous_call);

            update_selection_of_network_objects(seconds_from_previous_call);
        }
    }

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  view_projection_matrix;
    qtgl::view_projection_matrix(*m_camera,view_projection_matrix);

    qtgl::make_current(*m_batch_grid->draw_state());
    if (qtgl::make_current(*m_batch_grid, *m_batch_grid->draw_state()))
    {
        INVARIANT(m_batch_grid->shaders_binding().operator bool());
        render_batch(*m_batch_grid,view_projection_matrix);
    }

    if (network().operator bool())
        render_network(view_projection_matrix,m_batch_grid->draw_state());

//    {
//        if (qtgl::make_current(*m_batch_spiker, *draw_state))
//        {
//            INVARIANT(m_batch_spiker->shaders_binding().operator bool());

//            for (cell::pos_map::const_iterator it = nenet()->cells().cbegin(); it != nenet()->cells().cend(); ++it)
//            {
//                matrix44  world_transformation;
//                quaternion const  orientation =
//                    (it == m_selected_cell) ? angle_axis_to_quaternion(m_selected_rot_angle,vector3_unit_z()) :
//                                              quaternion_identity() ;
//                qtgl::transformation_matrix(qtgl::coordinate_system(it->first, orientation), world_transformation);
//                matrix44 const  transform_matrix = view_projection_matrix * world_transformation;

//                vector4 const  diffuse_colour(
//                        it->second.spiking_potential() > 0.0f ? 1.0f : 0.0f,
//                        it->second.spiking_potential() < 0.0f ? 1.0f : 0.0f,
//                        0.0f,
//                        std::min(std::abs(it->second.spiking_potential()),1.0f)
//                        );

//                for (qtgl::vertex_shader_uniform_symbolic_name const uniform : m_batch_spiker->symbolic_names_of_used_uniforms())
//                    switch (uniform)
//                    {
//                    case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
//                        qtgl::set_uniform_variable(m_batch_spiker->shaders_binding()->uniform_variable_accessor(), uniform, diffuse_colour);
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
//                        qtgl::set_uniform_variable(m_batch_spiker->shaders_binding()->uniform_variable_accessor(),uniform,transform_matrix);
//                        break;
//                    }

//                qtgl::draw();

//                if (it == m_selected_cell)
//                {
//                    if (!m_selected_cell_input_spot_lines.operator bool())
//                    {
//                        std::vector< std::pair<vector3, vector3> >  lines;
//                        for (auto const iit : it->second.input_spots())
//                            lines.push_back({ it->first,iit->first });
//                        m_selected_cell_input_spot_lines = qtgl::create_lines3d(lines, vector3{ 1.0f,1.0f,0.0f });
//                    }
//                    //if (!m_selected_cell_output_terminal_lines.operator bool())
//                    {
//                        std::vector< std::pair<vector3, vector3> >  lines;
//                        for (auto const iit : it->second.output_terminals())
//                            lines.push_back({ it->first,iit->pos() });
//                        m_selected_cell_output_terminal_lines = qtgl::create_lines3d(lines, vector3{ 1.0f,0.0f,0.5f });
//                    }
//                }
//            }
//            draw_state = m_batch_spiker->draw_state();
//        }
//    }

//    {
//        if (qtgl::make_current(*m_batch_dock, *draw_state))
//        {
//            INVARIANT(m_batch_dock->shaders_binding().operator bool());

//            for (input_spot::pos_map::const_iterator it = nenet()->input_spots().cbegin(); it != nenet()->input_spots().cend(); ++it)
//            {
//                matrix44  world_transformation;
//                quaternion const  orientation =
//                    (it == m_selected_input_spot) ? angle_axis_to_quaternion(m_selected_rot_angle, vector3_unit_z()) :
//                                                    quaternion_identity();
//                qtgl::transformation_matrix(qtgl::coordinate_system(it->first, orientation), world_transformation);
//                matrix44 const  transform_matrix = view_projection_matrix * world_transformation;

//                for (qtgl::vertex_shader_uniform_symbolic_name const uniform : m_batch_dock->symbolic_names_of_used_uniforms())
//                    switch (uniform)
//                    {
//                    case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
//                        qtgl::set_uniform_variable(m_batch_dock->shaders_binding()->uniform_variable_accessor(), uniform, vector4(1.0f, 1.0f, 1.0f, 0.0f));
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
//                        qtgl::set_uniform_variable(m_batch_dock->shaders_binding()->uniform_variable_accessor(), uniform, transform_matrix);
//                        break;
//                    }

//                qtgl::draw();

//                if (it == m_selected_input_spot)
//                {
//                    if (!m_selected_cell_input_spot_lines.operator bool())
//                        m_selected_cell_input_spot_lines =
//                            qtgl::create_lines3d(
//                                { { get_position_of_selected(), it->second.cell()->first } },
//                                vector3{ 1.0f,1.0f,0.0f }
//                                );
//                }
//            }
//            draw_state = m_batch_dock->draw_state();
//        }
//    }

//    {
//        if (qtgl::make_current(*m_batch_ship, *draw_state))
//        {
//            INVARIANT(m_batch_ship->shaders_binding().operator bool());

//            for (output_terminal const&  oterm : nenet()->output_terminals())
//            {
//                matrix44  world_transformation;
//                quaternion const  orientation =
//                    (&oterm == m_selected_output_terminal) ? angle_axis_to_quaternion(m_selected_rot_angle, vector3_unit_z()) :
//                                                             quaternion_identity();
//                qtgl::transformation_matrix(qtgl::coordinate_system(oterm.pos(), orientation), world_transformation);
//                matrix44 const  transform_matrix = view_projection_matrix * world_transformation;

//                for (qtgl::vertex_shader_uniform_symbolic_name const uniform : m_batch_ship->symbolic_names_of_used_uniforms())
//                    switch (uniform)
//                    {
//                    case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
//                        qtgl::set_uniform_variable(m_batch_ship->shaders_binding()->uniform_variable_accessor(), uniform, vector4(1.0f, 1.0f, 1.0f, 0.0f));
//                        break;
//                    case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
//                        qtgl::set_uniform_variable(m_batch_ship->shaders_binding()->uniform_variable_accessor(), uniform, transform_matrix);
//                        break;
//                    }

//                qtgl::draw();

//                if (&oterm == m_selected_output_terminal)
//                {
//                    m_selected_cell_output_terminal_lines =
//                        qtgl::create_lines3d(
//                            { { oterm.pos(), oterm.cell()->first } },
//                            vector3{ 1.0f,0.0f,0.5f }
//                            );
//                }
//            }
//            draw_state = m_batch_dock->draw_state();
//        }
//    }

//    if (m_selected_cell_input_spot_lines.operator bool())
//    {
//        if (qtgl::make_current(*m_selected_cell_input_spot_lines, *draw_state))
//        {
//            INVARIANT(m_selected_cell_input_spot_lines->shaders_binding().operator bool());

//            matrix44 const  transform_matrix = view_projection_matrix;
//            for (qtgl::vertex_shader_uniform_symbolic_name const uniform : m_selected_cell_input_spot_lines->symbolic_names_of_used_uniforms())
//                switch (uniform)
//                {
//                case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
//                    break;
//                case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
//                    break;
//                case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
//                    qtgl::set_uniform_variable(m_selected_cell_input_spot_lines->shaders_binding()->uniform_variable_accessor(), uniform, transform_matrix);
//                    break;
//                }

//            qtgl::draw();

//            draw_state = m_selected_cell_input_spot_lines->draw_state();
//        }
//    }

//    if (m_selected_cell_output_terminal_lines.operator bool())
//    {
//        if (qtgl::make_current(*m_selected_cell_output_terminal_lines, *draw_state))
//        {
//            INVARIANT(m_selected_cell_output_terminal_lines->shaders_binding().operator bool());

//            matrix44 const  transform_matrix = view_projection_matrix;
//            for (qtgl::vertex_shader_uniform_symbolic_name const uniform : m_selected_cell_output_terminal_lines->symbolic_names_of_used_uniforms())
//                switch (uniform)
//                {
//                case qtgl::vertex_shader_uniform_symbolic_name::COLOUR_ALPHA:
//                    break;
//                case qtgl::vertex_shader_uniform_symbolic_name::DIFFUSE_COLOUR:
//                    break;
//                case qtgl::vertex_shader_uniform_symbolic_name::TRANSFORM_MATRIX_TRANSPOSED:
//                    qtgl::set_uniform_variable(m_selected_cell_output_terminal_lines->shaders_binding()->uniform_variable_accessor(), uniform, transform_matrix);
//                    break;
//                }

//            qtgl::draw();

//            draw_state = m_selected_cell_output_terminal_lines->draw_state();
//        }
//    }

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
    for ( ; num_iterations != 0ULL; --num_iterations)
    {
        network()->update(
            true,true,true,
            nullptr
//                        m_selected_input_spot_stats.operator bool() ? m_selected_input_spot_stats.get() : nullptr,
//                        m_selected_output_terminal_stats.operator bool() ? m_selected_output_terminal_stats.get() : nullptr,
//                        m_selected_cell_stats.operator bool() ? m_selected_cell_stats.get() : nullptr
            );

        if (m_do_single_step)
        {
            INVARIANT(!paused());
            m_paused = true;
            call_listeners(simulator_notifications::paused());
            m_do_single_step = false;
            break;
        }

        if (std::chrono::duration<float_64_bit>(std::chrono::high_resolution_clock::now() - update_start_time).count() > 1.0 / 30.0)
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

            if (m_batch_spiker->buffers_binding().operator bool() &&
                m_batch_spiker->buffers_binding()->find_vertex_buffer_properties().operator bool())
            {
                qtgl::spatial_boundary const  boundary =
                    m_batch_spiker->buffers_binding()->find_vertex_buffer_properties()->boundary();

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

            if (m_batch_dock->buffers_binding().operator bool() &&
                m_batch_dock->buffers_binding()->find_vertex_buffer_properties().operator bool())
            {
                qtgl::spatial_boundary const  boundary =
                    m_batch_dock->buffers_binding()->find_vertex_buffer_properties()->boundary();

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

            if (m_batch_ship->buffers_binding().operator bool() &&
                m_batch_ship->buffers_binding()->find_vertex_buffer_properties().operator bool())
            {
                qtgl::spatial_boundary const  boundary =
                    m_batch_ship->buffers_binding()->find_vertex_buffer_properties()->boundary();

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
                        netexp::experiment_factory::instance().create_tracked_spiker_stats(m_experiment_name, object_indices) :
                object_kind == 2U ?
                        netexp::experiment_factory::instance().create_tracked_dock_stats(m_experiment_name, object_indices) :
                object_kind == 3U ?
                        netexp::experiment_factory::instance().create_tracked_ship_stats(m_experiment_name, object_indices) :
                        std::shared_ptr<netlab::tracked_network_object_stats>{}
                        ;
        }

        //call_listeners(simulator_notifications::selection_changed());

        if (m_dbg_raycast_sector_enumeration.is_enabled())
            m_dbg_raycast_sector_enumeration.enumerate(ray_begin,ray_end,network()->properties()->layer_props());
    }
}


void  simulator::render_network(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr  draw_state)
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

    render_network_spikers(view_projection_matrix,clip_planes,draw_state);
    render_network_docks(view_projection_matrix,clip_planes,draw_state);
    render_network_ships(view_projection_matrix,clip_planes,draw_state);

    render_selected_network_object(view_projection_matrix,draw_state);

    if (m_dbg_network_camera.is_enabled())
        m_dbg_network_camera.render_camera_frustum(view_projection_matrix,draw_state);
    if (m_dbg_frustum_sector_enumeration.is_enabled())
    {
        if (m_dbg_frustum_sector_enumeration.is_invalidated() ||
                !m_dbg_network_camera.is_enabled() ||
                window_props().just_resized())
            m_dbg_frustum_sector_enumeration.enumerate(clip_planes,network()->properties()->layer_props());
        m_dbg_frustum_sector_enumeration.render(view_projection_matrix,draw_state);
    }
    if (m_dbg_raycast_sector_enumeration.is_enabled())
        m_dbg_raycast_sector_enumeration.render(view_projection_matrix,draw_state);
}


void  simulator::render_network_spikers(
        matrix44 const&  view_projection_matrix,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state_ptr&  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(*m_batch_spiker, *draw_state))
    {
        INVARIANT(m_batch_spiker->shaders_binding().operator bool());

        if (!m_batch_spiker_bbox.operator bool())
        {
            INVARIANT(!m_batch_spiker_bsphere.operator bool());
            qtgl::spatial_boundary const  boundary =
                    m_batch_spiker->buffers_binding()->find_vertex_buffer_properties()->boundary();

            m_batch_spiker_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                             get_program_options()->dataRoot(),"/netviewer/spiker_bbox");
            m_batch_spiker_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,
                                                                   get_program_options()->dataRoot(),"/netviewer/spiker_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch_ptr const  batch_spiker,
                        matrix44 const&  view_projection_matrix,
                        vector3 const&  spiker_position
                        )
            {
                render_batch(
                    *batch_spiker,
                    view_projection_matrix,
                    angeo::coordinate_system(spiker_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_spiker_positions(
                    network()->properties()->layer_props(),
                    clip_planes,
                    std::bind(&local::callback,m_batch_spiker,std::cref(view_projection_matrix),std::placeholders::_1)
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_spiker->draw_state();
    }


//    if (qtgl::make_current(*m_batch_spiker, *draw_state))
//    {
//        INVARIANT(m_batch_spiker->shaders_binding().operator bool());

//        if (!m_batch_spiker_bbox.operator bool())
//        {
//            INVARIANT(!m_batch_spiker_bsphere.operator bool());
//            qtgl::spatial_boundary const  boundary =
//                    m_batch_spiker->buffers_binding()->find_vertex_buffer_properties()->boundary();

//            m_batch_spiker_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),"/netviewer/spiker");
//            m_batch_spiker_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,"/netviewer/spiker");
//        }

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_spikers(); ++object_index)
//            {
//                vector3  spiker_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.spiker_sector_coordinates(object_index, x,y,c);
//                    spiker_position = layer_props.spiker_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_spiker,
//                    view_projection_matrix,
//                    angeo::coordinate_system(
//                        spiker_position,
//                        //(it == m_selected_cell) ? angle_axis_to_quaternion(m_selected_rot_angle,vector3_unit_z()) :
//                                                    quaternion_identity()
//                        )
////                        ,
////                    vector4(
////                        it->second.spiking_potential() > 0.0f ? 1.0f : 0.0f,
////                        it->second.spiking_potential() < 0.0f ? 1.0f : 0.0f,
////                        0.0f,
////                        1.0f //std::min(std::abs(it->second.spiking_potential()),1.0f)
////                        )
//                    );
//            }

//        }
//        draw_state = m_batch_spiker->draw_state();
//    }

//    if (m_batch_spiker_bbox.operator bool() && qtgl::make_current(*m_batch_spiker_bbox, *draw_state))
//    {
//        INVARIANT(m_batch_spiker_bbox->shaders_binding().operator bool());

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_spikers(); ++object_index)
//            {
//                vector3  spiker_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.spiker_sector_coordinates(object_index, x,y,c);
//                    spiker_position = layer_props.spiker_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_spiker_bbox,
//                    view_projection_matrix,
//                    angeo::coordinate_system(spiker_position,quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//            }

//        }
//        draw_state = m_batch_spiker_bbox->draw_state();
//    }

//    if (m_batch_spiker_bsphere.operator bool() && qtgl::make_current(*m_batch_spiker_bsphere, *draw_state))
//    {
//        INVARIANT(m_batch_spiker_bsphere->shaders_binding().operator bool());

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_spikers(); ++object_index)
//            {
//                vector3  spiker_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.spiker_sector_coordinates(object_index, x,y,c);
//                    spiker_position = layer_props.spiker_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_spiker_bsphere,
//                    view_projection_matrix,
//                    angeo::coordinate_system(spiker_position,quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//            }

//        }
//        draw_state = m_batch_spiker_bsphere->draw_state();
//    }

//    if (qtgl::make_current(*m_batch_basis, *draw_state))
//    {
//        INVARIANT(m_batch_basis->shaders_binding().operator bool());
//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_spikers(); ++object_index)
//            {
//                vector3  spiker_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.spiker_sector_coordinates(object_index, x,y,c);
//                    spiker_position = layer_props.spiker_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_basis,
//                    view_projection_matrix,
//                    angeo::coordinate_system(spiker_position,quaternion_identity())
//                    );
//            }

//        }
//        draw_state = m_batch_basis->draw_state();
//    }
}


void  simulator::render_network_docks(
        matrix44 const&  view_projection_matrix,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state_ptr&  draw_state)
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(*m_batch_dock, *draw_state))
    {
        INVARIANT(m_batch_dock->shaders_binding().operator bool());

        if (!m_batch_dock_bbox.operator bool())
        {
            INVARIANT(!m_batch_dock_bsphere.operator bool());
            qtgl::spatial_boundary const  boundary =
                    m_batch_dock->buffers_binding()->find_vertex_buffer_properties()->boundary();

            m_batch_dock_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                           get_program_options()->dataRoot(),"/netviewer/dock_bbox");
            m_batch_dock_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,
                                                                 get_program_options()->dataRoot(),"/netviewer/dock_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch_ptr const  batch_dock,
                        matrix44 const&  view_projection_matrix,
                        vector3 const&  dock_position
                        )
            {
                render_batch(
                    *batch_dock,
                    view_projection_matrix,
                    angeo::coordinate_system(dock_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_dock_positions(
                    network()->properties()->layer_props(),
                    clip_planes,
                    std::bind(&local::callback,m_batch_dock,std::cref(view_projection_matrix),std::placeholders::_1)
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_dock->draw_state();
    }

//    if (qtgl::make_current(*m_batch_dock, *draw_state))
//    {
//        INVARIANT(m_batch_dock->shaders_binding().operator bool());

//        if (!m_batch_dock_bbox.operator bool())
//        {
//            INVARIANT(!m_batch_dock_bsphere.operator bool());
//            qtgl::spatial_boundary const  boundary =
//                    m_batch_dock->buffers_binding()->find_vertex_buffer_properties()->boundary();

//            m_batch_dock_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),"/netviewer/dock");
//            m_batch_dock_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,"/netviewer/dock");
//        }

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_docks(); ++object_index)
//            {
//                vector3  dock_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.dock_sector_coordinates(object_index, x,y,c);
//                    dock_position = layer_props.dock_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_dock,
//                    view_projection_matrix,
//                    angeo::coordinate_system(
//                        dock_position,
//                        //(it == m_selected_cell) ? angle_axis_to_quaternion(m_selected_rot_angle,vector3_unit_z()) :
//                                                    quaternion_identity()
//                        )
////                        ,
////                    vector4(
////                        it->second.spiking_potential() > 0.0f ? 1.0f : 0.0f,
////                        it->second.spiking_potential() < 0.0f ? 1.0f : 0.0f,
////                        0.0f,
////                        1.0f //std::min(std::abs(it->second.spiking_potential()),1.0f)
////                        )
//                    );
//            }

//        }
//        draw_state = m_batch_dock->draw_state();
//    }

//    if (m_batch_dock_bbox.operator bool() && qtgl::make_current(*m_batch_dock_bbox, *draw_state))
//    {
//        INVARIANT(m_batch_dock_bbox->shaders_binding().operator bool());

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_docks(); ++object_index)
//            {
//                vector3  dock_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.dock_sector_coordinates(object_index, x,y,c);
//                    dock_position = layer_props.dock_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_dock_bbox,
//                    view_projection_matrix,
//                    angeo::coordinate_system(dock_position,quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//            }

//        }
//        draw_state = m_batch_dock_bbox->draw_state();
//    }

//    if (m_batch_dock_bsphere.operator bool() && qtgl::make_current(*m_batch_dock_bsphere, *draw_state))
//    {
//        INVARIANT(m_batch_dock_bsphere->shaders_binding().operator bool());

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_docks(); ++object_index)
//            {
//                vector3  dock_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.dock_sector_coordinates(object_index, x,y,c);
//                    dock_position = layer_props.dock_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_dock_bsphere,
//                    view_projection_matrix,
//                    angeo::coordinate_system(dock_position,quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//            }

//        }
//        draw_state = m_batch_dock_bsphere->draw_state();
//    }

//    if (qtgl::make_current(*m_batch_basis, *draw_state))
//    {
//        INVARIANT(m_batch_basis->shaders_binding().operator bool());
//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_docks(); ++object_index)
//            {
//                vector3  dock_position;
//                {
//                    netlab::sector_coordinate_type  x,y,c;
//                    layer_props.dock_sector_coordinates(object_index, x,y,c);
//                    dock_position = layer_props.dock_sector_centre(x,y,c);
//                }
//                render_batch(
//                    *m_batch_basis,
//                    view_projection_matrix,
//                    angeo::coordinate_system(dock_position,quaternion_identity())
//                    );
//            }

//        }
//        draw_state = m_batch_basis->draw_state();
//    }
}


void  simulator::render_network_ships(
        matrix44 const&  view_projection_matrix,
        std::vector< std::pair<vector3,vector3> > const& clip_planes,
        qtgl::draw_state_ptr&  draw_state
        )
{
    TMPROF_BLOCK();

    INVARIANT(network().operator bool());

    if (qtgl::make_current(*m_batch_ship, *draw_state))
    {
        INVARIANT(m_batch_ship->shaders_binding().operator bool());

        if (!m_batch_ship_bbox.operator bool())
        {
            INVARIANT(!m_batch_ship_bsphere.operator bool());
            qtgl::spatial_boundary const  boundary =
                    m_batch_ship->buffers_binding()->find_vertex_buffer_properties()->boundary();

            m_batch_ship_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),
                                                           get_program_options()->dataRoot(),"/netviewer/ship_bbox");
            m_batch_ship_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,
                                                                 get_program_options()->dataRoot(),"/netviewer/ship_bsphere");
        }

        struct  local
        {
            static bool  callback(
                        qtgl::batch_ptr const  batch_ship,
                        matrix44 const&  view_projection_matrix,
                        vector3 const&  ship_position
                        )
            {
                render_batch(
                    *batch_ship,
                    view_projection_matrix,
                    angeo::coordinate_system(ship_position,quaternion_identity())
                    );
                return true;
            }
        };

        natural_64_bit const  num_rendered = netview::enumerate_ship_positions(
                    *network(),
                    clip_planes,
                    std::bind(&local::callback,m_batch_ship,std::cref(view_projection_matrix),std::placeholders::_1)
                    );

        if (num_rendered != 0UL)
            draw_state = m_batch_ship->draw_state();
    }


//    if (qtgl::make_current(*m_batch_ship, *draw_state))
//    {
//        INVARIANT(m_batch_ship->shaders_binding().operator bool());

//        if (!m_batch_ship_bbox.operator bool())
//        {
//            INVARIANT(!m_batch_ship_bsphere.operator bool());
//            qtgl::spatial_boundary const  boundary =
//                    m_batch_ship->buffers_binding()->find_vertex_buffer_properties()->boundary();

//            m_batch_ship_bbox = qtgl::create_wireframe_box(boundary.lo_corner(),boundary.hi_corner(),"/netviewer/ship");
//            m_batch_ship_bsphere = qtgl::create_wireframe_sphere(boundary.radius(),5U,"/netviewer/ship");
//        }

//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_ships(); ++object_index)
//                render_batch(
//                    *m_batch_ship,
//                    view_projection_matrix,
//                    angeo::coordinate_system(
//                        network()->get_ship(layer_index,object_index).position(),
//                        //(it == m_selected_cell) ? angle_axis_to_quaternion(m_selected_rot_angle,vector3_unit_z()) :
//                                                    quaternion_identity()
//                        )
//                    );
//        }
//        draw_state = m_batch_ship->draw_state();
//    }

//    if (m_batch_ship_bbox.operator bool() && qtgl::make_current(*m_batch_ship_bbox, *draw_state))
//    {
//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_ships(); ++object_index)
//                render_batch(
//                    *m_batch_ship_bbox,
//                    view_projection_matrix,
//                    angeo::coordinate_system(network()->get_ship(layer_index,object_index).position(),quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//        }
//        draw_state = m_batch_ship_bbox->draw_state();
//    }

//    if (m_batch_ship_bsphere.operator bool() && qtgl::make_current(*m_batch_ship_bsphere, *draw_state))
//    {
//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_ships(); ++object_index)
//                render_batch(
//                    *m_batch_ship_bsphere,
//                    view_projection_matrix,
//                    angeo::coordinate_system(network()->get_ship(layer_index,object_index).position(),quaternion_identity()),
//                    vector4(1.0f,1.0f,1.0f,1.0f)
//                    );
//        }
//        draw_state = m_batch_ship_bsphere->draw_state();
//    }

//    if (qtgl::make_current(*m_batch_basis, *draw_state))
//    {
//        INVARIANT(m_batch_basis->shaders_binding().operator bool());
//        for (netlab::layer_index_type  layer_index = 0U; layer_index != network()->properties()->layer_props().size(); ++layer_index)
//        {
//            netlab::network_layer_props const&  layer_props = network()->properties()->layer_props().at(layer_index);
//            for (netlab::object_index_type  object_index = 0UL; object_index != layer_props.num_ships(); ++object_index)
//                render_batch(
//                    *m_batch_basis,
//                    view_projection_matrix,
//                    angeo::coordinate_system(network()->get_ship(layer_index,object_index).position(),quaternion_identity())
//                    );
//        }
//        draw_state = m_batch_basis->draw_state();
//    }
}


void  simulator::render_selected_network_object(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr&  draw_state)
{
    if (!network().operator bool())
        return;

    if (std::dynamic_pointer_cast<netlab::tracked_spiker_stats>(m_selected_object_stats) != nullptr &&
        m_batch_spiker_bsphere.operator bool())
    {
        if (qtgl::make_current(*m_batch_spiker_bsphere, *draw_state))
        {
            INVARIANT(m_batch_spiker_bsphere->shaders_binding().operator bool());

            netlab::network_layer_props const&  props =
                network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
            netlab::sector_coordinate_type  x, y, c;
            props.spiker_sector_coordinates(m_selected_object_stats->indices().object_index(), x, y, c);
            vector3 const&  pos = props.spiker_sector_centre(x, y, c);
            render_batch(
                *m_batch_spiker_bsphere,
                view_projection_matrix,
                angeo::coordinate_system(pos, quaternion_identity()),
                { 1.0f, 1.0f, 1.0f, 1.0f }
            );
            draw_state = m_batch_spiker_bsphere->draw_state();
        }
    }
    else if (std::dynamic_pointer_cast<netlab::tracked_dock_stats>(m_selected_object_stats) != nullptr &&
        m_batch_dock_bsphere.operator bool())
    {
        if (qtgl::make_current(*m_batch_dock_bsphere, *draw_state))
        {
            INVARIANT(m_batch_dock_bsphere->shaders_binding().operator bool());

            netlab::network_layer_props const&  props =
                network()->properties()->layer_props().at(m_selected_object_stats->indices().layer_index());
            netlab::sector_coordinate_type  x,y,c;
            props.dock_sector_coordinates(m_selected_object_stats->indices().object_index(),x,y,c);
            vector3 const&  pos = props.dock_sector_centre(x,y,c);
            render_batch(
                *m_batch_dock_bsphere,
                view_projection_matrix,
                angeo::coordinate_system(pos,quaternion_identity()),
                { 1.0f, 1.0f, 1.0f, 1.0f }
                );
            draw_state = m_batch_dock_bsphere->draw_state();
        }
    }
    else if (std::dynamic_pointer_cast<netlab::tracked_ship_stats>(m_selected_object_stats) != nullptr &&
        m_batch_ship_bsphere.operator bool())
    {
        if (qtgl::make_current(*m_batch_ship_bsphere, *draw_state))
        {
            INVARIANT(m_batch_ship_bsphere->shaders_binding().operator bool());

            vector3 const&  pos =
                network()->get_ship(m_selected_object_stats->indices().layer_index(),
                                    m_selected_object_stats->indices().object_index())
                        .position();
            render_batch(
                *m_batch_ship_bsphere,
                view_projection_matrix,
                angeo::coordinate_system(pos,quaternion_identity()),
                { 1.0f, 1.0f, 1.0f, 1.0f }
                );
            draw_state = m_batch_ship_bsphere->draw_state();
        }
    }
}


void  simulator::initiate_network_construction(std::string const&  experiment_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(g_is_network_being_constructed == false);

    if (g_create_experiment_thread.joinable())
        g_create_experiment_thread.join();

    m_network.reset();
    g_constructed_network.reset();
    g_experiment_name = experiment_name;

    g_is_network_being_constructed = true;
    g_create_experiment_thread = std::thread(&create_experiment_worker);
}

bool  simulator::is_network_being_constructed() const
{
    return g_is_network_being_constructed;
}


void simulator::set_desired_network_to_real_time_ratio(float_64_bit const  value)
{
    ASSUMPTION(value > 1e-5f);
    m_desired_network_to_real_time_ratio = value;
    m_spent_real_time = spent_network_time() / desired_network_to_real_time_ratio();
}


//vector3 const&  simulator::get_position_of_selected() const
//{
//    ASSUMPTION(is_selected_something());
//    if (is_selected_cell())
//        return m_selected_cell->first;
//    else if (is_selected_input_spot())
//        return m_selected_input_spot->first;
//    else
//        return m_selected_output_terminal->pos();
//}

//cell const&  simulator::get_selected_cell() const
//{
//    ASSUMPTION(is_selected_cell());
//    return m_selected_cell->second;
//}

//input_spot const&  simulator::get_selected_input_spot() const
//{
//    ASSUMPTION(is_selected_input_spot());
//    return m_selected_input_spot->second;
//}

//output_terminal const&  simulator::get_selected_output_terminal() const
//{
//    ASSUMPTION(is_selected_output_terminal());
//    return *m_selected_output_terminal;
//}

//std::string  simulator::get_selected_info_text() const
//{
//    if (!is_selected_something())
//        return "";

//    std::ostringstream  ostr;

//    ostr << "position: [ " << std::fixed << get_position_of_selected()(0)
//                          << ", "
//                          << std::fixed << get_position_of_selected()(1)
//                          << ", "
//                          << std::fixed << get_position_of_selected()(2)
//                          << " ]\n"
//         ;

//    if (is_selected_cell())
//    {
//        ostr << "spiking potential: " << std::fixed << get_selected_cell().spiking_potential() << "\n"
//             << "the last update: " << get_selected_cell().last_update() << "\n"
//             << "is excitatory: " << std::boolalpha << get_selected_cell().is_excitatory() << "\n"
//             ;
//        INVARIANT(m_selected_cell_stats.operator bool());
//        ostr << "observed from update: " << m_selected_cell_stats->start_update() << "\n"
//             << "last mini spike update: " << m_selected_cell_stats->last_mini_spike_update() << "\n"
//             << "number of mini spikes: " << m_selected_cell_stats->num_mini_spikes() << "\n"
//             << "average mini spikes rate: " << std::fixed
//                    << m_selected_cell_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
//             << "last spike update: " << m_selected_cell_stats->last_spike_update() << "\n"
//             << "number of spikes: " << m_selected_cell_stats->num_spikes() << "\n"
//             << "average spikes rate: " << std::fixed
//                    << m_selected_cell_stats->average_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
//             << "last presynaptic potential update: " << m_selected_cell_stats->last_presynaptic_potential_update() << "\n"
//             << "number of presynaptic potentials: " << m_selected_cell_stats->num_presynaptic_potentials() << "\n"
//             << "average presynaptic potentials rate: " << std::fixed
//                    << m_selected_cell_stats->average_presynaptic_potentials_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
//             ;
//    }
//    else if (is_selected_input_spot())
//    {
//        ostr << "";
//        INVARIANT(m_selected_input_spot_stats.operator bool());
//        ostr << "observed from update: " << m_selected_input_spot_stats->start_update() << "\n"
//             << "last mini spike update: " << m_selected_input_spot_stats->last_mini_spike_update() << "\n"
//             << "number of mini spikes: " << m_selected_input_spot_stats->num_mini_spikes() << "\n"
//             << "average mini spikes rate: " << std::fixed
//                    << m_selected_input_spot_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
//             ;
//    }
//    else if (is_selected_output_terminal())
//    {
//        ostr << "velocity: [ " << std::fixed << get_selected_output_terminal().velocity()(0)
//                               << ", "
//                               << std::fixed << get_selected_output_terminal().velocity()(1)
//                               << ", "
//                               << std::fixed << get_selected_output_terminal().velocity()(2)
//                               << " ]\n"
//             << "synaptic weight: " << std::fixed << get_selected_output_terminal().synaptic_weight() << "\n"
//             ;
//        INVARIANT(m_selected_output_terminal_stats.operator bool());
//        ostr << "observed from update: " << m_selected_output_terminal_stats->start_update() << "\n"
//             << "last mini spike update: " << m_selected_output_terminal_stats->last_mini_spike_update() << "\n"
//             << "number of mini spikes: " << m_selected_output_terminal_stats->num_mini_spikes() << "\n"
//             << "average mini spikes rate: " << std::fixed
//                    << m_selected_output_terminal_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
//             ;
//    }
//    return ostr.str();
//}
