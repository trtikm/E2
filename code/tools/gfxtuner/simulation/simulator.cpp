#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene_utils.hpp>
#include <angeo/collide.hpp>
#include <qtgl/glapi.hpp>
#include <qtgl/draw.hpp>
#include <qtgl/batch_generators.hpp>
#include <qtgl/texture_generators.hpp>
#include <qtgl/camera_utils.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/random.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/development.hpp>

#include <vector>
#include <map>
#include <algorithm>


simulator::simulator()
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
    , m_effects_config(
            qtgl::effects_config::light_types{
                qtgl::LIGHT_TYPE::AMBIENT,
                qtgl::LIGHT_TYPE::DIRECTIONAL,
                },
            qtgl::effects_config::lighting_data_types{
                { qtgl::LIGHTING_DATA_TYPE::POSITION, qtgl::SHADER_DATA_INPUT_TYPE::UNIFORM },
                { qtgl::LIGHTING_DATA_TYPE::NORMAL, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE },
                { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE },
                { qtgl::LIGHTING_DATA_TYPE::SPECULAR, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            qtgl::effects_config::shader_output_types{
                qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT
                },
            qtgl::FOG_TYPE::NONE
            )
    , m_diffuse_colour{ 1.0f, 1.0f, 1.0f, 1.0f }
    , m_ambient_colour{ 0.5f, 0.5f, 0.5f }
    , m_specular_colour{ 1.0f, 1.0f, 1.0f, 2.0f }
    , m_directional_light_direction(normalised(-vector3(1.0f, 1.0f, 1.0f)))
    , m_directional_light_colour{ 1.0f, 1.0f, 1.0f }
    , m_fog_colour{ 0.25f, 0.25f, 0.25f, 2.0f }
    , m_fog_near(0.25f)
    , m_fog_far(1000.0f)

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
                    m_effects_config.get_fog_type(),
                    "gfxtuner"
                    )
            }
    , m_do_show_grid(true)

    , m_paused(true)
    , m_do_single_step(false)

    , m_scene(new scn::scene)
    , m_scene_selection(m_scene)
    , m_scene_history(new scn::scene_history)
    , m_batch_coord_system(qtgl::create_basis_vectors())
    , m_scene_edit_data(scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT)

    , m_gfx_animated_objects()
    , m_collision_scene()
{}

simulator::~simulator()
{
}


float_32_bit  simulator::get_camera_side_plane_minimal_distance() const
{
    return std::max(
                0.01f,
                std::min({
                        std::fabs(m_camera->left()),
                        std::fabs(m_camera->right()),
                        std::fabs(m_camera->top()),
                        std::fabs(m_camera->bottom())
                        })
                );
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


void  simulator::next_round(float_64_bit const  seconds_from_previous_call,
                            bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    bool  is_simulation_round = false;
    if (!is_this_pure_redraw_request)
    {
        qtgl::adjust(*m_camera,window_props());
        auto const translated_rotated =
            qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                           seconds_from_previous_call, mouse_props(), keyboard_props());
        if (translated_rotated.first)
            call_listeners(simulator_notifications::camera_position_updated());
        if (translated_rotated.second)
            call_listeners(simulator_notifications::camera_orientation_updated());

        bool const  old_paused = paused();

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
        {
            is_simulation_round = true;
            if (old_paused != paused())
                validate_simulation_state();
            perform_simulation_step(seconds_from_previous_call);
        }
        else
            perform_scene_update(seconds_from_previous_call);

        if (m_do_single_step)
        {
            m_paused = true;
            call_listeners(simulator_notifications::paused());
            m_do_single_step = false;
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
            render_batch(
                m_batch_grid,
                qtgl::vertex_shader_uniform_data_provider(
                    m_batch_grid,
                    { matrix_from_world_to_camera },
                    matrix_from_camera_to_clipspace,
                    m_diffuse_colour,
                    m_ambient_colour,
                    m_specular_colour,
                    transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                    m_directional_light_colour,
                    m_fog_colour,
                    m_fog_near,
                    m_fog_far
                    )
                );
            draw_state = m_batch_grid.get_draw_state();
        }

    if (is_simulation_round)
        render_simulation_state(matrix_from_world_to_camera, matrix_from_camera_to_clipspace ,draw_state);
    else
    {
        render_scene_batches(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
        render_scene_coord_systems(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    }

    qtgl::swap_buffers();
}


void  simulator::validate_simulation_state()
{
    TMPROF_BLOCK();

    std::unordered_set<std::pair<std::string, std::string> >  to_remove;
    for (auto& elem : m_gfx_animated_objects)
    {
        auto const  node = get_scene().get_scene_node(elem.first.first);
        if (node == nullptr || !node->has_batch(elem.first.second))
            to_remove.insert(elem.first);
    }
    for (auto const&  key : to_remove)
        m_gfx_animated_objects.erase(key);

    for (auto const& name_node : get_scene().get_all_scene_nodes())
        for (auto const& name_batch : name_node.second->get_batches())
            if (name_batch.second.ready() && name_batch.second.get_available_resources().skeletal().size() == 1UL)
            {
                std::pair<std::string, std::string> const  key = { name_node.first, name_batch.first };
                if (m_gfx_animated_objects.count(key) == 0UL)
                    m_gfx_animated_objects.emplace(key, gfx_animated_object(name_batch.second));
            }
}


void  simulator::perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    for (auto&  elem : m_gfx_animated_objects)
        elem.second.next_round(time_to_simulate_in_seconds);
}


void  simulator::render_simulation_state(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    std::unordered_map<
            std::string,    // batch ID
            std::pair<
                qtgl::batch,
                std::vector<std::pair<scn::scene_node_const_ptr,gfx_animated_object const*> > > >
        batches;
    for (auto const& name_node : get_scene().get_all_scene_nodes())
        for (auto const& name_batch : name_node.second->get_batches())
        {
            auto&  record = batches[name_batch.second.path_component_of_uid()];
            if (record.first.empty())
                record.first = name_batch.second;
            INVARIANT(record.first == name_batch.second);
            auto const  it = m_gfx_animated_objects.find({ name_node.first, name_batch.first });
            INVARIANT(it == m_gfx_animated_objects.cend() || it->second.get_batch() == record.first);
            record.second.push_back({
                name_node.second,
                it != m_gfx_animated_objects.cend() ? &it->second : nullptr
                });
        }
    for (auto const& elem : batches)
        if (qtgl::make_current(elem.second.first, draw_state))
        {
            for (auto const& node_and_anim : elem.second.second)
                if (node_and_anim.second == nullptr)
                    qtgl::render_batch(
                        elem.second.first,
                        qtgl::vertex_shader_uniform_data_provider(
                            elem.second.first,
                            { matrix_from_world_to_camera * node_and_anim.first->get_world_matrix() },
                            matrix_from_camera_to_clipspace,
                            m_diffuse_colour,
                            m_ambient_colour,
                            m_specular_colour,
                            transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                            m_directional_light_colour,
                            m_fog_colour,
                            m_fog_near,
                            m_fog_far
                            ),
                        qtgl::fragment_shader_uniform_data_provider(
                            m_diffuse_colour,
                            m_ambient_colour,
                            m_specular_colour,
                            transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                            m_directional_light_colour,
                            m_fog_colour,
                            m_fog_near,
                            m_fog_far
                            )
                        );
                else
                {
                    std::vector<matrix44>  frame;
                    node_and_anim.second->get_transformations(
                            frame,
                            matrix_from_world_to_camera * node_and_anim.first->get_world_matrix()
                            );
                    qtgl::render_batch(
                            elem.second.first,
                            qtgl::vertex_shader_uniform_data_provider(
                                elem.second.first,
                                frame,
                                matrix_from_camera_to_clipspace,
                                m_diffuse_colour,
                                m_ambient_colour,
                                m_specular_colour,
                                transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                                m_directional_light_colour,
                                m_fog_colour,
                                m_fog_near,
                                m_fog_far
                                ),
                            qtgl::fragment_shader_uniform_data_provider(
                                m_diffuse_colour,
                                m_ambient_colour,
                                m_specular_colour,
                                transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                                m_directional_light_colour,
                                m_fog_colour,
                                m_fog_near,
                                m_fog_far
                                )
                        );
                }
            draw_state = elem.second.first.get_draw_state();
        }
}


void  simulator::perform_scene_update(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    switch (get_scene_edit_mode())
    {
    case scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT:
        select_scene_objects(time_to_simulate_in_seconds);
        break;
    case scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES:
        translate_scene_selected_objects(time_to_simulate_in_seconds);
        break;
    case scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES:
        rotate_scene_selected_objects(time_to_simulate_in_seconds);
        break;
    default:
        UNREACHABLE();
        break;
    }
}

void  simulator::select_scene_objects(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (mouse_props().was_just_released(qtgl::LEFT_MOUSE_BUTTON()) == mouse_props().was_just_released(qtgl::RIGHT_MOUSE_BUTTON()))
        return;

    if (get_scene_edit_data().are_data_invalidated())
        m_scene_edit_data.initialise_selection_data({});

    vector3 ray_begin, ray_end;
    cursor_line_begin(
        *m_camera,
        { mouse_props().x(), mouse_props().y() },
        window_props(),
        ray_begin
        );
    cursor_line_end(*m_camera, ray_begin, ray_end);

    std::map<scalar, std::string>  nodes_on_line;
    find_scene_nodes_on_line(get_scene(), ray_begin, ray_end, nodes_on_line);

    if (mouse_props().was_just_released(qtgl::RIGHT_MOUSE_BUTTON()) ||
        keyboard_props().is_pressed(qtgl::KEY_LCTRL()) ||
        keyboard_props().is_pressed(qtgl::KEY_RCTRL()))
    {
        if (nodes_on_line.empty())
            return;

        std::string const&  chosen_node_name = nodes_on_line.begin()->second;

        std::unordered_set<std::string>  nodes_of_selected_batches;
        get_nodes_of_selected_batches(m_scene_selection, nodes_of_selected_batches);
        if (m_scene_selection.is_node_selected(chosen_node_name) ||
            nodes_of_selected_batches.count(chosen_node_name) != 0UL)
        {
            m_scene_selection.erase_node(chosen_node_name);
            m_scene_selection.erase_batches_of_node(chosen_node_name);
        }
        else if (get_scene().get_scene_node(chosen_node_name)->get_batches().empty())
            m_scene_selection.insert_node(chosen_node_name);
        else
            m_scene_selection.insert_batches_of_node(chosen_node_name);
    }
    else
    {
        m_scene_selection.clear();

        if (!nodes_on_line.empty())
        {
            std::vector<std::string>  candidate_nodes_on_line;
            {
                scalar const  max_candidate_distance = 2.0f;
                    //compute_bounding_sphere_radius_of_scene_node(*get_scene().get_scene_node(nodes_on_line.cbegin()->second));
                scalar const  line_length = length(ray_end - ray_begin);
                scalar const  max_param_value =
                    nodes_on_line.cbegin()->first + (line_length < max_candidate_distance ? 1.0 : max_candidate_distance / line_length);
                for (auto const&  param_and_name : nodes_on_line)
                {
                    if (param_and_name.first > max_param_value)
                        break;
                    candidate_nodes_on_line.push_back(param_and_name.second);
                }
                INVARIANT(!candidate_nodes_on_line.empty());
            }
            std::string const  chosen_node_name = m_scene_edit_data.get_selection_data().choose_best_selection(candidate_nodes_on_line);
            if (!get_scene().get_scene_node(chosen_node_name)->get_batches().empty())
                m_scene_selection.insert_batches_of_node(chosen_node_name);
            else
                m_scene_selection.insert_node(chosen_node_name);
        }
    }

    call_listeners(simulator_notifications::scene_scene_selection_changed());
}

void  simulator::translate_scene_selected_objects(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (!mouse_props().is_pressed(qtgl::LEFT_MOUSE_BUTTON()))
    {
        if (m_scene_edit_data.was_operation_started() && mouse_props().was_just_released(qtgl::LEFT_MOUSE_BUTTON()))
            call_listeners(simulator_notifications::scene_node_position_update_finished());
        return;
    }
    if (m_scene_selection.empty())
        return;
    if (get_scene_edit_data().are_data_invalidated())
    {
        if (m_scene_selection.is_node_selected(scn::get_pivot_node_name()))
            m_scene_edit_data.initialise_translation_data({ 
                get_scene().get_scene_node(scn::get_pivot_node_name())->get_coord_system()->origin()
                });
        else
        {
            vector3  lo, hi;
            if (!get_bbox_of_selected_scene_nodes(m_scene_selection, m_scene, lo, hi))
                return;
            m_scene_edit_data.initialise_translation_data({ 0.5f * (lo + hi) });
        }
        call_listeners(simulator_notifications::scene_node_position_update_started());
    }
    m_scene_edit_data.get_translation_data().update(
            keyboard_props().is_pressed(qtgl::KEY_X()),
            keyboard_props().is_pressed(qtgl::KEY_Y()),
            keyboard_props().is_pressed(qtgl::KEY_Z()),
            m_camera->coordinate_system()->origin()
            );
    if (mouse_props().was_just_pressed(qtgl::LEFT_MOUSE_BUTTON()))
        m_scene_edit_data.get_translation_data().invalidate_plain_point();

    vector3 new_ray_begin, new_ray_end;
    cursor_line_begin(
            *m_camera,
            { mouse_props().x(), mouse_props().y() },
            window_props(),
            new_ray_begin
            );
    cursor_line_end(*m_camera, new_ray_begin, new_ray_end);

    vector3  new_plane_point;
    if (!angeo::collision_ray_and_plane(
            new_ray_begin,
            normalised(new_ray_end - new_ray_begin),
            get_scene_edit_data().get_translation_data().get_origin(),
            get_scene_edit_data().get_translation_data().get_normal(),
            nullptr,
            nullptr,
            nullptr,
            &new_plane_point
            ))
        return;

    vector3 const  raw_shift = m_scene_edit_data.get_translation_data().get_shift(new_plane_point);

    std::unordered_set<std::string> nodes_to_translate = m_scene_selection.get_nodes();
    for (auto const& node_batch_names : m_scene_selection.get_batches())
        nodes_to_translate.insert(node_batch_names.first);
    if (nodes_to_translate.size() > 1UL)
        nodes_to_translate.erase(scn::get_pivot_node_name());
    vector3 const  shift =
        m_scene_selection.is_node_selected(scn::get_pivot_node_name())
        && nodes_to_translate.count(scn::get_pivot_node_name()) == 0UL ?
                contract43(get_scene().get_scene_node(scn::get_pivot_node_name())->get_world_matrix()
                    * expand34(raw_shift, 0.0f)) :
                raw_shift;
    for (auto const& node_name : nodes_to_translate)
    {
        scn::scene_node_ptr const  node = get_scene_node(node_name);
        vector3 const  node_shift = node->has_parent() ?
            contract43(inverse44(node->get_parent()->get_world_matrix()) * expand34(shift, 0.0f)) :
            shift;
        node->translate(node_shift);
    }

    call_listeners(simulator_notifications::scene_node_position_updated());
}

void  simulator::rotate_scene_selected_objects(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (mouse_props().is_pressed(qtgl::LEFT_MOUSE_BUTTON()) == mouse_props().is_pressed(qtgl::RIGHT_MOUSE_BUTTON()))
    {
        if (m_scene_edit_data.was_operation_started() && (mouse_props().was_just_released(qtgl::LEFT_MOUSE_BUTTON()) ||
                                                          mouse_props().was_just_released(qtgl::RIGHT_MOUSE_BUTTON())))
            call_listeners(simulator_notifications::scene_node_orientation_update_finished());
        return;
    }
    if (m_scene_selection.empty())
        return;

    if (get_scene_edit_data().are_data_invalidated())
    {
        if (m_scene_selection.is_node_selected(scn::get_pivot_node_name()))
            m_scene_edit_data.initialise_rotation_data({
                get_scene().get_scene_node(scn::get_pivot_node_name())->get_coord_system()->origin()
                });
        else
        {
            vector3  lo, hi;
            if (!get_bbox_of_selected_scene_nodes(m_scene_selection, m_scene, lo, hi))
                return;
            m_scene_edit_data.initialise_rotation_data({ 0.5f * (lo + hi) });
        }
        call_listeners(simulator_notifications::scene_node_orientation_update_started());
    }

    float_32_bit const  horisontal_full_angle_in_pixels = (3.0f / 4.0f) * window_props().width_in_pixels();
    float_32_bit const  vertical_full_angle_in_pixels = (3.0f / 4.0f) * window_props().height_in_pixels();

    float_32_bit const  horisontal_angle = (2.0f * PI()) * (mouse_props().x_delta() / horisontal_full_angle_in_pixels);
    float_32_bit const  vertical_angle = (2.0f * PI()) * (mouse_props().y_delta() / vertical_full_angle_in_pixels);

    bool const  x_down = keyboard_props().is_pressed(qtgl::KEY_X());
    bool const  y_down = keyboard_props().is_pressed(qtgl::KEY_Y());
    bool const  z_down = keyboard_props().is_pressed(qtgl::KEY_Z());
    bool const  p_down = keyboard_props().is_pressed(qtgl::KEY_P());
    bool const  r_down = keyboard_props().is_pressed(qtgl::KEY_R());

    quaternion  raw_rotation;
    if (x_down || y_down || z_down || p_down || r_down)
    {
        raw_rotation = quaternion_identity();
        if (x_down)
            raw_rotation *= angle_axis_to_quaternion(vertical_angle, vector3_unit_x());
        if (y_down)
            raw_rotation *= angle_axis_to_quaternion(vertical_angle, vector3_unit_y());
        if (z_down)
            raw_rotation *= angle_axis_to_quaternion(horisontal_angle, vector3_unit_z());
        if (p_down)
        {
            vector3 const  camera_x_axis = axis_x(*m_camera->coordinate_system());
            raw_rotation *= angle_axis_to_quaternion(horisontal_angle, { -camera_x_axis(1), camera_x_axis(0), 0.0f});
        }
        if (r_down)
            raw_rotation *= angle_axis_to_quaternion(vertical_angle, axis_x(*m_camera->coordinate_system()));
    }
    else
        raw_rotation = angle_axis_to_quaternion(horisontal_angle, vector3_unit_z());

    vector3  raw_axis;
    scalar const  angle = quaternion_to_angle_axis(raw_rotation, raw_axis);

    std::unordered_set<std::string> nodes_to_rotate = m_scene_selection.get_nodes();
    for (auto const& node_batch_names : m_scene_selection.get_batches())
        nodes_to_rotate.insert(node_batch_names.first);
    if (nodes_to_rotate.size() > 1UL)
        nodes_to_rotate.erase(scn::get_pivot_node_name());
    bool const  rotate_around_pivot =
        m_scene_selection.is_node_selected(scn::get_pivot_node_name())
        && nodes_to_rotate.count(scn::get_pivot_node_name()) == 0UL;
    bool const  is_alternative_rotation_enabled =
        nodes_to_rotate.size() > 1UL && mouse_props().is_pressed(qtgl::RIGHT_MOUSE_BUTTON());
    vector3 const  axis = rotate_around_pivot ?
        contract43(get_scene().get_scene_node(scn::get_pivot_node_name())->get_world_matrix() * expand34(raw_axis, 0.0f)) :
        raw_axis;
    matrix33 const  radius_vector_rotation_matrix = is_alternative_rotation_enabled ?
        quaternion_to_rotation_matrix(angle_axis_to_quaternion(angle, axis)) :
        matrix33_identity();
    for (auto const& node_name : nodes_to_rotate)
    {
        scn::scene_node_ptr const  node = get_scene_node(node_name);
        vector3 const  rotation_axis = node->has_parent() ?
                contract43(inverse44(node->get_parent()->get_world_matrix()) * expand34(axis, 0.0f)) :
                axis ;
        quaternion const  rotation = angle_axis_to_quaternion(angle, rotation_axis);
        node->rotate(rotation);

        if (is_alternative_rotation_enabled)
        {
            vector3 const  original_radius_vector = 
                    contract43(node->get_world_matrix() * expand34(vector3_zero())) -
                    get_scene_edit_data().get_rotation_data().get_origin();
            vector3 const  rotated_radius_vector = radius_vector_rotation_matrix * original_radius_vector;
            vector3 const  raw_shift = rotated_radius_vector - original_radius_vector;
            vector3 const  shift = node->has_parent() ?
                    contract43(inverse44(node->get_parent()->get_world_matrix()) * expand34(raw_shift, 0.0f)) :
                    raw_shift;
            node->translate(shift);
        }
    }

    call_listeners(simulator_notifications::scene_node_orientation_updated());
}

void  simulator::rotate_scene_node(std::string const&  scene_node_name, float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

}

void  simulator::render_scene_batches(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    std::unordered_map<std::string, std::vector<std::pair<qtgl::batch, scn::scene_node_const_ptr> > >
            batches;
    for (auto const& name_node : get_scene().get_all_scene_nodes())
        for (auto const& name_batch : name_node.second->get_batches())
            batches[name_batch.second.path_component_of_uid()].push_back({name_batch.second, name_node.second});
    for (auto const& path_and_pairs : batches)
        if (qtgl::make_current(path_and_pairs.second.front().first, draw_state))
        {
            for (auto const& batch_and_node : path_and_pairs.second)
                qtgl::render_batch(
                        batch_and_node.first,
                        qtgl::vertex_shader_uniform_data_provider(
                            batch_and_node.first,
                            { matrix_from_world_to_camera * batch_and_node.second->get_world_matrix() },
                            matrix_from_camera_to_clipspace,
                            m_diffuse_colour,
                            m_ambient_colour,
                            m_specular_colour,
                            transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                            m_directional_light_colour,
                            m_fog_colour,
                            m_fog_near,
                            m_fog_far
                            ),
                        qtgl::fragment_shader_uniform_data_provider(
                            m_diffuse_colour,
                            m_ambient_colour,
                            m_specular_colour,
                            transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                            m_directional_light_colour,
                            m_fog_colour,
                            m_fog_near,
                            m_fog_far
                            )
                        );
            draw_state = path_and_pairs.second.front().first.get_draw_state();
        }
}

void  simulator::render_scene_coord_systems(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    if (!qtgl::make_current(m_batch_coord_system, draw_state))
        return;

    //auto const  old_depth_test_state = qtgl::glapi().glIsEnabled(GL_DEPTH_TEST);
    //qtgl::glapi().glDisable(GL_DEPTH_TEST);

    std::unordered_set<std::string>  nodes_to_draw = m_scene_selection.get_nodes();
    get_nodes_of_selected_batches(m_scene_selection, nodes_to_draw);
    if (get_scene().has_scene_node(scn::get_pivot_node_name())) // The pivot may be missing, if the scene is not completely initialised yet.
        nodes_to_draw.insert(scn::get_pivot_node_name());
    for (auto const& node_name : nodes_to_draw)
        qtgl::render_batch(
            m_batch_coord_system,
            qtgl::vertex_shader_uniform_data_provider(
                m_batch_coord_system,
                { matrix_from_world_to_camera * get_scene().get_scene_node(node_name)->get_world_matrix() },
                matrix_from_camera_to_clipspace,
                m_diffuse_colour,
                m_ambient_colour,
                m_specular_colour,
                transform_vector(m_directional_light_direction, matrix_from_world_to_camera),
                m_directional_light_colour,
                m_fog_colour,
                m_fog_near,
                m_fog_far
                )
            );

    //if (old_depth_test_state)
    //    qtgl::glapi().glEnable(GL_DEPTH_TEST);

    draw_state = m_batch_coord_system.get_draw_state();
}

void  simulator::erase_scene_node(std::string const&  name)
{
    TMPROF_BLOCK();

    m_scene_selection.erase_node(name);
    m_scene_selection.erase_batches_of_node(name);

    m_scene_edit_data.invalidate_data();

    get_scene().erase_scene_node(name);

}

void  simulator::insert_batch_to_scene_node(std::string const&  batch_name, boost::filesystem::path const&  batch_pathname, std::string const&  scene_node_name)
{
    TMPROF_BLOCK();

    ASSUMPTION(get_scene().has_scene_node(scene_node_name));
    auto const  batch = qtgl::batch(canonical_path(batch_pathname), get_effects_config());
    get_scene_node(scene_node_name)->insert_batches({ { batch_name, batch } });
}

void  simulator::erase_batch_from_scene_node(std::string const&  batch_name, std::string const&  scene_node_name)
{
    m_scene_selection.erase_batch({ scene_node_name, batch_name });
    auto const  node = get_scene_node(scene_node_name);
    node->erase_batches({ batch_name });
}


void  simulator::clear_scene()
{
    m_scene_selection.clear();
    get_scene().clear();
}

void  simulator::translate_scene_node(std::string const&  scene_node_name, vector3 const&  shift)
{
    get_scene_node(scene_node_name)->translate(shift);
    m_scene_edit_data.invalidate_data();
}

void  simulator::rotate_scene_node(std::string const&  scene_node_name, quaternion const&  rotation)
{
    get_scene_node(scene_node_name)->rotate(rotation);
    m_scene_edit_data.invalidate_data();
}

void  simulator::set_position_of_scene_node(std::string const&  scene_node_name, vector3 const&  new_origin)
{
    get_scene_node(scene_node_name)->set_origin(new_origin);
    m_scene_edit_data.invalidate_data();
}

void  simulator::set_orientation_of_scene_node(std::string const&  scene_node_name, quaternion const&  new_orientation)
{
    get_scene_node(scene_node_name)->set_orientation(new_orientation);
    m_scene_edit_data.invalidate_data();
}

void  simulator::relocate_scene_node(std::string const&  scene_node_name, vector3 const&  new_origin, quaternion const&  new_orientation)
{
    get_scene_node(scene_node_name)->relocate(new_origin, new_orientation);
    m_scene_edit_data.invalidate_data();
}

void  simulator::set_scene_selection(
        std::unordered_set<std::string> const&  selected_scene_nodes,
        std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
        )
{
    TMPROF_BLOCK();

    m_scene_selection.clear();
    insert_to_scene_selection(selected_scene_nodes, selected_batches);
}

void  simulator::insert_to_scene_selection(
        std::unordered_set<std::string> const&  selected_scene_nodes,
        std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
        )
{
    TMPROF_BLOCK();

    for (auto const& name : selected_scene_nodes)
        m_scene_selection.insert_node(name);
    for (auto const& node_batch_names : selected_batches)
        m_scene_selection.insert_batch(node_batch_names);

    m_scene_edit_data.invalidate_data();
}

void  simulator::erase_from_scene_selection(
        std::unordered_set<std::string> const&  selected_scene_nodes,
        std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
        )
{
    TMPROF_BLOCK();

    for (auto const& name : selected_scene_nodes)
        m_scene_selection.erase_node(name);
    for (auto const& node_batch_names : selected_batches)
        m_scene_selection.erase_batch(node_batch_names);

    m_scene_edit_data.invalidate_data();
}

void  simulator::get_scene_selection(
        std::unordered_set<std::string>&  selected_scene_nodes,
        std::unordered_set<std::pair<std::string, std::string> >&  selected_batches
        ) const
{
    TMPROF_BLOCK();

    selected_scene_nodes = m_scene_selection.get_nodes();
    selected_batches = m_scene_selection.get_batches();
}


void  simulator::set_scene_edit_mode(scn::SCENE_EDIT_MODE const  edit_mode)
{
    if (m_scene_edit_data.get_mode() != edit_mode)
    {
        m_scene_edit_data.set_mode(edit_mode);
        call_listeners(simulator_notifications::scene_edit_mode_changed());
    }
}
