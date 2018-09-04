#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <qtgl/camera_utils.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/random.hpp>
#include <utility/canonical_path.hpp>
#include <sstream>
#include <iomanip>
#include <string>
#include <algorithm>
#include <cmath>

#include <iostream>


simulator::simulator(vector3 const&  initial_clear_colour, bool const  paused, nenet::params_ptr const  params,
                     float_64_bit const  desired_number_of_simulated_seconds_per_real_time_second)
    : qtgl::real_time_simulator()
    , m_nenet(std::make_shared<::nenet>(
            vector3{-30.0f, -30.0f, 0.0f}, vector3{ 30.0f, 30.0f, 40.0f },
            3,3,2,
            10,
            params
            ))
    , m_spent_real_time(0.0)
    , m_paused(paused)
    , m_do_single_step(false)
    , m_desired_number_of_simulated_seconds_per_real_time_second(desired_number_of_simulated_seconds_per_real_time_second)

    , m_selected_cell(m_nenet->cells().cend())
    , m_selected_input_spot(m_nenet->input_spots().cend())
    , m_selected_output_terminal(nullptr)
    , m_selected_rot_angle(0.0f)

    , m_selected_cell_stats()
    , m_selected_input_spot_stats()
    , m_selected_output_terminal_stats()

    , m_camera(
            qtgl::camera_perspective::create(
                    angeo::coordinate_system::create(
                            vector3(6.56402016f, 6.53800011f, 3.27975011f),
                            quaternion(0.298171997f, 0.228851005f, 0.564207971f, 0.735112011f)
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

    , m_effects_config(
            qtgl::effects_config::light_types{},
            qtgl::effects_config::lighting_data_types{
                { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            qtgl::effects_config::shader_output_types{ qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT },
            qtgl::FOG_TYPE::NONE
            )

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
                    "../data"
                    )
            }
    , m_batch_cell{qtgl::batch(canonical_path("../data/shared/gfx/models/neuron/body.txt"), m_effects_config)}
    , m_batch_input_spot{ qtgl::batch(canonical_path("../data/shared/gfx/models/input_spot/input_spot.txt"), m_effects_config) }
    , m_batch_output_terminal{ qtgl::batch(canonical_path("../data/shared/gfx/models/output_terminal/output_terminal.txt"), m_effects_config) }
    , m_selected_cell_input_spot_lines()
    , m_selected_cell_output_terminal_lines()
{
    TMPROF_BLOCK();
    set_clear_color(initial_clear_colour);
}

simulator::~simulator()
{
    TMPROF_BLOCK();
}

void simulator::next_round(float_64_bit const  seconds_from_previous_call,
                           bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    /////////////////////////////////////////////////////////////////////////////////////
    // Next round computation
    /////////////////////////////////////////////////////////////////////////////////////

    if (!is_this_pure_redraw_request)
    {
        if (keyboard_props().was_just_released(qtgl::KEY_SPACE()))
        {
            if (paused())
            {
                m_paused = !m_paused;
                call_listeners(notifications::paused());
            }
            m_do_single_step = true;
        }
        
        if (!m_do_single_step && keyboard_props().was_just_released(qtgl::KEY_PAUSE()))
        {
            m_paused = !m_paused;
            call_listeners(notifications::paused());
        }

        if (!paused())
        {
            m_spent_real_time += seconds_from_previous_call;

            natural_64_bit  num_iterations = (natural_64_bit)
                std::max(
                    0.0,
                    std::round((desired_number_of_simulated_seconds_per_real_time_second() * spent_real_time() - spent_simulation_time())
                                    / nenet()->get_params()->update_time_step_in_seconds())
                    );
            std::chrono::high_resolution_clock::time_point const  update_start_time = std::chrono::high_resolution_clock::now();
            for ( ; num_iterations != 0ULL; --num_iterations)
            {            
                nenet()->update(
                    true,true,true,
                    m_selected_input_spot_stats.operator bool() ? m_selected_input_spot_stats.get() : nullptr,
                    m_selected_output_terminal_stats.operator bool() ? m_selected_output_terminal_stats.get() : nullptr,
                    m_selected_cell_stats.operator bool() ? m_selected_cell_stats.get() : nullptr
                    );

                if (m_do_single_step)
                {
                    INVARIANT(!paused());
                    m_paused = true;
                    call_listeners(notifications::paused());
                    m_do_single_step = false;
                    break;
                }

                if (std::chrono::duration<float_64_bit>(std::chrono::high_resolution_clock::now() - update_start_time).count() > 1.0 / 30.0)
                    break;
            }
        }

        if (mouse_props().was_just_released(qtgl::LEFT_MOUSE_BUTTON()))
        {
            m_selected_cell = m_nenet->cells().cend();
            m_selected_input_spot = m_nenet->input_spots().cend();
            m_selected_output_terminal = nullptr;

            m_selected_cell_stats.reset();
            m_selected_input_spot_stats.reset();
            m_selected_output_terminal_stats.reset();

            m_selected_rot_angle = 0.0f;

            m_selected_cell_input_spot_lines.release();
            m_selected_cell_output_terminal_lines.release();

            vector3  line_begin;
            qtgl::cursor_line_begin(*m_camera, { mouse_props().x() ,mouse_props().y() }, window_props(), line_begin);
            vector3 const  ray = line_begin - m_camera->coordinate_system()->origin();
            scalar  param = 1e30f;
            m_selected_cell = nenet()->find_closest_cell(m_camera->coordinate_system()->origin(),ray,0.75f,&param);
            {
                m_selected_input_spot = nenet()->find_closest_input_spot(m_camera->coordinate_system()->origin(), ray, 0.5f, &param);
                if (m_selected_input_spot != nenet()->input_spots().cend())
                    m_selected_cell = nenet()->cells().cend();//m_selected_input_spot->second.cell();
            }
            {
                auto const oit = nenet()->find_closest_output_terminal(m_camera->coordinate_system()->origin(), ray, 0.22f, &param);
                if (oit != nenet()->output_terminals_set().cend())
                {
                    ASSUMPTION(oit->second != nullptr);
                    m_selected_output_terminal = oit->second;
                    m_selected_cell = nenet()->cells().cend();//m_selected_outpu_terminal->second->cell();
                    m_selected_input_spot = nenet()->input_spots().cend();
                }
            }

            if (m_selected_cell != nenet()->cells().cend())
                m_selected_cell_stats = std::unique_ptr<stats_of_cell>(new stats_of_cell(m_selected_cell,nenet()->update_id()));
            if (m_selected_input_spot != nenet()->input_spots().cend())
                m_selected_input_spot_stats = std::unique_ptr<stats_of_input_spot>(new stats_of_input_spot(m_selected_input_spot, nenet()->update_id()));
            if (m_selected_output_terminal != nullptr)
                m_selected_output_terminal_stats = std::unique_ptr<stats_of_output_terminal>(new stats_of_output_terminal(m_selected_output_terminal, nenet()->update_id()));

            call_listeners(notifications::selection_changed());
        }

        if (is_selected_something())
        {
            m_selected_rot_angle += (2.0f * PI()) * seconds_from_previous_call;
            while (m_selected_rot_angle > 2.0f * PI())
                m_selected_rot_angle -= 2.0f * PI();
        }
    }

    /////////////////////////////////////////////////////////////////////////////////////
    // Rendering
    /////////////////////////////////////////////////////////////////////////////////////

    qtgl::adjust(*m_camera, window_props());
    auto const translated_rotated =
        qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                       seconds_from_previous_call, mouse_props(), keyboard_props());
    if (translated_rotated.first)
        call_listeners(notifications::camera_position_updated());
    if (translated_rotated.second)
        call_listeners(notifications::camera_orientation_updated());

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    matrix44  matrix_from_world_to_camera;
    m_camera->to_camera_space_matrix(matrix_from_world_to_camera);
    matrix44  matrix_from_camera_to_clipspace;
    m_camera->projection_matrix(matrix_from_camera_to_clipspace);

    qtgl::draw_state  draw_state = m_batch_grid.get_draw_state();
    qtgl::make_current(draw_state);

    {
        if (qtgl::make_current(m_batch_grid, draw_state))
        {
            for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_batch_grid.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                switch (uniform)
                {
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                    m_batch_grid.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_world_to_camera);
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                    m_batch_grid.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                    break;
                }

            qtgl::draw();
        }
        draw_state = m_batch_grid.get_draw_state();
    }

    {
        if (qtgl::make_current(m_batch_cell, draw_state))
        {
            for (cell::pos_map::const_iterator it = nenet()->cells().cbegin(); it != nenet()->cells().cend(); ++it)
            {
                matrix44  world_transformation;
                quaternion const  orientation =
                    (it == m_selected_cell) ? angle_axis_to_quaternion(m_selected_rot_angle,vector3_unit_z()) :
                                              quaternion_identity() ;
                angeo::from_base_matrix(angeo::coordinate_system(it->first, orientation), world_transformation);
                matrix44 const  transform_matrix = matrix_from_world_to_camera * world_transformation;

                vector4 const  diffuse_colour(
                        it->second.spiking_potential() > 0.0f ? 1.0f : 0.0f,
                        it->second.spiking_potential() < 0.0f ? 1.0f : 0.0f,
                        0.0f,
                        std::min(std::abs(it->second.spiking_potential()),1.0f)
                        );

                for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_batch_cell.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                    switch (uniform)
                    {
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                        m_batch_cell.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, diffuse_colour);
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                        m_batch_cell.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform,transform_matrix);
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                        m_batch_cell.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                        break;
                    }

                qtgl::draw();

                if (it == m_selected_cell)
                {
                    if (m_selected_cell_input_spot_lines.empty())
                    {
                        std::vector< std::pair<vector3, vector3> >  lines;
                        for (auto const iit : it->second.input_spots())
                            lines.push_back({ it->first,iit->first });
                        m_selected_cell_input_spot_lines =
                            qtgl::create_lines3d(lines, vector4{ 1.0f,1.0f,0.0f,1.0f },qtgl::FOG_TYPE::NONE,"../data");
                    }
                    if (m_selected_cell_output_terminal_lines.empty())
                    {
                        std::vector< std::pair<vector3, vector3> >  lines;
                        for (auto const iit : it->second.output_terminals())
                            lines.push_back({ it->first,iit->pos() });
                        m_selected_cell_output_terminal_lines = 
                            qtgl::create_lines3d(lines, vector4{ 1.0f,0.0f,0.5f,1.0f },qtgl::FOG_TYPE::NONE,"../data");
                    }
                }
            }
            draw_state = m_batch_cell.get_draw_state();
        }
    }

    {
        if (qtgl::make_current(m_batch_input_spot, draw_state))
        {
            for (input_spot::pos_map::const_iterator it = nenet()->input_spots().cbegin(); it != nenet()->input_spots().cend(); ++it)
            {
                matrix44  world_transformation;
                quaternion const  orientation =
                    (it == m_selected_input_spot) ? angle_axis_to_quaternion(m_selected_rot_angle, vector3_unit_z()) :
                                                    quaternion_identity();
                angeo::from_base_matrix(angeo::coordinate_system(it->first, orientation), world_transformation);
                matrix44 const  transform_matrix = matrix_from_world_to_camera * world_transformation;

                for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_batch_input_spot.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                    switch (uniform)
                    {
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                        m_batch_input_spot.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, vector4(1.0f, 1.0f, 1.0f, 1.0f));
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                        m_batch_input_spot.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, transform_matrix);
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                        m_batch_input_spot.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                        break;
                    }

                qtgl::draw();

                if (it == m_selected_input_spot)
                {
                    if (m_selected_cell_input_spot_lines.empty())
                        m_selected_cell_input_spot_lines =
                            qtgl::create_lines3d(
                                { { get_position_of_selected(), it->second.cell()->first } },
                                vector4{ 1.0f,1.0f,0.0f,1.0f },
                                qtgl::FOG_TYPE::NONE,
                                "../data"
                                );
                }
            }
            draw_state = m_batch_input_spot.get_draw_state();
        }
    }

    {
        if (qtgl::make_current(m_batch_output_terminal, draw_state))
        {
            for (output_terminal const&  oterm : nenet()->output_terminals())
            {
                matrix44  world_transformation;
                quaternion const  orientation =
                    (&oterm == m_selected_output_terminal) ? angle_axis_to_quaternion(m_selected_rot_angle, vector3_unit_z()) :
                                                             quaternion_identity();
                angeo::from_base_matrix(angeo::coordinate_system(oterm.pos(), orientation), world_transformation);
                matrix44 const  transform_matrix = matrix_from_world_to_camera * world_transformation;

                for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_batch_output_terminal.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                    switch (uniform)
                    {
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                        m_batch_output_terminal.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, vector4(1.0f, 1.0f, 1.0f, 0.0f));
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                        m_batch_output_terminal.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, transform_matrix);
                        break;
                    case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                        m_batch_output_terminal.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                        break;
                    }

                qtgl::draw();

                if (&oterm == m_selected_output_terminal)
                {
                    m_selected_cell_output_terminal_lines =
                        qtgl::create_lines3d(
                            { { oterm.pos(), oterm.cell()->first } },
                            vector4{ 1.0f,0.0f,0.5f,1.0f },
                            qtgl::FOG_TYPE::NONE,
                            "../data"
                            );
                }
            }
            draw_state = m_batch_input_spot.get_draw_state();
        }
    }

    if (!m_selected_cell_input_spot_lines.empty())
    {
        if (qtgl::make_current(m_selected_cell_input_spot_lines, draw_state))
        {
            for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_selected_cell_input_spot_lines.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                switch (uniform)
                {
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                    m_selected_cell_input_spot_lines.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_world_to_camera);
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                    m_selected_cell_input_spot_lines.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                    break;
                }

            qtgl::draw();

            draw_state = m_selected_cell_input_spot_lines.get_draw_state();
        }
    }

    if (!m_selected_cell_output_terminal_lines.empty())
    {
        if (qtgl::make_current(m_selected_cell_output_terminal_lines, draw_state))
        {
            for (qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME const uniform : m_selected_cell_output_terminal_lines.get_shaders_binding().get_vertex_shader().get_symbolic_names_of_used_uniforms())
                switch (uniform)
                {
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::DIFFUSE_COLOUR:
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_MODEL_TO_CAMERA:
                    m_selected_cell_output_terminal_lines.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_world_to_camera);
                    break;
                case qtgl::VERTEX_SHADER_UNIFORM_SYMBOLIC_NAME::MATRIX_FROM_CAMERA_TO_CLIPSPACE:
                    m_selected_cell_output_terminal_lines.get_shaders_binding().get_vertex_shader().set_uniform_variable(uniform, matrix_from_camera_to_clipspace);
                    break;
                }

            qtgl::draw();

            draw_state = m_selected_cell_output_terminal_lines.get_draw_state();
        }
    }

    qtgl::swap_buffers();
}

void simulator::set_desired_number_of_simulated_seconds_per_real_time_second(float_64_bit const  value)
{
    ASSUMPTION(value > 1e-5f);
    m_desired_number_of_simulated_seconds_per_real_time_second = value;
    m_spent_real_time = spent_simulation_time() / desired_number_of_simulated_seconds_per_real_time_second();
}

vector3 const&  simulator::get_position_of_selected() const
{
    ASSUMPTION(is_selected_something());
    if (is_selected_cell())
        return m_selected_cell->first;
    else if (is_selected_input_spot())
        return m_selected_input_spot->first;
    else 
        return m_selected_output_terminal->pos();
}

cell const&  simulator::get_selected_cell() const
{
    ASSUMPTION(is_selected_cell());
    return m_selected_cell->second;
}

input_spot const&  simulator::get_selected_input_spot() const
{
    ASSUMPTION(is_selected_input_spot());
    return m_selected_input_spot->second;
}

output_terminal const&  simulator::get_selected_output_terminal() const
{
    ASSUMPTION(is_selected_output_terminal());
    return *m_selected_output_terminal;
}

std::string  simulator::get_selected_info_text() const
{
    if (!is_selected_something())
        return "";

    std::ostringstream  ostr;

    ostr << "position: [ " << std::fixed << get_position_of_selected()(0)
                          << ", "
                          << std::fixed << get_position_of_selected()(1)
                          << ", "
                          << std::fixed << get_position_of_selected()(2)
                          << " ]\n"
         ;

    if (is_selected_cell())
    {
        ostr << "spiking potential: " << std::fixed << get_selected_cell().spiking_potential() << "\n"
             << "the last update: " << get_selected_cell().last_update() << "\n"
             << "is excitatory: " << std::boolalpha << get_selected_cell().is_excitatory() << "\n"
             ;
        INVARIANT(m_selected_cell_stats.operator bool());
        ostr << "observed from update: " << m_selected_cell_stats->start_update() << "\n"
             << "last mini spike update: " << m_selected_cell_stats->last_mini_spike_update() << "\n"
             << "number of mini spikes: " << m_selected_cell_stats->num_mini_spikes() << "\n"
             << "average mini spikes rate: " << std::fixed
                    << m_selected_cell_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
             << "last spike update: " << m_selected_cell_stats->last_spike_update() << "\n"
             << "number of spikes: " << m_selected_cell_stats->num_spikes() << "\n"
             << "average spikes rate: " << std::fixed
                    << m_selected_cell_stats->average_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
             << "last presynaptic potential update: " << m_selected_cell_stats->last_presynaptic_potential_update() << "\n"
             << "number of presynaptic potentials: " << m_selected_cell_stats->num_presynaptic_potentials() << "\n"
             << "average presynaptic potentials rate: " << std::fixed
                    << m_selected_cell_stats->average_presynaptic_potentials_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
             ;
    }
    else if (is_selected_input_spot())
    {
        ostr << "";
        INVARIANT(m_selected_input_spot_stats.operator bool());
        ostr << "observed from update: " << m_selected_input_spot_stats->start_update() << "\n"
             << "last mini spike update: " << m_selected_input_spot_stats->last_mini_spike_update() << "\n"
             << "number of mini spikes: " << m_selected_input_spot_stats->num_mini_spikes() << "\n"
             << "average mini spikes rate: " << std::fixed
                    << m_selected_input_spot_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
             ;
    }
    else if (is_selected_output_terminal())
    {
        ostr << "velocity: [ " << std::fixed << get_selected_output_terminal().velocity()(0)
                               << ", "
                               << std::fixed << get_selected_output_terminal().velocity()(1)
                               << ", "
                               << std::fixed << get_selected_output_terminal().velocity()(2)
                               << " ]\n"
             << "synaptic weight: " << std::fixed << get_selected_output_terminal().synaptic_weight() << "\n"
             ;
        INVARIANT(m_selected_output_terminal_stats.operator bool());
        ostr << "observed from update: " << m_selected_output_terminal_stats->start_update() << "\n"
             << "last mini spike update: " << m_selected_output_terminal_stats->last_mini_spike_update() << "\n"
             << "number of mini spikes: " << m_selected_output_terminal_stats->num_mini_spikes() << "\n"
             << "average mini spikes rate: " << std::fixed
                    << m_selected_output_terminal_stats->average_mini_spikes_rate(nenet()->get_params()->update_time_step_in_seconds()) << "\n"
             ;
    }
    return ostr.str();
}
