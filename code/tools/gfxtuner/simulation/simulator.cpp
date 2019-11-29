#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <gfxtuner/simulation/bind_ai_scene_to_simulator.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene_utils.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <angeo/collide.hpp>
#include <angeo/mass_and_inertia_tensor.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <angeo/utility.hpp>
#include <angeo/tensor_std_specialisations.hpp>
#include <ai/skeleton_utils.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
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
#include <boost/filesystem.hpp>
#include <vector>
#include <map>
#include <algorithm>
#include <unordered_set>

namespace detail {


struct  collider_triangle_mesh_vertex_getter
{
    collider_triangle_mesh_vertex_getter(
            qtgl::buffer const  vertex_buffer_,
            qtgl::buffer const  index_buffer_
            )
        : vertex_buffer(vertex_buffer_)
        , index_buffer(index_buffer_)
    {
        ASSUMPTION(vertex_buffer.loaded_successfully() && index_buffer.loaded_successfully());
        ASSUMPTION(
            vertex_buffer.num_bytes_per_component() == sizeof(float_32_bit) &&
            vertex_buffer.num_components_per_primitive() == 3U &&
            vertex_buffer.has_integral_components() == false
            );
        ASSUMPTION(
            index_buffer.num_bytes_per_component() == sizeof(natural_32_bit) &&
            index_buffer.num_components_per_primitive() == 3U &&
            index_buffer.has_integral_components() == true
            );
    }

    vector3  operator()(natural_32_bit const  triangle_index, natural_8_bit const  vertex_index) const
    {
        return vector3(((float_32_bit const*)vertex_buffer.data().data()) + 3U * read_index_buffer(triangle_index, vertex_index));
    }

    natural_32_bit  read_index_buffer(natural_32_bit const  triangle_index, natural_8_bit const  vertex_index) const
    {
        return *(((natural_32_bit const*)index_buffer.data().data()) + 3U * triangle_index + vertex_index);
    }

    qtgl::buffer  get_vertex_buffer() const { return vertex_buffer; }
    qtgl::buffer  get_index_buffer() const { return index_buffer; }

private:
    qtgl::buffer  vertex_buffer;
    qtgl::buffer  index_buffer;
};


void  skeleton_enumerate_nodes_of_bones(
        scn::scene_node_ptr const  agent_node_ptr,
        ai::skeletal_motion_templates&  motion_templates,
        std::function<
                bool(    // Continue in the enumeration of the remaining bones? I.e. 'false' early terminates the enumeration.
                    natural_32_bit,         // bone index
                    scn::scene_node_ptr,    // the corresponding node in the scene; can be 'nullptr', if the node is not in the scene.
                    bool                    // True if the bone has a parent bone, and False otherwise.
                    )> const&  callback // Called for each bone, in the fixed increasing order of the bone index: 0,1,2,...
        )
{
    std::vector<scn::scene_node_ptr>  nodes{ agent_node_ptr };
    for (natural_32_bit bone = 0U; bone != motion_templates.pose_frames().size(); ++bone)
    {
        scn::scene_node_ptr  child_node_ptr = nullptr;
        {
            scn::scene_node_ptr const  parent_node_ptr = nodes.at(motion_templates.hierarchy().parents().at(bone) + 1);
            if (parent_node_ptr != nullptr)
            {
                auto const  child_node_it = parent_node_ptr->find_child(motion_templates.names().at(bone));
                if (child_node_it != parent_node_ptr->get_children().cend())
                    child_node_ptr = child_node_it->second;
            }
        }
        if (callback(bone, child_node_ptr, motion_templates.hierarchy().parents().at(bone) >= 0) == false)
            break;
        nodes.push_back(child_node_ptr);
    }
}


scn::scene_node_id  skeleton_build_scene_node_id_of_bones(
        natural_32_bit const  bone,
        std::vector<integer_32_bit> const&  parents,
        ai::skeletal_motion_templates::bone_names const&  names
        )
{
    scn::scene_node_id::path_type  path;
    for (integer_32_bit b = (integer_32_bit)bone; b >= 0; b = parents.at(b))
        path.push_back(names.at(b));
    std::reverse(path.begin(), path.end());
    return scn::scene_node_id{ path };
}


}


simulator::simulator()
    : qtgl::real_time_simulator()

    // Data providing feedback loop between a human user and 3D scene in the tool

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
    , m_camera_controller_type_in_edit_mode(CAMERA_CONTROLLER_FREE_FLY)
    , m_camera_controller_type_in_simulation_mode(CAMERA_CONTROLLER_FREE_FLY)
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
    , m_orbit_config{
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
                }
            }
    , m_only_move_camera_config
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
            }
    , m_camera_target_node_id()
    , m_camera_target_vector_in_camera_space(vector3_zero())
    , m_camera_target_vector_invalidated(true)

    , m_effects_config(
            nullptr,
            qtgl::effects_config::light_types{
                qtgl::LIGHT_TYPE::AMBIENT,
                qtgl::LIGHT_TYPE::DIRECTIONAL,
                },
            qtgl::effects_config::lighting_data_types{
                { qtgl::LIGHTING_DATA_TYPE::DIRECTION, qtgl::SHADER_DATA_INPUT_TYPE::UNIFORM },
                { qtgl::LIGHTING_DATA_TYPE::NORMAL, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE },
                { qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE },
                //{ qtgl::LIGHTING_DATA_TYPE::DIFFUSE, qtgl::SHADER_DATA_INPUT_TYPE::UNIFORM },
                { qtgl::LIGHTING_DATA_TYPE::SPECULAR, qtgl::SHADER_DATA_INPUT_TYPE::TEXTURE }
                },
            qtgl::SHADER_PROGRAM_TYPE::VERTEX,
            qtgl::effects_config::shader_output_types{
                qtgl::SHADER_DATA_OUTPUT_TYPE::DEFAULT
                },
            qtgl::FOG_TYPE::NONE,
            qtgl::SHADER_PROGRAM_TYPE::VERTEX
            )
    , m_diffuse_colour{ 1.0f, 1.0f, 1.0f, 1.0f }
    , m_ambient_colour{ 0.5f, 0.5f, 0.5f }
    , m_specular_colour{ 1.0f, 1.0f, 1.0f, 2.0f }
    , m_directional_light_direction(normalised(-vector3(2.0f, 1.0f, 3.0f)))
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
    , m_do_show_batches(true)
    , m_do_show_colliders(true)
    , m_do_show_contact_normals(false)
    , m_do_show_normals_of_collision_triangles(false)
    , m_do_show_neighbours_of_collision_triangles(false)
    , m_do_show_ai_action_controller_props(false)
    , m_colliders_colour{ 0.75f, 0.75f, 1.0f, 1.0f }
    , m_render_in_wireframe(false)

    // Common and shared data for both modes: Editing and Simulation

    , m_simulation_time_config()
    , m_scene(new scn::scene)
    , m_cache_of_batches_of_colliders()
    , m_cache_of_batches_of_ai_agents()
    , m_font_props(
            []() -> qtgl::font_mono_props {
                qtgl::font_mono_props  props;
                qtgl::load_font_mono_props(
                    boost::filesystem::path(get_program_options()->dataRoot()) / "shared" / "gfx" / "fonts" / "Liberation_Mono.txt",
                    props
                    );
                return props;
            }()        
            )

    // Editing mode data

    , m_scene_selection(m_scene)
    , m_scene_history(new scn::scene_history)
    , m_scene_edit_data(scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT)
    , m_batch_coord_system(qtgl::create_basis_vectors())

    // Simulation mode data

    , m_collision_scene_ptr(std::make_shared<angeo::collision_scene>())
    , m_rigid_body_simulator_ptr(std::make_shared<angeo::rigid_body_simulator>())
    , m_binding_of_collision_objects()
    , m_binding_of_rigid_bodies()
    , m_agents_ptr(std::make_shared<ai::agents>(std::make_shared<bind_ai_scene_to_simulator>(this)))
    , m_binding_of_agents_to_scene()

    // Debugging

    , __dbg_batch_coord_cross(qtgl::create_coord_cross(0.1f, m_colliders_colour))

    // Experiment

    , m_offscreens()
    , m_offscreen_recovery_times()
{
}

simulator::~simulator()
{
    this->clear_scene();
    async::terminate();
}


void  simulator::synchronise_with_dependent_modules()
{
    call_listeners(simulator_notifications::scene_edit_mode_changed());
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


void  simulator::next_round(float_64_bit  seconds_from_previous_call,
                            bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    simulation_time_config::auto_updater const  time_config(m_simulation_time_config, this);

    if (keyboard_props().was_just_released(qtgl::KEY_ESCAPE()))
        call_listeners(simulator_notifications::escape_simulator_window());

    if (!is_this_pure_redraw_request)
    {
        qtgl::adjust(*m_camera,window_props());

        bool  camera_controller_changed = false;
        qtgl::free_fly_report  free_fly_report;
        if (time_config().is_paused())
        {
            if (m_scene_selection.get_records().empty()
                    && m_scene_selection.get_nodes().size() == 1UL
                    && scn::has_rigid_body(*get_scene_node(*m_scene_selection.get_nodes().begin()))
                    )
            {
                m_camera_target_node_id = *m_scene_selection.get_nodes().begin();
                m_camera_target_vector_invalidated = true;
            }

            if (keyboard_props().is_pressed(qtgl::KEY_LCTRL()) || keyboard_props().is_pressed(qtgl::KEY_RCTRL()))
            {
                if (m_camera_controller_type_in_edit_mode != CAMERA_CONTROLLER_ORBIT)
                {
                    m_camera_controller_type_in_edit_mode = CAMERA_CONTROLLER_ORBIT;
                    camera_controller_changed = true;
                }

                vector3  center;
                {
                    std::vector<vector3> origins;
                    {
                        for (auto const& node_id : m_scene_selection.get_nodes())
                            origins.push_back(transform_point(vector3_zero(), get_scene_node(node_id)->get_world_matrix()));
                        for (auto const& record_id : m_scene_selection.get_records())
                            origins.push_back(transform_point(vector3_zero(), get_scene_node(record_id.get_node_id())->get_world_matrix()));
                    }
                    if (origins.empty())
                        center = get_scene_node(scn::get_pivot_node_id())->get_coord_system()->origin();
                    else
                    {
                        angeo::axis_aligned_bounding_box  bbox{ origins.back(), origins.back()};
                        origins.pop_back();
                        for (vector3 const& origin : origins)
                            angeo::extend_union_bbox(bbox, origin);
                        center = angeo::center_of_bbox(bbox);
                    }
                }

                vector3 const d = center - m_camera->coordinate_system()->origin();

                matrix44 T;
                angeo::to_base_matrix(*m_camera->coordinate_system(), T);

                free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                                  seconds_from_previous_call, mouse_props(), keyboard_props());
                if (free_fly_report.rotated == true)
                    free_fly_report.translated = true;

                matrix44 F;
                angeo::from_base_matrix(*m_camera->coordinate_system(), F);

                m_camera->coordinate_system()->set_origin(center - transform_vector(transform_vector(d, T), F));
            }
            else
            {
                if (m_camera_controller_type_in_edit_mode != CAMERA_CONTROLLER_FREE_FLY)
                {
                    m_camera_controller_type_in_edit_mode = CAMERA_CONTROLLER_FREE_FLY;
                    camera_controller_changed = true;
                }
                free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                                                  seconds_from_previous_call, mouse_props(), keyboard_props());
            }
        }
        else if (scn::scene_node_const_ptr const  node_ptr = get_scene_node(m_camera_target_node_id))
        {
            if (mouse_props().was_just_released(qtgl::MIDDLE_MOUSE_BUTTON()) && (keyboard_props().is_pressed(qtgl::KEY_LCTRL()) ||
                                                                                 keyboard_props().is_pressed(qtgl::KEY_RCTRL())))
            {
                camera_controller_changed = true;
                m_camera_target_vector_invalidated = true;
                if (keyboard_props().is_pressed(qtgl::KEY_LSHIFT()) || keyboard_props().is_pressed(qtgl::KEY_RSHIFT()))
                    m_camera_controller_type_in_simulation_mode = (CAMERA_CONTROLLER_TYPE)
                        ((m_camera_controller_type_in_simulation_mode + NUM_CAMERA_CONTROLLER_TYPES - 1) % NUM_CAMERA_CONTROLLER_TYPES);
                else
                    m_camera_controller_type_in_simulation_mode = (CAMERA_CONTROLLER_TYPE)
                        ((m_camera_controller_type_in_simulation_mode + 1) % NUM_CAMERA_CONTROLLER_TYPES);
            }

            if (m_camera_controller_type_in_simulation_mode == CAMERA_CONTROLLER_FREE_FLY)
                free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                                                  seconds_from_previous_call, mouse_props(), keyboard_props());
            else
            {
                free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_only_move_camera_config,
                                                  seconds_from_previous_call, mouse_props(), keyboard_props());
                if (free_fly_report.translated)
                    m_camera_target_vector_invalidated =true;

                vector3 const  center = transform_point(vector3_zero(), node_ptr->get_world_matrix());
                if (m_camera_target_vector_invalidated)
                {
                    vector3 const d = center - m_camera->coordinate_system()->origin();
                    matrix44 T;
                    angeo::to_base_matrix(*m_camera->coordinate_system(), T);
                    m_camera_target_vector_in_camera_space = transform_vector(d, T);
                    m_camera_target_vector_invalidated = false;
                }

                switch (m_camera_controller_type_in_simulation_mode)
                {
                case CAMERA_CONTROLLER_ORBIT:
                    {
                        free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                                            seconds_from_previous_call, mouse_props(), keyboard_props());
                        if (free_fly_report.rotated == true)
                            free_fly_report.translated = true;

                        matrix44 F;
                        angeo::from_base_matrix(*m_camera->coordinate_system(), F);

                        m_camera->coordinate_system()->set_origin(center - transform_vector(m_camera_target_vector_in_camera_space, F));
                    }
                    break;
                case CAMERA_CONTROLLER_FOLLOW:
                    {
                        free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                                            seconds_from_previous_call, mouse_props(), keyboard_props());

                        free_fly_report.translated = true;
                        vector3  d = m_camera->coordinate_system()->origin() - center;
                        float_32_bit const  d_len = length(d);
                        if (d_len > 0.0001f)
                            d *= length(m_camera_target_vector_in_camera_space) / d_len;

                        m_camera->coordinate_system()->set_origin(center + d);
                    }
                    break;
                case CAMERA_CONTROLLER_LOOK_AT:
                    {
                        free_fly_report.rotated = true;
                        vector3  d = m_camera->coordinate_system()->origin() - center;
                        qtgl::look_at(*m_camera->coordinate_system(), center, length(d));
                    }
                    break;
                case CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT:
                    {
                        free_fly_report.translated = true;
                        free_fly_report.rotated = true;
                        qtgl::look_at(*m_camera->coordinate_system(), center, length(m_camera_target_vector_in_camera_space));
                }
                    break;
                default: UNREACHABLE(); break;
                }
            }
        }
        else
            free_fly_report += qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                                              seconds_from_previous_call, mouse_props(), keyboard_props());

        if (camera_controller_changed)
            call_listeners(simulator_notifications::camera_controller_changed());
        if (free_fly_report.translated)
            call_listeners(simulator_notifications::camera_position_updated());
        if (free_fly_report.rotated)
            call_listeners(simulator_notifications::camera_orientation_updated());

        get_collision_scene()->get_statistics().on_next_frame();

        if (!time_config().is_paused())
            perform_simulation_step(time_config().get_clipped_simulation_time_step_in_seconds(seconds_from_previous_call));
        else
            perform_scene_update(seconds_from_previous_call);
    }

static bool  update_retinas = false;
if (keyboard_props().was_just_released(qtgl::KEY_F2()))
    update_retinas = !update_retinas;

if (update_retinas)
    if (!time_config().is_paused())
        update_retina_of_agents_from_offscreen_images(
                time_config().get_clipped_simulation_time_step_in_seconds(seconds_from_previous_call)
                );

    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());
    qtgl::glapi().glPolygonMode(GL_FRONT_AND_BACK, m_render_in_wireframe ? GL_LINE : GL_FILL);

    matrix44  matrix_from_world_to_camera;
    m_camera->to_camera_space_matrix(matrix_from_world_to_camera);
    matrix44  matrix_from_camera_to_clipspace;
    m_camera->projection_matrix(matrix_from_camera_to_clipspace);

    qtgl::draw_state  draw_state;
    if (m_do_show_grid)
        if (qtgl::make_current(m_batch_grid, draw_state))
        {
            qtgl::render_batch(
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

    if (m_do_show_colliders)
        render_colliders(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    if (m_do_show_contact_normals)
        render_contact_normals(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    if (m_do_show_ai_action_controller_props)
        render_ai_action_controller_props(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);

    if (!time_config().is_paused())
        render_simulation_state(matrix_from_world_to_camera, matrix_from_camera_to_clipspace ,draw_state);
    else
    {
        if (m_do_show_batches)
            render_scene_batches(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
        render_scene_coord_systems(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    }

static bool  render_retinas = false;
if (keyboard_props().was_just_released(qtgl::KEY_F3()))
    render_retinas = !render_retinas;

if (render_retinas)
    for (auto const& agent_id_and_node_id : m_binding_of_agents_to_scene)
    {
        if (!m_agents_ptr->ready(agent_id_and_node_id.first))
            continue;

        ai::retina_ptr const  retina_ptr = m_agents_ptr->at(agent_id_and_node_id.first).get_blackboard()->m_retina_ptr;
        if (retina_ptr == nullptr)
            continue;

        auto  offscreen_it = m_offscreens.find(agent_id_and_node_id.first);
        if (offscreen_it == m_offscreens.cend())
            continue;

        float_32_bit const  scale = 3.0f;

        qtgl::dbg::draw_offscreen_depth_image(
            *retina_ptr->get_depth_image(),
            0U,
            0U,
            window_props().width_in_pixels(),
            window_props().height_in_pixels(),
            scale
            );
        if (retina_ptr->get_colour_image() != nullptr)
            qtgl::dbg::draw_offscreen_colour_image(
                *retina_ptr->get_colour_image(),
                (natural_32_bit)(scale * retina_ptr->get_depth_image()->get_width_in_pixels()),
                0U,
                window_props().width_in_pixels(),
                window_props().height_in_pixels(),
                scale
                );
    }

static bool  render_text = false;
if (keyboard_props().was_just_released(qtgl::KEY_F4()))
render_text = !render_text;

if (render_text)
{
    static std::string const  text =
        "! \" # $ % & ' ( ) * + , - . \n"
        "/ 0 1 2 3 4 5 6 7 8 9 : ; < \n"
        "= > ? @ A B C D E F G H I J \n"
        "K L M N O P Q R S T U V W X \n"
        "Y Z [ \\ ] ^ _ ` a b c d e f \n"
        "g h i j k l m n o p q r s t \n"
        "u v w x y z { | } ~ \n"
        "Hello World!\n"
        "The proximity map provides fast search for objects distributed in 3D space.\n"
        "A shape of an object is approximated by an axis aligned bounding box. The\n"
        "map this assumes, there is a fast algorithm obtaining a bounding box for\n"
        "any objects inserted into the map. Instead of requiring the type of objects\n"
        "contain a method for obtaining a bounding box, the map rather accepts two\n"
        "algorithms in the initialisation providing minimal and maximal corner points\n"
        "of a bounding box for any object in the map. For example, let type of our\n"
        "objects looks like this:\n"
        "\n"
        "     struct my_object3d_type\n"
        "     {\n"
        "         vector3 lo, hi; // min. and max. corner points of the bounding box.\n"
        "     };\n"
        "\n"
        "Then we can initialise a proximity map of these objects as follows:\n"
        "\n"
        "     angeo::proximity_map<my_object3d_type*> map(\n"
        "             [](my_object3d_type* obj) { return obj->lo; },  // getter for min. corner\n"
        "             [](my_object3d_type* obj) { return obj->hi; }   // getter for max. corner\n"
        "             );\n"
        "\n"
        "The typical usage of the map in a 3d simulation is as follows. In the initial\n"
        "step, we insert objects into the map. \n"
        "\n"
        "     std::vector<my_object3d_type*>  my_objects_to_simulate;\n"
        "     ...  // Create and initialise your objects\n"
        "     for (my_object3d_type* obj :  my_objects_to_simulate)\n"
        "         map.insert(obj);  // Insert objects into the map.\n"
        "     map.rebalance();  // Optimize the map structure for subsequent search operations.\n"
        "\n"
        "In the update step of the simulation (from the current state to the next one)\n"
        "you use the map to search for objects in desired space. For example, a search\n"
        "for objects in a 3d space denoted by an axis aligned bounding box will look\n"
        "like this:\n"
        "\n"
        "     vector3 query_bbox_min_corner, query_bbox_max_corner; // Defines 3d space where to search for objects\n"
        "     ...  // Initialise the corner points to desired values.\n"
        "     std::unordered_set<my_object3d_type*>  collected_objects;  // Will be filled in by found objects.\n"
        "     map.find_by_bbox(\n"
        "             query_bbox_min_corner,\n"
        "             query_bbox_max_corner,\n"
        "             [&collected_objects](my_object3d_type* obj) {\n"
        "                 collected_objects.insert(obj);\n"
        "                 return true; // Tell the map to continue the search for remaining objects (if any).\n"
        "                              // The return value 'false' would instruct the map to terminate the search.\n"
        "              });\n"
        ;
    static float_32_bit  scale = 1.0f;
    static vector3  shift{0.0f, -1.0f, 0.0f};
    static vector3  ambient_colour{ 0.9f, 0.9f, 0.95f };
    vector3 const  pos{
        m_camera->left() + scale * shift(0) * m_font_props.char_width,
        m_camera->top() + scale * shift(1)* m_font_props.char_height,
        -m_camera->near_plane()
    };
    qtgl::batch const  text_batch = qtgl::create_text(
        text,
        m_font_props,
        (m_camera->right() - pos(0)) / scale
        );
    if (qtgl::make_current(text_batch, draw_state))
    {
        matrix44 ortho;
        m_camera->projection_matrix_orthogonal(ortho);

        qtgl::render_batch(
            text_batch,
            pos,
            scale,
            ortho,
            ambient_colour
            );
        draw_state = text_batch.get_draw_state();
    }
}

    qtgl::swap_buffers();
}


void  simulator::on_simulation_paused()
{
    call_listeners(simulator_notifications::paused());
}


void  simulator::on_simulation_resumed()
{
    m_scene_selection.clear();
    m_scene_edit_data.invalidate_data();
    m_scene_history->clear();

    m_cache_of_batches_of_colliders.clear();
    m_cache_of_batches_of_ai_agents.clear();

    call_listeners(simulator_notifications::resumed());
}


void  simulator::perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    if (m_do_show_contact_normals)
    {
        if (m_cache_of_batches_of_colliders.collision_normals_points.operator bool())
            m_cache_of_batches_of_colliders.collision_normals_points->clear();
        else
            m_cache_of_batches_of_colliders.collision_normals_points =
                    std::make_unique<std::vector< std::pair<vector3, vector3> > >();
        m_cache_of_batches_of_colliders.collision_normals_batch.release();
    }
    if (m_do_show_ai_action_controller_props)
    {
        if (m_cache_of_batches_of_ai_agents.lines.operator bool())
        {
            m_cache_of_batches_of_ai_agents.lines->first.clear();
            m_cache_of_batches_of_ai_agents.lines->second.clear();
        }
        else
            m_cache_of_batches_of_ai_agents.lines =
                    std::make_unique<std::pair<std::vector< std::pair<vector3, vector3> >, std::vector<vector4> > >();
        m_cache_of_batches_of_ai_agents.lines_batch.release();
    }

    m_agents_ptr->next_round((float_32_bit)time_to_simulate_in_seconds, keyboard_props(), mouse_props(), window_props());

    perform_simulation_micro_step(time_to_simulate_in_seconds, true);

    //constexpr float_64_bit  min_micro_time_step_in_seconds = 0.001;
    //constexpr float_64_bit  max_micro_time_step_in_seconds = 0.04;
    //static float_64_bit  time_buffer_in_seconds = 0.0f;
    //static float_64_bit  duration_of_last_simulation_step_in_seconds = 0.01f;

    //time_buffer_in_seconds += time_to_simulate_in_seconds;
    //float_64_bit  max_computation_time_in_seconds = time_to_simulate_in_seconds / 4.0f;
    //while (time_buffer_in_seconds >= min_micro_time_step_in_seconds)
    //{
    //    natural_32_bit const  num_estimated_sub_steps =
    //            std::max(1U, (natural_32_bit)(max_computation_time_in_seconds / duration_of_last_simulation_step_in_seconds));
    //    float_32_bit const  micro_time_step_in_seconds =
    //            std::min(
    //                max_micro_time_step_in_seconds,
    //                std::max(min_micro_time_step_in_seconds, time_buffer_in_seconds / num_estimated_sub_steps)
    //                );
    //    bool const  is_last_micro_step = time_buffer_in_seconds - micro_time_step_in_seconds < min_micro_time_step_in_seconds;
    //    std::chrono::high_resolution_clock::time_point const  start_time_point =
    //            std::chrono::high_resolution_clock::now();


    //    perform_simulation_micro_step(micro_time_step_in_seconds, is_last_micro_step);


    //    time_buffer_in_seconds -= micro_time_step_in_seconds;
    //    duration_of_last_simulation_step_in_seconds =
    //            std::chrono::duration<float_64_bit>(std::chrono::high_resolution_clock::now() - start_time_point).count();
    //    max_computation_time_in_seconds -= duration_of_last_simulation_step_in_seconds;
    //}

    if (m_do_show_ai_action_controller_props)
    {
        for (auto const&  agent_id_and_node_id : m_binding_of_agents_to_scene)
            if (m_agents_ptr->ready(agent_id_and_node_id.first))
            {
                ai::blackboard_const_ptr const  blackboard = m_agents_ptr->at(agent_id_and_node_id.first).get_blackboard();

                scn::scene_node_ptr const  node_ptr = get_scene_node(blackboard->m_action_controller->get_motion_object_node_id());

                matrix44  motion_object_from_base_matrix;
                angeo::from_base_matrix(*node_ptr->get_coord_system(), motion_object_from_base_matrix);

                if (auto const  rb_ptr = scn::get_rigid_body(*node_ptr))
                {
                    vector3 const  motion_object_forward_direction_in_world_space =
                        transform_vector(blackboard->m_motion_templates.directions().forward(), motion_object_from_base_matrix);
                    vector3 const  motion_object_up_direction_in_world_space =
                        transform_vector(blackboard->m_motion_templates.directions().up(), motion_object_from_base_matrix);

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin(),
                            node_ptr->get_coord_system()->origin() + motion_object_forward_direction_in_world_space
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({0.25f, 0.75f, 0.75f, 1.0f}); // AQUA

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin(),
                            node_ptr->get_coord_system()->origin() + motion_object_up_direction_in_world_space
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({0.25f, 0.5f, 0.75f, 1.0f}); // AZURE

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin() + 0.0025f * vector3_unit_z(),
                            node_ptr->get_coord_system()->origin() + 0.0025f * vector3_unit_z()
                                + m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id())
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 1.0f, 0.25f, 1.0f}); // YELLOW

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin() - 0.0025f * motion_object_forward_direction_in_world_space,
                            node_ptr->get_coord_system()->origin() - 0.0025f * motion_object_forward_direction_in_world_space
                                + m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id())
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 0.5f, 0.25f, 1.0f}); // ORANGE
                }

                m_cache_of_batches_of_ai_agents.lines->first.push_back({
                        node_ptr->get_coord_system()->origin() + 0.005f * vector3_unit_z(),
                        node_ptr->get_coord_system()->origin() + 0.005f * vector3_unit_z()
                            + transform_vector(blackboard->m_cortex->get_motion_desire_props().forward_unit_vector_in_local_space,
                                               motion_object_from_base_matrix)
                        });
                m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 1.0f, 1.0f, 1.0f}); // WHITE

                m_cache_of_batches_of_ai_agents.lines->first.push_back({
                        node_ptr->get_coord_system()->origin() + 0.0075f * vector3_unit_z(),
                        node_ptr->get_coord_system()->origin() + 0.0075f * vector3_unit_z()
                            + 0.75f * transform_vector(blackboard->m_cortex->get_motion_desire_props().linear_velocity_unit_direction_in_local_space,
                                                       motion_object_from_base_matrix)
                        });
                m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 0.5f, 1.0f, 1.0f}); // PINK

                m_cache_of_batches_of_ai_agents.lines->first.push_back({
                        node_ptr->get_coord_system()->origin() + 0.01f * vector3_unit_z(),
                        node_ptr->get_coord_system()->origin() + 0.01f * vector3_unit_z()
                            + blackboard->m_cortex->get_motion_desire_props().linear_speed
                                * transform_vector(blackboard->m_cortex->get_motion_desire_props().linear_velocity_unit_direction_in_local_space,
                                                   motion_object_from_base_matrix)
                        });
                m_cache_of_batches_of_ai_agents.lines->second.push_back({0.75f, 0.25f, 0.75f, 1.0f}); // PURPLE

                for (natural_32_bit  bone : blackboard->m_action_controller->get_free_bones_for_look_at()->end_effector_bones)
                {
                    scn::scene_node_id const  raw_bone_id = detail::skeleton_build_scene_node_id_of_bones(
                            bone,
                            blackboard->m_motion_templates.hierarchy().parents(),
                            blackboard->m_motion_templates.names()
                            );
                    scn::scene_node_id const  bone_id = agent_id_and_node_id.second / raw_bone_id;
                    scn::scene_node_ptr const  bone_node_ptr = get_scene_node(bone_id);
                    if (bone_node_ptr != nullptr)
                    {
                        m_cache_of_batches_of_ai_agents.lines->first.push_back({
                                transform_point(vector3_zero(), bone_node_ptr->get_world_matrix()),
                                transform_point(5.0f * vector3_unit_y(), bone_node_ptr->get_world_matrix())
                                });
                        m_cache_of_batches_of_ai_agents.lines->second.push_back({ 0.85f, 0.85f, 0.85f, 1.0f }); // GRAY
                    }
                }
            }
    }
}


void  simulator::perform_simulation_micro_step(float_64_bit const  time_to_simulate_in_seconds, bool const  is_last_micro_step)
{
    TMPROF_BLOCK();

    auto const  ai_scene_binding = std::dynamic_pointer_cast<bind_ai_scene_to_simulator>(m_agents_ptr->get_scene_ptr());

    struct  contact_props
    {
        vector3  contact_point;
        vector3  unit_normal;
        angeo::COLLISION_MATERIAL_TYPE  material;
        float_32_bit  normal_force_magnitude;
    };
    std::unordered_map<angeo::collision_object_id, std::unordered_map<angeo::motion_constraint_system::constraint_id, contact_props> >
            ai_scene_binding_contacts;

    m_collision_scene_ptr->compute_contacts_of_all_dynamic_objects(
            [this, is_last_micro_step, ai_scene_binding, &ai_scene_binding_contacts](
                angeo::contact_id const& cid,
                vector3 const& contact_point,
                vector3 const& unit_normal,
                float_32_bit  penetration_depth) -> bool {
                    if (is_last_micro_step && m_do_show_contact_normals)
                        m_cache_of_batches_of_colliders.collision_normals_points->push_back({
                                contact_point, contact_point + 0.25f * unit_normal
                                });

                    angeo::collision_object_id const  coid_1 = angeo::get_object_id(angeo::get_first_collider_id(cid));
                    angeo::collision_object_id const  coid_2 = angeo::get_object_id(angeo::get_second_collider_id(cid));
                    angeo::COLLISION_MATERIAL_TYPE const  material_1 = m_collision_scene_ptr->get_material(coid_1);
                    angeo::COLLISION_MATERIAL_TYPE const  material_2 = m_collision_scene_ptr->get_material(coid_2);

                    bool const  track_coid_1 = ai_scene_binding->do_tracking_collision_contact_of_collision_object(coid_1);
                    bool const  track_coid_2 = ai_scene_binding->do_tracking_collision_contact_of_collision_object(coid_2);

                    auto const  rb_1_it = m_binding_of_collision_objects.find(coid_1);
                    auto const  rb_2_it = m_binding_of_collision_objects.find(coid_2);

                    if (rb_1_it == m_binding_of_collision_objects.cend() || rb_2_it == m_binding_of_collision_objects.cend())
                    {
                        if (track_coid_1)
                            ai_scene_binding->on_collision_contact(coid_1, contact_point, unit_normal, material_2, 0.0f);
                        if (track_coid_2)
                            ai_scene_binding->on_collision_contact(coid_2, contact_point, -unit_normal, material_1, 0.0f);
                        return true;
                    }

                    bool const  use_friction =
                            material_1 != angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING &&
                            material_2 != angeo::COLLISION_MATERIAL_TYPE::NO_FRINCTION_NO_BOUNCING ;
                    angeo::rigid_body_simulator::contact_friction_constraints_info  friction_info;
                    if (use_friction)
                    {
                        friction_info.m_unit_tangent_plane_vectors.resize(2UL);
                        angeo::compute_tangent_space_of_unit_vector(
                                unit_normal,
                                friction_info.m_unit_tangent_plane_vectors.front(),
                                friction_info.m_unit_tangent_plane_vectors.back()
                                );
                        friction_info.m_suppress_negative_directions = false;
                        friction_info.m_max_tangent_relative_speed_for_static_friction = 0.001f;
                    }
                    std::vector<angeo::motion_constraint_system::constraint_id>  output_constraint_ids;
                    m_rigid_body_simulator_ptr->insert_contact_constraints(
                            rb_1_it->second,
                            rb_2_it->second,
                            cid,
                            contact_point,
                            unit_normal,
                            material_1,
                            material_2,
                            use_friction ? &friction_info : nullptr,
                            penetration_depth,
                            20.0f,
                            (track_coid_1 || track_coid_2) ? &output_constraint_ids : nullptr
                            );
                    if (track_coid_1)
                    {
                        auto&  info_ref = ai_scene_binding_contacts[coid_1][output_constraint_ids.front()];
                        info_ref.contact_point = contact_point;
                        info_ref.unit_normal = unit_normal;
                        info_ref.material = material_2;
                    }
                    if (track_coid_2)
                    {
                        auto&  info_ref = ai_scene_binding_contacts[coid_2][output_constraint_ids.front()];
                        info_ref.contact_point = contact_point;
                        info_ref.unit_normal = -unit_normal;
                        info_ref.material = material_1;
                    }

                    return true;
                },
            true
            );
    m_rigid_body_simulator_ptr->solve_constraint_system(time_to_simulate_in_seconds, time_to_simulate_in_seconds);

    for (auto const&  coid_and_info : ai_scene_binding_contacts)
        for (auto const& constraint_id_and_info : coid_and_info.second)
            ai_scene_binding->on_collision_contact(
                    coid_and_info.first,
                    constraint_id_and_info.second.contact_point,
                    constraint_id_and_info.second.unit_normal,
                    constraint_id_and_info.second.material,
                    m_rigid_body_simulator_ptr->get_constraint_system().get_solution_of_constraint(constraint_id_and_info.first)
                    );

    m_rigid_body_simulator_ptr->integrate_motion_of_rigid_bodies(time_to_simulate_in_seconds);
    m_rigid_body_simulator_ptr->prepare_contact_cache_and_constraint_system_for_next_frame();
    for (auto const&  rb_id_and_node_ptr : m_binding_of_rigid_bodies)
    {
        angeo::rigid_body_id const  rb_id = rb_id_and_node_ptr.first;
        if (m_rigid_body_simulator_ptr->get_inverted_mass(rb_id) < 0.0001f)
            continue;
        scn::scene_node_ptr const  rb_node_ptr = rb_id_and_node_ptr.second;

        rb_node_ptr->relocate(
                m_rigid_body_simulator_ptr->get_position_of_mass_centre(rb_id),
                m_rigid_body_simulator_ptr->get_orientation(rb_id)
                );

        update_collider_locations_in_subtree(rb_node_ptr);
    }
}


void  simulator::update_retina_of_agents_from_offscreen_images(float_32_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    float_32_bit const  camera_FOV_angle = PI() / 2.0f;
    float_32_bit const  camera_proj_dist = 0.05f;
    float_32_bit const  camera_origin_z_shift = 0.0f;

    for (auto const&  agent_id_and_node_id : m_binding_of_agents_to_scene)
    {
        if (!m_agents_ptr->ready(agent_id_and_node_id.first))
            continue;

        ai::retina_ptr const  retina_ptr = m_agents_ptr->at(agent_id_and_node_id.first).get_blackboard()->m_retina_ptr;
        if (retina_ptr == nullptr)
            continue;

        auto  recovery_time_it = m_offscreen_recovery_times.find(agent_id_and_node_id.first);
        if (recovery_time_it == m_offscreen_recovery_times.end())
            recovery_time_it = m_offscreen_recovery_times.insert({ agent_id_and_node_id.first, 0.0f }).first;
        recovery_time_it->second += time_to_simulate_in_seconds;
        static float_32_bit  RECOVERY_PERIOD_IN_SECONDS = 0.0f;// 1.0f / 30.0f;
        if (recovery_time_it->second < RECOVERY_PERIOD_IN_SECONDS)
            continue;
        m_offscreen_recovery_times.erase(recovery_time_it);

        ai::sensory_controller_sight_ptr const  sight_ptr =
                m_agents_ptr->at(agent_id_and_node_id.first).get_sensory_controller().get_sight();
        if (sight_ptr == nullptr)
            continue;
        ai::sensory_controller_sight::camera_perspective_ptr const  camera_ptr = sight_ptr->get_camera();
        if (camera_ptr == nullptr)
            continue;

        auto  offscreen_it = m_offscreens.find(agent_id_and_node_id.first);
        if (offscreen_it == m_offscreens.cend())
            offscreen_it = m_offscreens.insert({
                    agent_id_and_node_id.first,
                    {
                        qtgl::make_offscreen(
                            retina_ptr->get_width_in_pixels(),
                            retina_ptr->get_height_in_pixels(),
                            retina_ptr->get_colour_image() != nullptr
                            ),
                        qtgl::make_offscreen(
                            retina_ptr->get_width_in_pixels(),
                            retina_ptr->get_height_in_pixels(),
                            retina_ptr->get_colour_image() != nullptr
                            )
                        }
                    }).first;
        else
        {
            TMPROF_BLOCK();

            qtgl::update_offscreen_depth_image(*retina_ptr->get_depth_image(), *offscreen_it->second.first);
            //qtgl::from_screen_space_to_camera_space(*retina_ptr->get_depth_image(), camera_it->second->near_plane(), camera_it->second->far_plane());
            //qtgl::from_camera_space_to_interval_01(*retina_ptr->get_depth_image(), camera_it->second->near_plane(), camera_it->second->far_plane());
            qtgl::normalise_offscreen_depth_image_interval_01(*retina_ptr->get_depth_image());
            if (retina_ptr->get_colour_image() != nullptr)
            {
                qtgl::update_offscreen_colour_image(*retina_ptr->get_colour_image(), *offscreen_it->second.first);
                //qtgl::modulate_offscreen_colour_image_by_depth_image(*retina_ptr->get_colour_image(), *retina_ptr->get_depth_image());
            }
        }

        matrix44  offscreen_matrix_from_world_to_camera;
        camera_ptr->to_camera_space_matrix(offscreen_matrix_from_world_to_camera);
        matrix44  offscreen_matrix_from_camera_to_clipspace;
        camera_ptr->projection_matrix(offscreen_matrix_from_camera_to_clipspace);
        qtgl::draw_state  offscreen_draw_state;

        {
            TMPROF_BLOCK();

            qtgl::make_current_offscreen const  offscreen_guard(*offscreen_it->second.second);

            qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            qtgl::glapi().glViewport(0, 0, offscreen_it->second.second->get_width_in_pixels(), offscreen_it->second.second->get_height_in_pixels());
            qtgl::glapi().glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

            render_scene_batches(offscreen_matrix_from_world_to_camera, offscreen_matrix_from_camera_to_clipspace, offscreen_draw_state);

            std::swap(offscreen_it->second.first, offscreen_it->second.second);
        }
    }
}


void  simulator::render_simulation_state(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    if (m_do_show_batches)
        render_scene_batches(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
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


    scn::scene_record_id const*  chosen_scene_object;
    {
        std::multimap<scalar, scn::scene_node_id>  nodes_on_line;
        scn::collision_scene_vs_line(get_scene(), ray_begin, ray_end, nodes_on_line);

        constexpr float_32_bit  RANGE_FROM_CLOSEST_IN_METERS = 50.0f;

        std::vector<scn::scene_record_id>  nearnest_records_in_range;
        collect_nearest_scene_objects_on_line_within_parameter_range(
                &nodes_on_line,
                nullptr,
                std::min(1.0f, RANGE_FROM_CLOSEST_IN_METERS / (length(ray_end - ray_begin) + 0.001f)),
                nearnest_records_in_range,
                nullptr
                );

        chosen_scene_object = m_scene_edit_data.get_selection_data().choose_best_selection(nearnest_records_in_range);
    }

    if (mouse_props().was_just_released(qtgl::RIGHT_MOUSE_BUTTON()) ||
        keyboard_props().is_pressed(qtgl::KEY_LCTRL()) ||
        keyboard_props().is_pressed(qtgl::KEY_RCTRL()))
    {
        if (chosen_scene_object == nullptr)
            return;

        if (chosen_scene_object->is_node_reference())
        {
            if (m_scene_selection.is_node_selected(chosen_scene_object->get_node_id()))
                m_scene_selection.erase_node(chosen_scene_object->get_node_id());
            else
                m_scene_selection.insert_node(chosen_scene_object->get_node_id());
        }
        else
        {
            if (m_scene_selection.is_record_selected(*chosen_scene_object))
                m_scene_selection.erase_record(*chosen_scene_object);
            else
                m_scene_selection.insert_record(*chosen_scene_object);
        }
    }
    else
    {
        m_scene_selection.clear();

        if (chosen_scene_object != nullptr)
        {
            if (chosen_scene_object->is_node_reference())
                m_scene_selection.insert_node(chosen_scene_object->get_node_id());
            else
                m_scene_selection.insert_record(*chosen_scene_object);
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
        {
            call_listeners(simulator_notifications::scene_node_position_update_finished());
            m_scene_edit_data.invalidate_data();
        }
        return;
    }
    if (m_scene_selection.empty())
        return;
    if (get_scene_edit_data().are_data_invalidated())
    {
        if (m_scene_selection.is_node_selected(scn::get_pivot_node_id()))
            m_scene_edit_data.initialise_translation_data({ 
                get_scene().get_scene_node(scn::get_pivot_node_id())->get_coord_system()->origin()
                });
        else
            m_scene_edit_data.initialise_translation_data(scn::get_center_of_selected_scene_nodes(m_scene_selection));
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

    std::unordered_set<scn::scene_node_id> nodes_to_translate = m_scene_selection.get_nodes();
    for (auto const& record_id : m_scene_selection.get_records())
        nodes_to_translate.insert(record_id.get_node_id());
    if (nodes_to_translate.size() > 1UL)
        nodes_to_translate.erase(scn::get_pivot_node_id());
    vector3 const  shift =
        m_scene_selection.is_node_selected(scn::get_pivot_node_id())
        && nodes_to_translate.count(scn::get_pivot_node_id()) == 0UL ?
                contract43(get_scene().get_scene_node(scn::get_pivot_node_id())->get_world_matrix()
                    * expand34(raw_shift, 0.0f)) :
                raw_shift;
    for (auto const& node_name : nodes_to_translate)
    {
        scn::scene_node_ptr const  node = get_scene_node(node_name);
        vector3 const  node_shift = node->has_parent() ?
            contract43(inverse44(node->get_parent()->get_world_matrix()) * expand34(shift, 0.0f)) :
            shift;
        node->translate(node_shift);

        on_relocation_of_scene_node(node);
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
        {
            call_listeners(simulator_notifications::scene_node_orientation_update_finished());
            m_scene_edit_data.invalidate_data();
        }
        return;
    }
    if (m_scene_selection.empty())
        return;

    if (get_scene_edit_data().are_data_invalidated())
    {
        if (m_scene_selection.is_node_selected(scn::get_pivot_node_id()))
            m_scene_edit_data.initialise_rotation_data({
                get_scene().get_scene_node(scn::get_pivot_node_id())->get_coord_system()->origin()
                });
        else
            m_scene_edit_data.initialise_rotation_data(scn::get_center_of_selected_scene_nodes(m_scene_selection));
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

    std::unordered_set<scn::scene_node_id> nodes_to_rotate = m_scene_selection.get_nodes();
    for (auto const& record_id : m_scene_selection.get_records())
        nodes_to_rotate.insert(record_id.get_node_id());
    if (nodes_to_rotate.size() > 1UL)
        nodes_to_rotate.erase(scn::get_pivot_node_id());
    bool const  rotate_around_pivot =
        m_scene_selection.is_node_selected(scn::get_pivot_node_id())
        && nodes_to_rotate.count(scn::get_pivot_node_id()) == 0UL;
    bool const  is_alternative_rotation_enabled =
        nodes_to_rotate.size() > 1UL && mouse_props().is_pressed(qtgl::RIGHT_MOUSE_BUTTON());
    vector3 const  axis = rotate_around_pivot ?
        contract43(get_scene().get_scene_node(scn::get_pivot_node_id())->get_world_matrix() * expand34(raw_axis, 0.0f)) :
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

        on_relocation_of_scene_node(node);
    }

    call_listeners(simulator_notifications::scene_node_orientation_updated());
}

void  simulator::render_scene_batches(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    using  batch_and_nodes_vector = std::pair<qtgl::batch, std::vector<scn::scene_node_ptr> >;
    using  vector_index_and_element_index = std::pair<natural_32_bit, natural_32_bit>;

    std::array<std::vector<batch_and_nodes_vector>, 2>  opaque_and_tanslucent_batches;
    std::unordered_map<std::string, vector_index_and_element_index> from_batch_id_to_indices;
    get_scene().foreach_node(
        [this, &from_batch_id_to_indices , &opaque_and_tanslucent_batches](scn::scene_node_ptr const  node_ptr) -> bool {
                for (auto const& name_holder : scn::get_batch_holders(*node_ptr))
                {
                    qtgl::batch const  batch = scn::as_batch(name_holder.second);
                    if (!batch.loaded_successfully())
                        continue;
                    auto  it = from_batch_id_to_indices.find(batch.key().get_unique_id());
                    if (it == from_batch_id_to_indices.end())
                    {
                        natural_32_bit const  vector_index = batch.is_translucent() ? 1U : 0U;
                        std::vector<batch_and_nodes_vector>&  nodes = opaque_and_tanslucent_batches.at(vector_index);
                        natural_32_bit const  element_index = (natural_32_bit)nodes.size();
                        nodes.push_back({batch, {node_ptr}});
                        it = from_batch_id_to_indices.insert({batch.key().get_unique_id(), {vector_index, element_index}}).first;
                    }
                    else
                        opaque_and_tanslucent_batches.at(it->second.first).at(it->second.second).second.push_back(node_ptr);
                }
                return true;
            },
        false
        );
    for (auto const& tasks : opaque_and_tanslucent_batches)
    for (auto const& batch_and_nodes : tasks)
    {
        bool const  use_instancing =
                batch_and_nodes.first.get_available_resources().skeletal() == nullptr &&
                batch_and_nodes.first.has_instancing_data() &&
                batch_and_nodes.second.size() > 1UL
                ;
        if (!qtgl::make_current(batch_and_nodes.first, draw_state, use_instancing))
            continue;

        if (use_instancing)
        {
            qtgl::vertex_shader_instanced_data_provider  instanced_data_provider(batch_and_nodes.first);
            for (auto const& node_ptr : batch_and_nodes.second)
                instanced_data_provider.insert_from_model_to_camera_matrix(
                        matrix_from_world_to_camera * node_ptr->get_world_matrix()
                        );
            qtgl::render_batch(
                    batch_and_nodes.first,
                    instanced_data_provider,
                    qtgl::vertex_shader_uniform_data_provider(
                            batch_and_nodes.first,
                            {},
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
        else
            for (auto const& node_ptr : batch_and_nodes.second)
            {
                ai::skeletal_motion_templates  motion_templates;
                {
                    scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
                    if (agent_ptr != nullptr && agent_ptr->get_skeleton_props() != nullptr)
                        motion_templates = agent_ptr->get_skeleton_props()->skeletal_motion_templates;
                }
                if (!motion_templates.loaded_successfully())
                {
                    qtgl::render_batch(
                            batch_and_nodes.first,
                            qtgl::vertex_shader_uniform_data_provider(
                                    batch_and_nodes.first,
                                    { matrix_from_world_to_camera * node_ptr->get_world_matrix() },
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
                    continue;
                }

                std::vector<matrix44>  frame;
                {
                    matrix44 alignment_matrix;
                    angeo::from_base_matrix(batch_and_nodes.first.get_skeleton_alignment().get_skeleton_alignment(), alignment_matrix);

                    std::vector<matrix44>  to_bone_matrices;
                    {
                        to_bone_matrices.resize(motion_templates.pose_frames().size());
                        for (natural_32_bit  bone = 0U; bone != motion_templates.pose_frames().size(); ++bone)
                        {
                            angeo::to_base_matrix(motion_templates.pose_frames().at(bone), to_bone_matrices.at(bone));
                            if (motion_templates.hierarchy().parents().at(bone) >= 0)
                                to_bone_matrices.at(bone) *= to_bone_matrices.at(motion_templates.hierarchy().parents().at(bone));
                        }
                    }

                    detail::skeleton_enumerate_nodes_of_bones(
                            node_ptr,
                            motion_templates,
                            [&frame, &matrix_from_world_to_camera, motion_templates, &alignment_matrix, &to_bone_matrices, node_ptr](
                                natural_32_bit const bone, scn::scene_node_ptr const  bone_node_ptr, bool const  has_parent) -> bool
                                {

                                    if (bone_node_ptr != nullptr)
                                        frame.push_back(
                                                matrix_from_world_to_camera *
                                                bone_node_ptr->get_world_matrix() *
                                                to_bone_matrices.at(bone) *
                                                alignment_matrix
                                                );
                                    else
                                    {
                                        // The deformable mesh is in inconsistent state: the controlling scene nodes
                                        // are not available; they were perhaps manually deleted from the scene bu the
                                        // user. The code below is supposed to render the object in default pose despite
                                        // of the inconsistency. The code is not optimal (slow) though.
                                        matrix44 result;
                                        angeo::from_base_matrix(motion_templates.pose_frames().at(bone), result);
                                        for (integer_32_bit  bone_idx = motion_templates.hierarchy().parents().at(bone);
                                             bone_idx >= 0;
                                             bone_idx = motion_templates.hierarchy().parents().at(bone_idx))
                                        {
                                            matrix44 M;
                                            angeo::from_base_matrix(motion_templates.pose_frames().at(bone_idx), M);
                                            result = M * result;
                                        }
                                        frame.push_back(
                                                matrix_from_world_to_camera *
                                                node_ptr->get_world_matrix() *
                                                result *
                                                to_bone_matrices.at(bone) *
                                                alignment_matrix
                                                );
                                    }
                                    return true;
                                }
                            );
                }

                qtgl::render_batch(
                        batch_and_nodes.first,
                        qtgl::vertex_shader_uniform_data_provider(
                                batch_and_nodes.first,
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
        draw_state = batch_and_nodes.first.get_draw_state();
    }
}

void  simulator::render_scene_coord_systems(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    //auto const  old_depth_test_state = qtgl::glapi().glIsEnabled(GL_DEPTH_TEST);
    //qtgl::glapi().glDisable(GL_DEPTH_TEST);

    std::unordered_set<scn::scene_node_id>  nodes_to_draw = m_scene_selection.get_nodes();
    scn::get_nodes_of_selected_records(m_scene_selection, nodes_to_draw);
    if (scn::has_node(get_scene(), scn::get_pivot_node_id())) // The pivot may be missing, if the scene is not completely initialised yet.
        nodes_to_draw.insert(scn::get_pivot_node_id());

    if (nodes_to_draw.empty())
        return;

    if (!qtgl::make_current(m_batch_coord_system, draw_state, true))
        return;

    qtgl::vertex_shader_instanced_data_provider  instanced_data_provider(m_batch_coord_system);
    for (auto const& node_name : nodes_to_draw)
        instanced_data_provider.insert_from_model_to_camera_matrix(
                matrix_from_world_to_camera * get_scene().get_scene_node(node_name)->get_world_matrix()
                );
    qtgl::render_batch(
        m_batch_coord_system,
        instanced_data_provider,
        qtgl::vertex_shader_uniform_data_provider(
            m_batch_coord_system,
            {},
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

void  simulator::render_colliders(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    GLint  backup_polygon_mode[2];
    qtgl::glapi().glGetIntegerv(GL_POLYGON_MODE, &backup_polygon_mode[0]);
    qtgl::glapi().glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    get_scene().foreach_node(
        [this, &matrix_from_world_to_camera, &matrix_from_camera_to_clipspace, &draw_state](scn::scene_node_ptr const  node_ptr)
            -> bool {
                if (auto const collider_ptr = scn::get_collider(*node_ptr))
                {
                    std::vector<qtgl::batch>  batches;
                    {
                        angeo::collision_object_id const  coid = collider_ptr->id();
                        switch (angeo::get_shape_type(coid))
                        {
                        case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                            {
                                std::pair<float_32_bit, float_32_bit> const  key {
                                        m_collision_scene_ptr->get_capsule_half_distance_between_end_points(coid),
                                        m_collision_scene_ptr->get_capsule_thickness_from_central_line(coid)
                                        };
                                auto  it = m_cache_of_batches_of_colliders.capsules.find(key);
                                if (it == m_cache_of_batches_of_colliders.capsules.end())
                                    it = m_cache_of_batches_of_colliders.capsules.insert({
                                                key,
                                                qtgl::create_wireframe_capsule(key.first, key.second, 5U, m_colliders_colour)
                                                }).first;
                                batches.push_back(it->second);
                            }
                            break;
                        case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                            {
                                float_32_bit const  key = m_collision_scene_ptr->get_sphere_radius(coid);
                                auto  it = m_cache_of_batches_of_colliders.spheres.find(key);
                                if (it == m_cache_of_batches_of_colliders.spheres.end())
                                    it = m_cache_of_batches_of_colliders.spheres.insert({
                                                key,
                                                qtgl::create_wireframe_sphere(key, 5U, m_colliders_colour)
                                                }).first;
                                batches.push_back(it->second);
                            }
                            break;
                        case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
                            {
                                detail::collider_triangle_mesh_vertex_getter const* const  vertices_getter_ptr =
                                    m_collision_scene_ptr->get_triangle_points_getter(coid)
                                                     .target<detail::collider_triangle_mesh_vertex_getter>();
                                std::string const  key =
                                    boost::filesystem::path(vertices_getter_ptr->get_vertex_buffer().key().get_unique_id())
                                            .parent_path().string();
                                auto  it = m_cache_of_batches_of_colliders.triangle_meshes.find(key);
                                if (it == m_cache_of_batches_of_colliders.triangle_meshes.end())
                                {
                                    qtgl::batch  triangles_batch;
                                    qtgl::batch  normals_batch;
                                    qtgl::batch  neighbours_batch;
                                    {
                                        matrix44 const  to_node_matrix = inverse44(node_ptr->get_world_matrix());

                                        std::vector<std::pair<vector3,vector3> >  lines;
                                        std::vector<vector4>  colours_of_lines;
                                        vector4 const  ignored_edge_colour = expand34(0.5f * contract43(m_colliders_colour), 1.0f);
                                        std::vector<std::pair<vector3, vector3> >  normals;
                                        std::vector<std::pair<vector3, vector3> >  neighbours;
                                        for (angeo::collision_object_id const  coid : collider_ptr->ids())
                                        {
                                            auto const&  getter = m_collision_scene_ptr->get_triangle_points_getter(coid);
                                            natural_32_bit const  triangle_index = m_collision_scene_ptr->get_triangle_index(coid);
                                            natural_8_bit const  edge_ignore_mask = m_collision_scene_ptr->get_trinagle_edges_ignore_mask(coid);

                                            std::array<vector3, 3U> const  P = {
                                                getter(triangle_index, 0U),
                                                getter(triangle_index, 1U),
                                                getter(triangle_index, 2U)
                                            };

                                            lines.push_back({ P[0], P[1] });
                                            colours_of_lines.push_back((edge_ignore_mask & 1U) == 0U ? m_colliders_colour : ignored_edge_colour);
                                            lines.push_back({ P[1], P[2] });
                                            colours_of_lines.push_back((edge_ignore_mask & 2U) == 0U ? m_colliders_colour : ignored_edge_colour);
                                            lines.push_back({ P[2], P[0] });
                                            colours_of_lines.push_back((edge_ignore_mask & 4U) == 0U ? m_colliders_colour : ignored_edge_colour);

                                            vector3 const  center = (1.0f / 3.0f) * (P[0] + P[1] + P[2]);

                                            normals.push_back({
                                                    center,
                                                    center + transform_vector(m_collision_scene_ptr->get_triangle_unit_normal_in_world_space(coid), to_node_matrix)
                                                    });

                                            for (natural_32_bit i = 0U; i != 3U; ++i)
                                                if (m_collision_scene_ptr->get_trinagle_neighbour_over_edge(coid, i) != coid)
                                                {
                                                    vector3 const  A = 0.5f * (P[i] + P[(i + 1U) % 3U]);
                                                    vector3  B;
                                                    if (false)
                                                        B = A + 0.25f * (center - A);
                                                    else
                                                    {
                                                        // This is only for debugging purposes (when suspicious about neighbours somewhere).
                                                        angeo::collision_object_id const  neighbour_coid =
                                                                m_collision_scene_ptr->get_trinagle_neighbour_over_edge(coid, i);
                                                        auto const& neighbour_getter =
                                                                m_collision_scene_ptr->get_triangle_points_getter(neighbour_coid);
                                                        natural_32_bit const  neighbour_triangle_index =
                                                                m_collision_scene_ptr->get_triangle_index(neighbour_coid);

                                                        std::array<vector3, 3U> const  neighbour_P = {
                                                            neighbour_getter(neighbour_triangle_index, 0U),
                                                            neighbour_getter(neighbour_triangle_index, 1U),
                                                            neighbour_getter(neighbour_triangle_index, 2U)
                                                        };
                                                        vector3 const  neighbour_center =
                                                            (1.0f / 3.0f) * (neighbour_P[0] + neighbour_P[1] + neighbour_P[2]);
                                                        B = A + 0.25f * (neighbour_center - A);
                                                    }
                                                    neighbours.push_back({ A, B });
                                                }
                                        }
                                        if (!lines.empty())
                                            triangles_batch = qtgl::create_lines3d(lines, colours_of_lines);
                                        if (!normals.empty())
                                            normals_batch = qtgl::create_lines3d(normals, m_colliders_colour);
                                        if (!neighbours.empty())
                                            neighbours_batch = qtgl::create_lines3d(neighbours, m_colliders_colour);
                                    }
                                    it = m_cache_of_batches_of_colliders.triangle_meshes.insert({
                                                key,
                                                { triangles_batch, normals_batch, neighbours_batch }
                                                }).first;
                                }
                                batches.push_back(it->second.triangles);
                                if (m_do_show_normals_of_collision_triangles)
                                    batches.push_back(it->second.normals);
                                if (m_do_show_neighbours_of_collision_triangles)
                                    batches.push_back(it->second.neighbours);
                            }
                            break;
                        default:
                            NOT_IMPLEMENTED_YET();
                        }
                    }
                    for (qtgl::batch  batch : batches)
                        if (qtgl::make_current(batch, draw_state))
                        {
                            qtgl::render_batch(
                                    batch,
                                    matrix_from_world_to_camera * node_ptr->get_world_matrix(),
                                    matrix_from_camera_to_clipspace
                                    );
                            draw_state = batch.get_draw_state();
                        }
                }
                return true;
            },
        false
        );

    qtgl::glapi().glPolygonMode(GL_FRONT_AND_BACK, backup_polygon_mode[0]);
}


void  simulator::render_contact_normals(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    if (m_cache_of_batches_of_colliders.collision_normals_points.operator bool())
    {
        if (m_cache_of_batches_of_colliders.collision_normals_points->empty())
            m_cache_of_batches_of_colliders.collision_normals_batch.release();
        else
            m_cache_of_batches_of_colliders.collision_normals_batch = qtgl::create_lines3d(
                *m_cache_of_batches_of_colliders.collision_normals_points,
                m_colliders_colour
                );
        m_cache_of_batches_of_colliders.collision_normals_points.release();
    }
    if (m_cache_of_batches_of_colliders.collision_normals_batch.empty())
        return;

    if (qtgl::make_current(m_cache_of_batches_of_colliders.collision_normals_batch, draw_state))
    {
        qtgl::render_batch(
                m_cache_of_batches_of_colliders.collision_normals_batch,
                matrix_from_world_to_camera,
                matrix_from_camera_to_clipspace
                );
        draw_state = m_cache_of_batches_of_colliders.collision_normals_batch.get_draw_state();
    }
}


void  simulator::render_ai_action_controller_props(
        matrix44 const&  matrix_from_world_to_camera,
        matrix44 const&  matrix_from_camera_to_clipspace,
        qtgl::draw_state&  draw_state
        )
{
    TMPROF_BLOCK();

    if (m_cache_of_batches_of_ai_agents.lines.operator bool())
    {
        if (m_cache_of_batches_of_ai_agents.lines->first.empty())
            m_cache_of_batches_of_ai_agents.lines_batch.release();
        else
            m_cache_of_batches_of_ai_agents.lines_batch = qtgl::create_lines3d(
                m_cache_of_batches_of_ai_agents.lines->first,
                m_cache_of_batches_of_ai_agents.lines->second
                );
        m_cache_of_batches_of_ai_agents.lines.release();
    }

    if (!m_do_show_ai_action_controller_props)
    {
        m_cache_of_batches_of_ai_agents.lines.release();
        m_cache_of_batches_of_ai_agents.lines_batch.release();
    }

    if (m_cache_of_batches_of_ai_agents.lines_batch.empty())
        return;

    if (qtgl::make_current(m_cache_of_batches_of_ai_agents.lines_batch, draw_state))
    {
        qtgl::render_batch(
                m_cache_of_batches_of_ai_agents.lines_batch,
                matrix_from_world_to_camera,
                matrix_from_camera_to_clipspace
                );
        draw_state = m_cache_of_batches_of_ai_agents.lines_batch.get_draw_state();
    }

    for (auto const&  agent_id_and_node_id : m_binding_of_agents_to_scene)
    {
        if (!m_agents_ptr->ready(agent_id_and_node_id.first))
            continue;
        ai::sensory_controller_sight_ptr const  sight_ptr = m_agents_ptr->at(agent_id_and_node_id.first).get_sensory_controller().get_sight();
        if (sight_ptr == nullptr)
            continue;
        ai::sensory_controller_sight::camera_perspective_ptr const  camera_ptr = sight_ptr->get_camera();
        if (camera_ptr == nullptr)
        {
            m_cache_of_batches_of_ai_agents.sight_frustum_batches.erase(agent_id_and_node_id.first);
            continue;
        }

        matrix44  from_sight_to_camera_matrix;
        {
            matrix44  W;
            angeo::from_base_matrix(*camera_ptr->coordinate_system(), W);
            from_sight_to_camera_matrix = matrix_from_world_to_camera * W;
        }

        if (qtgl::make_current(m_batch_coord_system, draw_state))
        {
            qtgl::render_batch(
                m_batch_coord_system,
                from_sight_to_camera_matrix,
                matrix_from_camera_to_clipspace
            );
            draw_state = m_batch_coord_system.get_draw_state();
        }

        auto  frustum_batch_it = m_cache_of_batches_of_ai_agents.sight_frustum_batches.find(agent_id_and_node_id.first);
        if (frustum_batch_it == m_cache_of_batches_of_ai_agents.sight_frustum_batches.end())
            frustum_batch_it = m_cache_of_batches_of_ai_agents.sight_frustum_batches.insert({
                    agent_id_and_node_id.first,
                    qtgl::create_wireframe_perspective_frustum(
                            -camera_ptr->near_plane(),
                            -camera_ptr->far_plane(),
                            camera_ptr->left(),
                            camera_ptr->right(),
                            camera_ptr->top(),
                            camera_ptr->bottom(),
                            vector4{0.5f, 0.5f, 0.5f, 1.0f},
                            true
                            )
                    }).first;

        if (qtgl::make_current(frustum_batch_it->second, draw_state))
        {
            qtgl::render_batch(
                frustum_batch_it->second,
                from_sight_to_camera_matrix,
                matrix_from_camera_to_clipspace
            );
            draw_state = frustum_batch_it->second.get_draw_state();
        }
    }

    bool const  use_instancing =
            __dbg_batch_coord_cross.get_available_resources().skeletal() == nullptr &&
            __dbg_batch_coord_cross.has_instancing_data()
            ;
    for (auto const& agent_id_and_node_id : m_binding_of_agents_to_scene)
    {
        if (!m_agents_ptr->ready(agent_id_and_node_id.first))
            continue;

        ai::sensory_controller_ray_cast_sight_ptr const  sight_ptr =
                std::dynamic_pointer_cast<ai::sensory_controller_ray_cast_sight>(
                        m_agents_ptr->at(agent_id_and_node_id.first).get_sensory_controller().get_sight()
                        );
        if (sight_ptr == nullptr)
            continue;
        ai::sensory_controller_ray_cast_sight::ray_casts_in_time const&  ray_casts_in_time = sight_ptr->get_ray_casts_in_time();
        if (ray_casts_in_time.empty())
            continue;

        if (!qtgl::make_current(__dbg_batch_coord_cross, draw_state, use_instancing))
            continue;
        INVARIANT(use_instancing);

        qtgl::vertex_shader_instanced_data_provider  instanced_data_provider(__dbg_batch_coord_cross);
        for (auto const&  time_and_ray_cast_info : ray_casts_in_time)
        {
            vector3 const  contact_point =
                    time_and_ray_cast_info.second.ray_origin_in_world_space +
                    time_and_ray_cast_info.second.parameter_to_coid * time_and_ray_cast_info.second.ray_unit_direction_in_world_space
                    ;
            matrix44  W;
            compose_from_base_matrix(contact_point, matrix33_identity(), W);
            instanced_data_provider.insert_from_model_to_camera_matrix(matrix_from_world_to_camera * W);
        }
        if (false) // do we want to draw also seen triangles?
            for (auto const&  time_and_ray_cast_info : ray_casts_in_time)
            {
                vector3 pos;
                switch (angeo::get_shape_type(time_and_ray_cast_info.second.coid))
                {
                case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
                    pos = (1.0f / 3.0f) * (
                        m_collision_scene_ptr->get_triangle_end_point_in_world_space(time_and_ray_cast_info.second.coid, 0U) +
                        m_collision_scene_ptr->get_triangle_end_point_in_world_space(time_and_ray_cast_info.second.coid, 1U) +
                        m_collision_scene_ptr->get_triangle_end_point_in_world_space(time_and_ray_cast_info.second.coid, 2U)
                        );
                    break;
                default:
                    continue;
                }
                matrix44  W;
                compose_from_base_matrix(pos, matrix33_identity(), W);
                instanced_data_provider.insert_from_model_to_camera_matrix(matrix_from_world_to_camera * W);
            }
        qtgl::render_batch(
                __dbg_batch_coord_cross,
                instanced_data_provider,
                qtgl::vertex_shader_uniform_data_provider(
                    __dbg_batch_coord_cross,
                    {},
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
}


scn::scene_node_ptr simulator::insert_scene_simulation_node(scn::scene_node_id const&  id)
{
    scn::scene_node_ptr const n = insert_scene_node(id);
    call_listeners(simulator_notifications::scene_simulation_nodes_changed());
    return n;
}


void simulator::erase_scene_simulation_node(scn::scene_node_id const&  id)
{
    erase_scene_node(id);
    call_listeners(simulator_notifications::scene_simulation_nodes_changed());
}


void  simulator::erase_scene_node(scn::scene_node_id const&  id)
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene().get_scene_node(id);
    for (auto const&  elem : node_ptr->get_children())
        erase_scene_node(elem.second->get_id());

    if (scn::has_rigid_body(*node_ptr))
        erase_rigid_body_from_scene_node(id);
    if (scn::has_collider(*node_ptr))
        erase_collision_object_from_scene_node(scn::make_collider_record_id(id, get_collider_record_name(*node_ptr)));

    m_scene_selection.erase_node(id);
    m_scene_selection.erase_records_of_node(id);

    m_scene_edit_data.invalidate_data();

    get_scene().erase_scene_node(id);
}


scn::scene_node_ptr  simulator::find_nearest_rigid_body_node(scn::scene_node_ptr  node_ptr)
{
    TMPROF_BLOCK();

    for ( ; node_ptr != nullptr; node_ptr = node_ptr->get_parent())
        if (scn::has_rigid_body(*node_ptr))
            break;
    return node_ptr;
}


void  simulator::find_nearest_rigid_body_nodes_in_subtree(scn::scene_node_ptr  node_ptr, std::vector<scn::scene_node_ptr>&  output)
{
    TMPROF_BLOCK();

    if (scn::has_rigid_body(*node_ptr))
        output.push_back(node_ptr);
    else
        for (auto const&  name_and_node : node_ptr->get_children())
            find_nearest_rigid_body_nodes_in_subtree(name_and_node.second, output);
}


void  simulator::foreach_collider_in_subtree(
            scn::scene_node_ptr const  node_ptr,
            std::function<void(scn::collider&,scn::scene_node_ptr )> const&  action
            )
{
    if (auto const  collider_ptr = scn::get_collider(*node_ptr))
        action(*collider_ptr, node_ptr);
    for (auto const  name_and_child_ptr : node_ptr->get_children())
        foreach_collider_in_subtree(name_and_child_ptr.second, action);
}

void  simulator::foreach_rigid_body_in_subtree(
            scn::scene_node_ptr const  node_ptr,
            std::function<void(scn::rigid_body&,scn::scene_node_ptr)> const&  action
            )
{
    if (auto const  rb_ptr = scn::get_rigid_body(*node_ptr))
        action(*rb_ptr, node_ptr);
    for (auto const name_and_child_ptr : node_ptr->get_children())
        foreach_rigid_body_in_subtree(name_and_child_ptr.second, action);
}


void  simulator::collect_colliders_in_subtree(
            scn::scene_node_ptr const  node_ptr,
            std::vector<angeo::collision_object_id>&  output,
            std::vector<scn::scene_node_ptr>* const  output_nodes
            )
{
    foreach_collider_in_subtree(
            node_ptr,
            [&output, output_nodes](scn::collider& collider, scn::scene_node_ptr const  collider_node_ptr) {
                    for (auto  coid : collider.ids()) {
                        output.push_back(coid);
                        if (output_nodes != nullptr)
                            output_nodes->push_back(collider_node_ptr);
                    }
                }
            );
}

void  simulator::update_collider_locations_in_subtree(scn::scene_node_ptr  node_ptr)
{
    foreach_collider_in_subtree(
            node_ptr,
            [this](scn::collider& collider, scn::scene_node_ptr const  node_ptr) {
                    for (auto  coid : collider.ids())
                        m_collision_scene_ptr->on_position_changed(coid, node_ptr->get_world_matrix());
                }
            );
}


void  simulator::insert_batch_to_scene_node(
        scn::scene_node::record_name const&  batch_name,
        boost::filesystem::path const&  batch_pathname,
        std::string const&  skin_name,
        qtgl::effects_config const  effects,
        scn::scene_node_id const&  scene_node_id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(scn::has_node(get_scene(), scene_node_id));
    auto const  batch = qtgl::batch(canonical_path(batch_pathname), effects, skin_name);
    scn::insert_batch(*get_scene_node(scene_node_id), batch_name, batch);
}

void  simulator::replace_batch_in_scene_node(
        scn::scene_node::record_name const&  batch_name,
        std::string const&  new_skin_name,
        qtgl::effects_config const  new_effects,
        scn::scene_node_id const&  scene_node_id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(scene_node_id);
    ASSUMPTION(node_ptr != nullptr);

    qtgl::batch const  old_batch = scn::get_batch(*node_ptr, batch_name);
    ASSUMPTION(!old_batch.empty());

    qtgl::batch const  new_batch = qtgl::batch(old_batch.get_path(), new_effects, new_skin_name);

    scn::erase_batch(*node_ptr, batch_name);
    scn::insert_batch(*node_ptr, batch_name, new_batch);
}

void  simulator::erase_batch_from_scene_node(
        scn::scene_node::record_name const&  batch_name,
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    m_scene_selection.erase_record(scn::make_batch_record_id(id, batch_name));
    scn::erase_batch(*get_scene_node(id), batch_name);
}


void  simulator::insert_collision_sphere_to_scene_node(
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic,
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    angeo::collision_object_id const  collider_id =
            m_collision_scene_ptr->insert_sphere(radius, node_ptr->get_world_matrix(), material, as_dynamic);
    scn::insert_collider(*node_ptr, id.get_record_name(), collider_id, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}

void  simulator::insert_collision_capsule_to_scene_node(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic,
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    angeo::collision_object_id const  collider_id =
            m_collision_scene_ptr->insert_capsule(
                    half_distance_between_end_points,
                    thickness_from_central_line,
                    node_ptr->get_world_matrix(),
                    material,
                    as_dynamic
                    );
    scn::insert_collider(*node_ptr, id.get_record_name(), collider_id, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}

void  simulator::insert_collision_trianle_mesh_to_scene_node(
        qtgl::buffer const  vertex_buffer,
        qtgl::buffer const  index_buffer,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        float_32_bit const  density_multiplier,
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    detail::collider_triangle_mesh_vertex_getter const  getter(vertex_buffer, index_buffer);
    std::vector<angeo::collision_object_id>  collider_ids;
    m_collision_scene_ptr->insert_triangle_mesh(
            index_buffer.num_primitives(),
            getter,
            node_ptr->get_world_matrix(),
            material,
            false,
            collider_ids
            );

    scn::insert_collider(*node_ptr, id.get_record_name(), collider_ids, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}

void  simulator::erase_collision_object_from_scene_node(
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    auto const  collider_ptr = scn::get_collider(*node_ptr);
    ASSUMPTION(collider_ptr != nullptr);
    bool  has_dynamic = false;
    for (auto  coid : collider_ptr->ids())
    {
        if (m_collision_scene_ptr->is_dynamic(coid))
            has_dynamic = true;
        m_collision_scene_ptr->erase_object(coid);
        m_binding_of_collision_objects.erase(coid);
    }
    m_scene_selection.erase_record(id);
    scn::erase_collider(get_scene(), id);

    if (has_dynamic)
        if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
            rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}


void  simulator::get_collision_sphere_info(
        scn::scene_record_id const&  id,
        float_32_bit&  radius,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        float_32_bit&  density_multiplier,
        bool&  is_dynamic
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    radius = m_collision_scene_ptr->get_sphere_radius(collider->id());
    material = m_collision_scene_ptr->get_material(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene_ptr->is_dynamic(collider->id());
}

void  simulator::get_collision_capsule_info(
        scn::scene_record_id const&  id,
        float_32_bit&  half_distance_between_end_points,
        float_32_bit&  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        float_32_bit&  density_multiplier,
        bool&  is_dynamic
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    half_distance_between_end_points = m_collision_scene_ptr->get_capsule_half_distance_between_end_points(collider->id());
    thickness_from_central_line = m_collision_scene_ptr->get_capsule_thickness_from_central_line(collider->id());
    material = m_collision_scene_ptr->get_material(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene_ptr->is_dynamic(collider->id());
}


void  simulator::get_collision_triangle_mesh_info(
        scn::scene_record_id const&  id,
        qtgl::buffer&  vertex_buffer,
        qtgl::buffer&  index_buffer,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        float_32_bit&  density_multiplier
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    detail::collider_triangle_mesh_vertex_getter const* const  vertices_getter_ptr =
            m_collision_scene_ptr->get_triangle_points_getter(collider->id()).target<detail::collider_triangle_mesh_vertex_getter>();
    vertex_buffer = vertices_getter_ptr->get_vertex_buffer();
    index_buffer = vertices_getter_ptr->get_index_buffer();
    material = m_collision_scene_ptr->get_material(collider->id());
    density_multiplier = collider->get_density_multiplier();
}


void  simulator::insert_rigid_body_to_scene_node(
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration,
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id);
    ASSUMPTION(node_ptr != nullptr && !node_ptr->has_parent());

    std::vector<angeo::collision_object_id>  coids;
    std::vector<scn::scene_node_ptr>  coid_nodes;
    bool  has_static_collider;
    {
        collect_colliders_in_subtree(node_ptr, coids, &coid_nodes);

        has_static_collider = coids.empty();
        for (angeo::collision_object_id coid : coids)
            if (!m_collision_scene_ptr->is_dynamic(coid))
            {
                has_static_collider = true;
                break;
            }
        if (!has_static_collider)
            for (std::size_t i = 0U; i < coids.size(); ++i)
                for (std::size_t j = i + 1U; j < coids.size(); ++j)
                    m_collision_scene_ptr->disable_colliding(coids.at(i), coids.at(j));
    }

    float_32_bit  inverted_mass = 0.0f;
    matrix33  inverted_inertia_tensor_in_local_space = matrix33_zero();
    if (!has_static_collider)
    {
        angeo::mass_and_inertia_tensor_builder  builder;
        for (std::size_t i = 0UL; i != coids.size(); ++i)
        {
            angeo::collision_object_id const  coid = coids.at(i);
            scn::scene_node_ptr const  coid_node_ptr = coid_nodes.at(i);
            switch (angeo::get_shape_type(coid))
            {
            case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                builder.insert_capsule(
                        m_collision_scene_ptr->get_capsule_half_distance_between_end_points(coid),
                        m_collision_scene_ptr->get_capsule_thickness_from_central_line(coid),
                        coid_node_ptr->get_world_matrix(),
                        m_collision_scene_ptr->get_material(coid),
                        1.0f // TODO!
                        );
                break;
            case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                builder.insert_sphere(
                        translation_vector(coid_node_ptr->get_world_matrix()),
                        m_collision_scene_ptr->get_sphere_radius(coid),
                        m_collision_scene_ptr->get_material(coid),
                        1.0f // TODO!
                        );
                break;
            default:
                NOT_IMPLEMENTED_YET();
                break;
            }
        }
        vector3  center_of_mass_in_world_space;
        builder.run(inverted_mass, inverted_inertia_tensor_in_local_space, center_of_mass_in_world_space);

        vector3 const  origin_shift_in_world_space =
                    center_of_mass_in_world_space - translation_vector(node_ptr->get_world_matrix());
        vector3 const  origin_shift_in_local_space =
                    transform_vector(origin_shift_in_world_space, inverse44(node_ptr->get_world_matrix()));
        node_ptr->translate(origin_shift_in_world_space);
        node_ptr->foreach_child(
                [&origin_shift_in_local_space](scn::scene_node_ptr const  child_node_ptr) -> bool {
                        child_node_ptr->translate(-origin_shift_in_local_space);
                        return true;
                    },
                false
                );
    }

    angeo::rigid_body_id const  rb_id =
            m_rigid_body_simulator_ptr->insert_rigid_body(
                    node_ptr->get_coord_system()->origin(),
                    node_ptr->get_coord_system()->orientation(),
                    inverted_mass,
                    inverted_inertia_tensor_in_local_space,
                    has_static_collider ? vector3_zero() : linear_velocity,
                    has_static_collider ? vector3_zero() : angular_velocity,
                    has_static_collider ? vector3_zero() : external_linear_acceleration,
                    has_static_collider ? vector3_zero() : external_angular_acceleration
                    );

    scn::insert_rigid_body(*node_ptr, rb_id, true);
    m_binding_of_rigid_bodies[rb_id] = node_ptr;

    for (angeo::collision_object_id coid : coids)
        m_binding_of_collision_objects[coid] = rb_id;
}

void  simulator::insert_rigid_body_to_scene_node_ex(
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration,
        float_32_bit const  mass_inverted,
        matrix33 const&  inertia_tensor_inverted,
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id);
    ASSUMPTION(node_ptr != nullptr && !node_ptr->has_parent());

    std::vector<angeo::collision_object_id>  coids;
    bool  has_static_collider;
    {
        collect_colliders_in_subtree(node_ptr, coids);

        has_static_collider = coids.empty();
        for (angeo::collision_object_id coid : coids)
            if (!m_collision_scene_ptr->is_dynamic(coid))
            {
                has_static_collider = true;
                break;
            }
        if (!has_static_collider)
            for (std::size_t i = 0U; i < coids.size(); ++i)
                for (std::size_t j = i + 1U; j < coids.size(); ++j)
                    m_collision_scene_ptr->disable_colliding(coids.at(i), coids.at(j));
    }

    angeo::rigid_body_id const  rb_id =
            m_rigid_body_simulator_ptr->insert_rigid_body(
                    node_ptr->get_coord_system()->origin(),
                    node_ptr->get_coord_system()->orientation(),
                    mass_inverted,
                    inertia_tensor_inverted,
                    linear_velocity,
                    angular_velocity,
                    external_linear_acceleration,
                    external_angular_acceleration
                    );

    scn::insert_rigid_body(*node_ptr, rb_id, false);
    m_binding_of_rigid_bodies[rb_id] = node_ptr;

    for (angeo::collision_object_id coid : coids)
        m_binding_of_collision_objects[coid] = rb_id;
}


void  simulator::erase_rigid_body_from_scene_node(
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene_node(id);
    ASSUMPTION(node_ptr != nullptr);
    if (auto const  rb_ptr = scn::get_rigid_body(*node_ptr))
    {
        std::vector<angeo::collision_object_id>  coids;
        collect_colliders_in_subtree(node_ptr, coids);
        for (angeo::collision_object_id coid : coids)
            m_binding_of_collision_objects.erase(coid);
        bool  has_static_collider = false;
        for (angeo::collision_object_id coid : coids)
            if (!m_collision_scene_ptr->is_dynamic(coid))
            {
                has_static_collider = true;
                break;
            }
        if (!has_static_collider)
            for (std::size_t i = 0U; i < coids.size(); ++i)
                for (std::size_t j = i + 1U; j < coids.size(); ++j)
                    m_collision_scene_ptr->enable_colliding(coids.at(i), coids.at(j));

        m_rigid_body_simulator_ptr->erase_rigid_body(rb_ptr->id());
        m_binding_of_rigid_bodies.erase(rb_ptr->id());

        m_scene_selection.erase_record(scn::make_rigid_body_record_id(id));
        scn::erase_rigid_body(*node_ptr);
    }
}


void  simulator::rebuild_rigid_body_due_to_change_in_subtree(scn::scene_node_ptr const  phs_node_ptr)
{
    ASSUMPTION(phs_node_ptr != nullptr);
    auto  rb_ptr = scn::get_rigid_body(*phs_node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    if (!rb_ptr->auto_compute_mass_and_inertia_tensor())
    {
        std::vector<angeo::collision_object_id>  coids;
        collect_colliders_in_subtree(phs_node_ptr, coids, nullptr);
        for (angeo::collision_object_id coid : coids)
            m_binding_of_collision_objects[coid] = rb_ptr->id();
        return;
    }

    scn::rigid_body_props  rb_backup;
    rb_backup.m_linear_velocity = m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id());
    rb_backup.m_angular_velocity = m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id());
    rb_backup.m_external_linear_acceleration = m_rigid_body_simulator_ptr->get_external_linear_acceleration(rb_ptr->id());
    rb_backup.m_external_angular_acceleration = m_rigid_body_simulator_ptr->get_external_angular_acceleration(rb_ptr->id());

    erase_rigid_body_from_scene_node(phs_node_ptr->get_id());
    insert_rigid_body_to_scene_node(
            rb_backup.m_linear_velocity,
            rb_backup.m_angular_velocity,
            rb_backup.m_external_linear_acceleration,
            rb_backup.m_external_angular_acceleration,
            phs_node_ptr->get_id()
            );
}


void  simulator::get_rigid_body_info(
        scn::scene_node_id const&  id,
        bool&  auto_compute_mass_and_inertia_tensor,
        scn::rigid_body_props&  props
        )
{
    TMPROF_BLOCK();

    scn::scene_node_const_ptr const  node_ptr = get_scene_node(id);
    ASSUMPTION(node_ptr != nullptr);
    scn::rigid_body const* const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    auto_compute_mass_and_inertia_tensor = rb_ptr->auto_compute_mass_and_inertia_tensor();
    props.m_linear_velocity = m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id());
    props.m_angular_velocity = m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id());
    props.m_external_linear_acceleration = m_rigid_body_simulator_ptr->get_external_linear_acceleration(rb_ptr->id());
    props.m_external_angular_acceleration = m_rigid_body_simulator_ptr->get_external_angular_acceleration(rb_ptr->id());
    props.m_mass_inverted = m_rigid_body_simulator_ptr->get_inverted_mass(rb_ptr->id());
    props.m_inertia_tensor_inverted = m_rigid_body_simulator_ptr->get_inverted_inertia_tensor_in_local_space(rb_ptr->id());
}


void  simulator::insert_agent(scn::scene_record_id const&  id, scn::skeleton_props_const_ptr const  props)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr && !node_ptr->has_parent());
    ai::agent_id const  agent_id =
            m_agents_ptr->insert(
                    id.get_node_id(),
                    props->skeletal_motion_templates,
                    ai::make_retina(100U, 100U, true)
                    );
    scn::insert_agent(*node_ptr, agent_id, props);
    m_binding_of_agents_to_scene[agent_id] = id.get_node_id();
}


void  simulator::erase_agent(scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
    ASSUMPTION(agent_ptr != nullptr);

    m_agents_ptr->erase(agent_ptr->id());
    m_binding_of_agents_to_scene.erase(agent_ptr->id());

    m_scene_selection.erase_record(id);
    scn::erase_agent(*node_ptr);
}


void  simulator::load_collider(boost::property_tree::ptree const&  data, scn::scene_node_id const&  id)
{
    TMPROF_BLOCK();

    std::string const  shape_type = data.get<std::string>("shape_type");
    if (shape_type == "capsule")
        insert_collision_capsule_to_scene_node(
                data.get<float_32_bit>("half_distance_between_end_points"),
                data.get<float_32_bit>("thickness_from_central_line"),
                angeo::read_collison_material_from_string(data.get<std::string>("material")),
                data.get<float_32_bit>("density_multiplier"),
                data.get<bool>("is_dynamic"),
                scn::make_collider_record_id(id, shape_type)
                );
    else if (shape_type == "sphere")
        insert_collision_sphere_to_scene_node(
                data.get<float_32_bit>("radius"),
                angeo::read_collison_material_from_string(data.get<std::string>("material")),
                data.get<float_32_bit>("density_multiplier"),
                data.get<bool>("is_dynamic"),
                scn::make_collider_record_id(id, shape_type)
                );
    else if (shape_type == "triangle mesh")
    {
        boost::filesystem::path const  buffers_dir = data.get<std::string>("buffers_directory");
        qtgl::buffer  vertex_buffer(buffers_dir / "vertices.txt", std::numeric_limits<async::load_priority_type>::max());
        qtgl::buffer  index_buffer(buffers_dir / "indices.txt", std::numeric_limits<async::load_priority_type>::max());
        if (!vertex_buffer.wait_till_load_is_finished())
            throw std::runtime_error("Load of file 'vertices.txt' under directory '" + buffers_dir.string() + "' for 'triangle mesh' collider.");
        if (!index_buffer.wait_till_load_is_finished())
            throw std::runtime_error("Load of file 'indices.txt' under directory '" + buffers_dir.string() + "' for 'triangle mesh' collider.");
        insert_collision_trianle_mesh_to_scene_node(
                vertex_buffer,
                index_buffer,
                angeo::read_collison_material_from_string(data.get<std::string>("material")),
                data.get<float_32_bit>("density_multiplier"),
                scn::make_collider_record_id(id, shape_type)
                );
    }
    else
    {
        NOT_IMPLEMENTED_YET();
    }
}

void  simulator::save_collider(scn::collider const&  collider, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    switch (angeo::get_shape_type(collider.id()))
    {
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        data.put("shape_type", "capsule");
        data.put("half_distance_between_end_points", m_collision_scene_ptr->get_capsule_half_distance_between_end_points(collider.id()));
        data.put("thickness_from_central_line", m_collision_scene_ptr->get_capsule_thickness_from_central_line(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        data.put("shape_type", "sphere");
        data.put("radius", m_collision_scene_ptr->get_sphere_radius(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
        {
            data.put("shape_type", "triangle mesh");
            detail::collider_triangle_mesh_vertex_getter const* const  vertices_getter_ptr =
                m_collision_scene_ptr->get_triangle_points_getter(collider.id()).target<detail::collider_triangle_mesh_vertex_getter>();
            data.put(
                "buffers_directory",
                boost::filesystem::path(vertices_getter_ptr->get_vertex_buffer().key().get_unique_id()).parent_path().string()
                );
        }
        break;
    default:
        NOT_IMPLEMENTED_YET();
        break;
    }

    data.put("material", to_string(m_collision_scene_ptr->get_material(collider.id())));
    data.put("is_dynamic", m_collision_scene_ptr->is_dynamic(collider.id()));
    data.put("density_multiplier", collider.get_density_multiplier());
}


void  simulator::load_rigid_body(
        boost::property_tree::ptree const&  data,
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    auto const  load_vector = [&data](std::string const&  key) -> vector3 {
        boost::property_tree::path const  key_path(key, '/');
        return vector3(data.get<float_32_bit>(key_path / "x"),
                       data.get<float_32_bit>(key_path / "y"),
                       data.get<float_32_bit>(key_path / "z"));
    };

    auto const  load_matrix33 = [&data](std::string const&  key, matrix33&  M) -> void {
        boost::property_tree::path const  key_path(key, '/');
        M(0,0) = data.get<float_32_bit>(key_path / "00");
        M(0,1) = data.get<float_32_bit>(key_path / "00");
        M(0,2) = data.get<float_32_bit>(key_path / "00");
        M(1,0) = data.get<float_32_bit>(key_path / "10");
        M(1,1) = data.get<float_32_bit>(key_path / "11");
        M(1,2) = data.get<float_32_bit>(key_path / "12");
        M(2,0) = data.get<float_32_bit>(key_path / "20");
        M(2,1) = data.get<float_32_bit>(key_path / "21");
        M(2,2) = data.get<float_32_bit>(key_path / "22");
    };

    if (data.count("mass_inverted") != 0)
    {
        matrix33  inertia_inverted;
        load_matrix33("inertia_tensor_inverted", inertia_inverted);
        insert_rigid_body_to_scene_node_ex(
                load_vector("linear_velocity"),
                load_vector("angular_velocity"),
                load_vector("external_linear_acceleration"),
                load_vector("external_angular_acceleration"),
                data.get<float_32_bit>("mass_inverted"),
                inertia_inverted,
                id
                );
    }
    else
        insert_rigid_body_to_scene_node(
                load_vector("linear_velocity"),
                load_vector("angular_velocity"),
                load_vector("external_linear_acceleration"),
                load_vector("external_angular_acceleration"),
                id
                );
}

void  simulator::save_rigid_body(scn::scene_node_id const&  id, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene_node(id);
    auto const  rb_ptr = scn::get_rigid_body(*node_ptr);
    INVARIANT(rb_ptr != nullptr);

    auto const  save_vector = [&data](std::string const&  key, vector3 const&  u) -> void {
        boost::property_tree::path const  key_path(key, '/');
        data.put(key_path / "x", u(0));
        data.put(key_path / "y", u(1));
        data.put(key_path / "z", u(2));
    };

    auto const  save_matrix33 = [&data](std::string const&  key, matrix33 const&  M) -> void {
        boost::property_tree::path const  key_path(key, '/');
        data.put(key_path / "00", M(0,0));
        data.put(key_path / "00", M(0,1));
        data.put(key_path / "00", M(0,2));
        data.put(key_path / "10", M(1,0));
        data.put(key_path / "11", M(1,1));
        data.put(key_path / "12", M(1,2));
        data.put(key_path / "20", M(2,0));
        data.put(key_path / "21", M(2,1));
        data.put(key_path / "22", M(2,2));
    };

    if (rb_ptr->auto_compute_mass_and_inertia_tensor() == false)
    {
        data.put("mass_inverted", m_rigid_body_simulator_ptr->get_inverted_mass(rb_ptr->id()));
        save_matrix33("inertia_tensor_inverted", m_rigid_body_simulator_ptr->get_inverted_inertia_tensor_in_local_space(rb_ptr->id()));
    }

    save_vector("linear_velocity", m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id()));
    save_vector("angular_velocity", m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id()));
    save_vector("external_linear_acceleration", m_rigid_body_simulator_ptr->get_external_linear_acceleration(rb_ptr->id()));
    save_vector("external_angular_acceleration", m_rigid_body_simulator_ptr->get_external_angular_acceleration(rb_ptr->id()));
}


void  simulator::load_agent(boost::property_tree::ptree const&  data, scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  skeleton_dir = data.get<std::string>("skeleton_dir");
    ai::skeletal_motion_templates const skeletal_motion_templates(skeleton_dir, 75U);
    scn::skeleton_props_ptr const  props = scn::create_skeleton_props(skeleton_dir, skeletal_motion_templates);
    insert_agent(id, props);
}


void  simulator::save_agent(scn::scene_node_ptr const  node_ptr, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    scn::agent* const  agent_ptr = scn::get_agent(*node_ptr);
    ASSUMPTION(agent_ptr != nullptr);
    data.put("skeleton_dir", agent_ptr->get_skeleton_props()->skeleton_directory.string());
}


scn::skeleton_props_const_ptr  simulator::get_agent_info(scn::scene_node_id const& id)
{
    return scn::get_agent(*get_scene_node(id))->get_skeleton_props();
}


void  simulator::clear_scene()
{
    m_scene_selection.clear();
    m_scene_edit_data.invalidate_data();
    m_cache_of_batches_of_colliders.capsules.clear();
    m_cache_of_batches_of_colliders.spheres.clear();
    m_cache_of_batches_of_colliders.triangle_meshes.clear();
    m_cache_of_batches_of_colliders.collision_normals_points.release();
    m_cache_of_batches_of_colliders.collision_normals_batch.release();

    m_cache_of_batches_of_ai_agents.lines.release();
    m_cache_of_batches_of_ai_agents.lines_batch.release();
    m_cache_of_batches_of_ai_agents.sight_frustum_batches.clear();

    m_agents_ptr->clear();
    m_binding_of_agents_to_scene.clear();

    m_rigid_body_simulator_ptr->clear();
    m_binding_of_rigid_bodies.clear();

    m_collision_scene_ptr->clear();
    m_binding_of_collision_objects.clear();

    get_scene().clear();
}


void  simulator::set_position_of_scene_node(scn::scene_node_id const&  id, vector3 const&  new_origin)
{
    auto const  node_ptr = get_scene_node(id);
    node_ptr->set_origin(new_origin);
    m_scene_edit_data.invalidate_data();
    on_relocation_of_scene_node(node_ptr);
}

void  simulator::set_orientation_of_scene_node(scn::scene_node_id const&  id, quaternion const&  new_orientation)
{
    auto const  node_ptr = get_scene_node(id);
    node_ptr->set_orientation(new_orientation);
    m_scene_edit_data.invalidate_data();
    on_relocation_of_scene_node(node_ptr);
}

void  simulator::relocate_scene_node(scn::scene_node_id const&  id, vector3 const&  new_origin, quaternion const&  new_orientation)
{
    auto const  node_ptr = get_scene_node(id);
    node_ptr->relocate(new_origin, new_orientation);
    m_scene_edit_data.invalidate_data();
    on_relocation_of_scene_node(node_ptr);
}


void  simulator::on_relocation_of_scene_node(scn::scene_node_ptr const  node_ptr)
{
    std::vector<angeo::collision_object_id>  coids;
    std::vector<scn::scene_node_ptr>  coid_nodes;
    collect_colliders_in_subtree(node_ptr, coids, &coid_nodes);
    for (std::size_t i = 0UL; i != coids.size(); ++i)
        m_collision_scene_ptr->on_position_changed(coids.at(i), coid_nodes.at(i)->get_world_matrix());

    if (coids.empty())
        return;

    scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr);
    if (phs_node == nullptr)
        return;

    if (phs_node == node_ptr)
    {
        INVARIANT(!phs_node->has_parent());
        auto  rb_ptr = scn::get_rigid_body(*node_ptr);
        INVARIANT(rb_ptr != nullptr);
        m_rigid_body_simulator_ptr->set_position_of_mass_centre(rb_ptr->id(), phs_node->get_coord_system()->origin());
        m_rigid_body_simulator_ptr->set_orientation(rb_ptr->id(), phs_node->get_coord_system()->orientation());
    }
    else
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}


void  simulator::set_scene_selection(
        std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
        std::unordered_set<scn::scene_record_id> const&  selected_records
        )
{
    TMPROF_BLOCK();

    m_scene_selection.clear();
    insert_to_scene_selection(selected_scene_nodes, selected_records);
}

void  simulator::insert_to_scene_selection(
        std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
        std::unordered_set<scn::scene_record_id> const&  selected_records
        )
{
    TMPROF_BLOCK();

    for (auto const& name : selected_scene_nodes)
        m_scene_selection.insert_node(name);
    for (auto const& id : selected_records)
        m_scene_selection.insert_record(id);

    m_scene_edit_data.invalidate_data();
}

void  simulator::erase_from_scene_selection(
        std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
        std::unordered_set<scn::scene_record_id> const&  selected_records
        )
{
    TMPROF_BLOCK();

    for (auto const& name : selected_scene_nodes)
        m_scene_selection.erase_node(name);
    for (auto const& id : selected_records)
        m_scene_selection.erase_record(id);

    m_scene_edit_data.invalidate_data();
}

void  simulator::get_scene_selection(
        std::unordered_set<scn::scene_node_id>&  selected_scene_nodes,
        std::unordered_set<scn::scene_record_id>&  selected_records
        ) const
{
    TMPROF_BLOCK();

    selected_scene_nodes = m_scene_selection.get_nodes();
    selected_records = m_scene_selection.get_records();
}


void  simulator::set_scene_edit_mode(scn::SCENE_EDIT_MODE const  edit_mode)
{
    if (m_scene_edit_data.get_mode() != edit_mode)
    {
        m_scene_edit_data.set_mode(edit_mode);
        call_listeners(simulator_notifications::scene_edit_mode_changed());
    }
}
