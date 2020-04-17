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
#include <ai/property_map.hpp>
#include <ai/sensor_action.hpp>
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
#include <utility/log.hpp>
#include <boost/filesystem.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
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


vector3  get_rigid_body_external_acceleration(
        std::unordered_map<scn::scene_node_id, vector3> const&  rigid_body_external_accelerations,
        bool const  include_initial_accel
        )
{
    vector3  result = vector3_zero();
    for (auto const&  field_id_and_accel : rigid_body_external_accelerations)
        if (include_initial_accel || field_id_and_accel.first.valid())
            result += field_id_and_accel.second;
    return result;
}


vector3  get_rigid_body_external_acceleration(
        std::unordered_map<scn::scene_node_id, std::unordered_map<scn::scene_node_id, vector3> > const&  external_accelerations,
        scn::scene_node_id const&  id
        )
{
    vector3  result = vector3_zero();
    auto const  it = external_accelerations.find(id);
    if (it != external_accelerations.end())
        result = get_rigid_body_external_acceleration(it->second, true);
    return result;
}


void  update_rigid_body_external_acceleration(
        std::unordered_map<scn::scene_node_id, std::unordered_map<scn::scene_node_id, vector3> >&  external_accelerations,
        angeo::rigid_body_simulator&  rb_simulator,
        scn::scene_node_id const&  affected_object_nid,
        angeo::rigid_body_id const  affected_rb_id,
        scn::scene_node_id const&  accel_source_nid,
        vector3 const&  accel
        )
{
    auto  obj_it = external_accelerations.find(affected_object_nid);
    if (obj_it == external_accelerations.end())
        obj_it = external_accelerations.insert({ affected_object_nid, {} }).first;
    auto  field_it = obj_it->second.find(accel_source_nid);
    if (field_it == obj_it->second.end())
        field_it = obj_it->second.insert({ accel_source_nid, vector3_zero() }).first;
    vector3 const  old_total_accel = rb_simulator.get_external_linear_acceleration(affected_rb_id);
    rb_simulator.set_external_linear_acceleration(affected_rb_id, old_total_accel - field_it->second + accel);
    field_it->second = accel;
}


void  erase_rigid_body_external_acceleration(
        std::unordered_map<scn::scene_node_id, std::unordered_map<scn::scene_node_id, vector3> >&  external_accelerations,
        angeo::rigid_body_simulator&  rb_simulator,
        scn::scene_node_id const&  affected_object_nid,
        angeo::rigid_body_id const  affected_rb_id,
        scn::scene_node_id const&  accel_source_nid
        )
{
    auto const  obj_it = external_accelerations.find(affected_object_nid);
    if (obj_it != external_accelerations.end())
    {
        auto const  field_it = obj_it->second.find(accel_source_nid);
        if (field_it != obj_it->second.end())
        {
            vector3 const  old_total_accel = rb_simulator.get_external_linear_acceleration(affected_rb_id);
            rb_simulator.set_external_linear_acceleration(affected_rb_id, old_total_accel - field_it->second);
            obj_it->second.erase(field_it);
            if (obj_it->second.empty())
                external_accelerations.erase(obj_it);
        }
    }
}


void  switch_application_state_of_all_rigid_body_external_accelerations(
        std::unordered_map<scn::scene_node_id, std::unordered_map<scn::scene_node_id, vector3> > const&  external_accelerations,
        angeo::rigid_body_simulator&  rb_simulator,
        scn::scene const&  scene,
        bool const  state
        )
{
    float_32_bit const   multiplier = state ? 1.0f : -1.0f;
    for (auto const&  node_and_map : external_accelerations)
    {
        scn::scene_node_ptr const  node_ptr = scene.get_scene_node(node_and_map.first);
        if (node_ptr == nullptr)
            continue;
        scn::rigid_body const* const  rb_props = scn::get_rigid_body(*node_ptr);
        if (rb_props == nullptr)
            continue;
        vector3 const  accel = multiplier * get_rigid_body_external_acceleration(node_and_map.second, false);
        vector3 const  old_total_accel = rb_simulator.get_external_linear_acceleration(rb_props->id());
        rb_simulator.set_external_linear_acceleration(rb_props->id(), old_total_accel + accel);
    }
}


qtgl::batch  create_sketch_batch_from_sketch_id(std::string const&  sketch_id)
{
    boost::property_tree::ptree  props;
    qtgl::read_sketch_info_from_id(sketch_id, props);
    vector3  box_half_sizes_along_axes;
    float_32_bit  capsule_half_distance;
    float_32_bit  capsule_thickness;
    float_32_bit  sphere_radius;
    natural_8_bit  num_lines;
    vector4  colour;
    qtgl::FOG_TYPE  fog_type;
    bool wireframe;
    if (qtgl::parse_box_info_from_id(props, box_half_sizes_along_axes, colour, fog_type, wireframe))
        return wireframe ? qtgl::create_wireframe_box(box_half_sizes_along_axes, colour, fog_type) :
                           qtgl::create_solid_box(box_half_sizes_along_axes, colour, fog_type);
    else if (qtgl::parse_capsule_info_from_id(props, capsule_half_distance, capsule_thickness, num_lines, colour, fog_type, wireframe))
        return wireframe ? qtgl::create_wireframe_capsule(capsule_half_distance, capsule_thickness, num_lines, colour, fog_type) :
                           qtgl::create_solid_capsule(capsule_half_distance, capsule_thickness, num_lines, colour, fog_type);
    else if (qtgl::parse_sphere_info_from_id(props, sphere_radius, num_lines, colour, fog_type, wireframe))
        return wireframe ? qtgl::create_wireframe_sphere(sphere_radius, num_lines, colour, fog_type) :
                           qtgl::create_solid_sphere(sphere_radius, num_lines, colour, fog_type);
    else { UNREACHABLE(); return qtgl::batch(); }
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
    , m_rigid_bodies_external_linear_accelerations()
    , m_rigid_bodies_external_angular_accelerations()
    , m_ai_simulator_ptr(std::make_shared<ai::simulator>(std::make_shared<bind_ai_scene_to_simulator>(this)))
    , m_ai_requests_immediate()
    , m_ai_requests_delayed()

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

    qtgl::simulation_time_config::auto_updater const  time_config(m_simulation_time_config, this);

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

        if (!time_config().is_paused())
        {
            screen_text_logger::instance().clear();
            perform_simulation_step(time_config().get_clipped_simulation_time_step_in_seconds(seconds_from_previous_call));
        }
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
{
    ai::agents const&  agents = get_ai_simulator()->get_agents();
    for (ai::agent_id  agent_id = 0U; agent_id != agents.size(); ++agent_id)
    {
        if (!agents.ready(agent_id))
            continue;

        ai::retina_ptr const  retina_ptr = agents.at(agent_id).get_blackboard()->m_retina_ptr;
        if (retina_ptr == nullptr)
            continue;

        auto  offscreen_it = m_offscreens.find(agent_id);
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
}
static bool  render_text = false;
if (keyboard_props().was_just_released(qtgl::KEY_F4()))
render_text = !render_text;

if (render_text)
{
    static float_32_bit  scale = 1.0f;
    static vector3  shift{0.0f, -1.0f, 0.0f};
    static vector3  ambient_colour{ 0.9f, 0.9f, 0.95f };
    vector3 const  pos{
        m_camera->left() + scale * shift(0) * m_font_props.char_width,
        m_camera->top() + scale * shift(1)* m_font_props.char_height,
        -m_camera->near_plane()
    };
    qtgl::batch const  text_batch = qtgl::create_text(
        screen_text_logger::instance().text(),
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
    detail::switch_application_state_of_all_rigid_body_external_accelerations(
            m_rigid_bodies_external_linear_accelerations,
            *m_rigid_body_simulator_ptr,
            get_scene(),
            false
            );
    call_listeners(simulator_notifications::paused());
}


void  simulator::on_simulation_resumed()
{
    detail::switch_application_state_of_all_rigid_body_external_accelerations(
            m_rigid_bodies_external_linear_accelerations,
            *m_rigid_body_simulator_ptr,
            get_scene(),
            true
            );

    m_scene_selection.clear();
    m_scene_edit_data.invalidate_data();
    m_scene_history->clear();

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
        ai::agents const&  agents = get_ai_simulator()->get_agents();
        for (ai::agent_id  agent_id = 0U; agent_id != agents.size(); ++agent_id)
            if (agents.ready(agent_id))
            {
                ai::blackboard_agent_const_ptr const  blackboard = agents.at(agent_id).get_blackboard();

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
                    scn::scene_node_id const  bone_id =
                            get_ai_simulator()->get_record_id({ ai::OBJECT_KIND::AGENT, agent_id })->get_node_id() / raw_bone_id;
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

    get_collision_scene()->get_statistics().on_next_frame();

    auto const  ai_scene_binding = std::dynamic_pointer_cast<bind_ai_scene_to_simulator>(get_ai_simulator()->get_scene_ptr());

    m_collision_scene_ptr->compute_contacts_of_all_dynamic_objects(
            [this, is_last_micro_step, ai_scene_binding](
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

                    if (ai_scene_binding->do_tracking_collision_contact_of_collision_object(coid_1))
                        ai_scene_binding->on_collision_contact(contact_point, unit_normal, 0.0f, coid_1, material_1, coid_2, material_2);
                    if (ai_scene_binding->do_tracking_collision_contact_of_collision_object(coid_2))
                        ai_scene_binding->on_collision_contact(contact_point, -unit_normal, 0.0f, coid_2, material_2, coid_1, material_1);

                    auto const  rb_1_it = m_binding_of_collision_objects.find(coid_1);
                    auto const  rb_2_it = m_binding_of_collision_objects.find(coid_2);

                    if (rb_1_it == m_binding_of_collision_objects.cend() || rb_2_it == m_binding_of_collision_objects.cend())
                        return true;

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
                            20.0f
                            );

                    return true;
                },
            true
            );

    get_ai_simulator()->next_round((float_32_bit)time_to_simulate_in_seconds, keyboard_props(), mouse_props(), window_props());
    process_ai_requests(m_ai_requests_immediate, time_to_simulate_in_seconds);

    m_rigid_body_simulator_ptr->solve_constraint_system(time_to_simulate_in_seconds, time_to_simulate_in_seconds);
    m_rigid_body_simulator_ptr->integrate_motion_of_rigid_bodies(time_to_simulate_in_seconds);
    m_rigid_body_simulator_ptr->prepare_contact_cache_and_constraint_system_for_next_frame();
    for (auto const&  rb_id_and_node_ptr : m_binding_of_rigid_bodies)
    {
        angeo::rigid_body_id const  rb_id = rb_id_and_node_ptr.first;
        if (m_rigid_body_simulator_ptr->get_inverted_mass(rb_id) < 0.0001f &&
            length_squared(m_rigid_body_simulator_ptr->get_linear_velocity(rb_id)) < 1e-5f &&
            length_squared(m_rigid_body_simulator_ptr->get_angular_velocity(rb_id)) < 1e-5f)
        {
            continue;
        }

        scn::scene_node_ptr const  rb_node_ptr = rb_id_and_node_ptr.second;

        vector3  origin;
        quaternion  orientation;
        {
            if (rb_node_ptr->has_parent())
            {
                matrix44  from_node_to_patent_node_matrix;
                matrix33  R;
                {
                    matrix44  from_rigid_body_matrix;
                    compose_from_base_matrix(
                            m_rigid_body_simulator_ptr->get_position_of_mass_centre(rb_id),
                            quaternion_to_rotation_matrix(m_rigid_body_simulator_ptr->get_orientation(rb_id)),
                            from_rigid_body_matrix
                            );
                    matrix44  to_parent_node_matrix;
                    {
                        vector3  u;
                        decompose_matrix44(rb_node_ptr->get_parent()->get_world_matrix(), u, R);
                        compose_to_base_matrix(u, R, to_parent_node_matrix);
                    }
                    from_node_to_patent_node_matrix = to_parent_node_matrix * from_rigid_body_matrix;
                }
                decompose_matrix44(from_node_to_patent_node_matrix, origin, R);
                orientation = rotation_matrix_to_quaternion(R);
            }
            else
            {
                origin = m_rigid_body_simulator_ptr->get_position_of_mass_centre(rb_id);
                orientation = m_rigid_body_simulator_ptr->get_orientation(rb_id);
            }
        }
        rb_node_ptr->relocate(origin, orientation);
        update_collider_locations_in_subtree(rb_node_ptr);
    }

    process_ai_requests(m_ai_requests_delayed, time_to_simulate_in_seconds);
}


void  simulator::process_ai_requests(
        std::vector<ai::scene::request_ptr>&  ai_requests,
        float_64_bit const  time_to_simulate_in_seconds
        )
{
    TMPROF_BLOCK();

    std::vector<ai::scene::request_ptr>  requests(ai_requests.rbegin(), ai_requests.rend());
    ai_requests.clear();

    while (!requests.empty())
    {
        if (auto const  request = ai::scene::cast<ai::scene::request_merge_scene>(requests.back()))
        {
            scn::scene_node_ptr const  root_node_ptr =
                    import_scene(
                            request->props.get_string("scene_id"),
                            request->props.get_scene_node_id("parent_nid"),
                            request->props.get_scene_node_id("frame_nid")
                            );
            if (scn::rigid_body const* const  rb = scn::get_rigid_body(*root_node_ptr))
            {
                matrix44  W;

                W = matrix44_identity();
                if (request->props.has("velocities_frame_nid"))
                {
                    scn::scene_node_id const  motion_frame_id = request->props.get_scene_node_id("velocities_frame_nid");
                    if (motion_frame_id.valid())
                    {
                        scn::scene_node_ptr const  transform_node_ptr = get_scene_node(motion_frame_id);
                        if (transform_node_ptr != nullptr)
                            W = transform_node_ptr->get_world_matrix();
                    }
                }
                if (request->props.has_vector3("linear_velocity"))
                    m_rigid_body_simulator_ptr->set_linear_velocity(
                            rb->id(),
                            transform_vector(request->props.get_vector3("linear_velocity"), W)
                            );
                if (request->props.has_vector3("angular_velocity"))
                    m_rigid_body_simulator_ptr->set_angular_velocity(
                            rb->id(),
                            transform_vector(request->props.get_vector3("angular_velocity"), W)
                            );

                W = matrix44_identity();
                if (request->props.has("accelerations_frame_nid"))
                {
                    scn::scene_node_id const  motion_frame_id = request->props.get_scene_node_id("accelerations_frame_nid");
                    if (motion_frame_id.valid())
                    {
                        scn::scene_node_ptr const  transform_node_ptr = get_scene_node(motion_frame_id);
                        if (transform_node_ptr != nullptr)
                            W = transform_node_ptr->get_world_matrix();
                    }
                }
                if (request->props.has_vector3("linear_acceleration"))
                    m_rigid_body_simulator_ptr->set_external_linear_acceleration(
                            rb->id(),
                            transform_vector(request->props.get_vector3("linear_acceleration"), W)
                            );
                if (request->props.has_vector3("angular_acceleration"))
                    m_rigid_body_simulator_ptr->set_external_angular_acceleration(
                            rb->id(),
                            transform_vector(request->props.get_vector3("angular_acceleration"), W)
                            );
            }
        }
        else if (auto const  request = ai::scene::cast<ai::scene::request_erase_nodes_tree>(requests.back()))
        {
            ASSUMPTION(
                [](scn::scene_node_ptr const  node_ptr) -> bool {
                    return node_ptr == nullptr ||
                            scn::has_agent(*node_ptr) ||
                            scn::has_device(*node_ptr) ||
                            !scn::get_sensor_holders(*node_ptr).empty();
                    }(get_scene_node(request->root_nid))
                );
            erase_scene_node(request->root_nid);
        }
        else if (auto const  request = ai::scene::cast<ai::scene::request_update_radial_force_field>(requests.back()))
        {
            scn::scene_node_ptr const  affected_node_ptr = find_nearest_rigid_body_node(request->affected_object_rid.get_node_id());
            scn::scene_node_ptr const  field_node_ptr = get_scene_node(request->force_field_rid.get_node_id());
            scn::scene_node_ptr const  origin_node_ptr = get_scene_node(request->props.get_scene_node_id("origin_nid"));
            if (affected_node_ptr != nullptr && field_node_ptr != nullptr && origin_node_ptr != nullptr)
            {
                angeo::rigid_body_id const  rb_id = scn::get_rigid_body(*affected_node_ptr)->id();
                vector3  accel;
                {
                    vector3  origin_delta =
                            transform_point(vector3_zero(), origin_node_ptr->get_world_matrix()) -
                            transform_point(vector3_zero(), affected_node_ptr->get_world_matrix());
                    float_32_bit  distance = length(origin_delta);
                    if (distance < 1e-3f)
                    {
                        distance = 1e-3f;
                        origin_delta = distance * vector3_unit_z();
                    }
                    float_32_bit const  min_radius = request->props.get_float("min_radius");
                    if (distance < min_radius)
                    {
                        origin_delta *= (min_radius / distance);
                        distance = min_radius;
                    }
                    float_32_bit  inverted_mass;
                    {
                        std::string const&  mass_usage_type_name = request->props.get_string("mass_usage");
                        if (mass_usage_type_name == "NONE")
                            inverted_mass = 1.0f;
                        else
                        {
                            float_32_bit const  rb_mass_inv = m_rigid_body_simulator_ptr->get_inverted_mass(rb_id);
                            if (mass_usage_type_name == "HEAVY")
                                inverted_mass = rb_mass_inv == 0.0f ? 0.0f : 1.0f;
                            else if (mass_usage_type_name == "ALL")
                                inverted_mass = rb_mass_inv;
                            else { UNREACHABLE(); }
                        }
                    }
                    float_32_bit const  magnitude =
                            inverted_mass *
                            request->props.get_float("multiplier") *
                            std::powf(distance, request->props.get_float("exponent"));
                    accel = (magnitude / distance) * origin_delta;
                }
                detail::update_rigid_body_external_acceleration(
                        m_rigid_bodies_external_linear_accelerations,
                        *m_rigid_body_simulator_ptr,
                        affected_node_ptr->get_id(),
                        rb_id,
                        field_node_ptr->get_id(),
                        accel
                        );
            }
        }
        else if (auto const  request = ai::scene::cast<ai::scene::request_update_linear_force_field>(requests.back()))
        {
            scn::scene_node_ptr const  affected_node_ptr = get_scene_node(request->affected_object_rid.get_node_id());
            if (affected_node_ptr != nullptr)
            {
                angeo::rigid_body_id const  rb_id = scn::get_rigid_body(*affected_node_ptr)->id();
                float_32_bit  inverted_mass;
                {
                    std::string const&  mass_usage_type_name = request->props.get_string("mass_usage");
                    if (mass_usage_type_name == "NONE")
                        inverted_mass = 1.0f;
                    else
                    {
                        float_32_bit const  rb_mass_inv = m_rigid_body_simulator_ptr->get_inverted_mass(rb_id);
                        if (mass_usage_type_name == "HEAVY")
                            inverted_mass = rb_mass_inv == 0.0f ? 0.0f : 1.0f;
                        else if (mass_usage_type_name == "ALL")
                            inverted_mass = rb_mass_inv;
                        else { UNREACHABLE(); }
                    }
                }
                vector3 const  accel = inverted_mass * request->props.get_vector3();
                detail::update_rigid_body_external_acceleration(
                        m_rigid_bodies_external_linear_accelerations,
                        *m_rigid_body_simulator_ptr,
                        affected_node_ptr->get_id(),
                        rb_id,
                        request->force_field_rid.get_node_id(),
                        accel
                        );
            }
        }
        else if (auto const  request = ai::scene::cast<ai::scene::request_leave_force_field>(requests.back()))
        {
            scn::scene_node_ptr const  affected_node_ptr = find_nearest_rigid_body_node(request->affected_object_rid.get_node_id());
            if (affected_node_ptr != nullptr)
                detail::erase_rigid_body_external_acceleration(
                        m_rigid_bodies_external_linear_accelerations,
                        *m_rigid_body_simulator_ptr,
                        affected_node_ptr->get_id(),
                        scn::get_rigid_body(*affected_node_ptr)->id(),
                        request->force_field_rid.get_node_id()
                        );
        }
        else if (auto const  request = ai::scene::cast<ai::scene::request_insert_rigid_body_constraint>(requests.back()))
        {
            scn::scene_node_ptr const  other_node_ptr = find_nearest_rigid_body_node(request->other_rb_nid);
            if (other_node_ptr != nullptr)
            {
                scn::rigid_body const* const  other_rb_ptr = scn::get_rigid_body(*other_node_ptr);
                if (other_rb_ptr != nullptr)
                {
                    angeo::rigid_body_id const  other_rb_id = other_rb_ptr->id();

                    scn::scene_node_ptr const  self_node_ptr = find_nearest_rigid_body_node(request->self_rb_nid);
                    ASSUMPTION(self_node_ptr != nullptr && scn::has_rigid_body(*self_node_ptr));
                    angeo::rigid_body_id const  self_rb_id = scn::get_rigid_body(*self_node_ptr)->id();

                    m_rigid_body_simulator_ptr->get_constraint_system().insert_constraint(
                            self_rb_id,
                            request->self_linear_component,
                            request->self_angular_component,
                            other_rb_id,
                            request->other_linear_component,
                            request->other_angular_component,
                            request->bias,
                            [request](std::vector<float_32_bit> const&) { return request->variable_lower_bound; },
                            [request](std::vector<float_32_bit> const&) { return request->variable_upper_bound; },
                            request->variable_initial_value
                            );
                }
            }
        }
        else
        {
            UNREACHABLE();
        }
        requests.pop_back();
    }

    INVARIANT(ai_requests.empty());
}


scn::scene_node_ptr  simulator::import_scene(
        std::string const&  scene_id,
        scn::scene_node_id const&  parent_id,
        scn::scene_node_id const&  frame_id
        )
{
    TMPROF_BLOCK();

    boost::filesystem::path const  scene_pathname =
            boost::filesystem::path(get_program_options()->dataRoot()) / scene_id / "hierarchy.info"
            ;
    ASSUMPTION(boost::filesystem::is_regular_file(scene_pathname));
    boost::property_tree::ptree  impored_scene_ptree;
    boost::property_tree::read_info(scene_pathname.string(), impored_scene_ptree);

    scn::scene_node_ptr  root_node_ptr = nullptr;
    for (auto it = impored_scene_ptree.begin(); it != impored_scene_ptree.end(); ++it)
    {
        if (it->first.empty() || it->first.front() == '@')
            continue;
        scn::scene_node_id  node_id = parent_id / it->first;
        natural_32_bit  id_counter = 0U;
        while (get_scene_node(node_id) != nullptr)
        {
            node_id = parent_id / scn::scene_node_id{ msgstream() << it->first << id_counter };
            ++id_counter;
        }
        ASSUMPTION(root_node_ptr == nullptr);
        root_node_ptr = import_scene_node(node_id, it->second, get_scene_node(frame_id));
    }
    ASSUMPTION(root_node_ptr != nullptr);
    return root_node_ptr;
}


scn::scene_node_ptr  simulator::import_scene_node(
        scn::scene_node_id const&  id,
        boost::property_tree::ptree const&  node_tree,
        scn::scene_node_ptr const  relocation_node_ptr
        )
{
    vector3  origin;
    quaternion  orientation;
    if (relocation_node_ptr == nullptr)
    {
        boost::property_tree::ptree const&  origin_tree = node_tree.find("origin")->second;
        origin = vector3(
                origin_tree.get<scalar>("x"),
                origin_tree.get<scalar>("y"),
                origin_tree.get<scalar>("z")
                );

        boost::property_tree::ptree const&  orientation_tree = node_tree.find("orientation")->second;
        orientation = make_quaternion_xyzw(
                orientation_tree.get<scalar>("x"),
                orientation_tree.get<scalar>("y"),
                orientation_tree.get<scalar>("z"),
                orientation_tree.get<scalar>("w")
                );
    }
    else
    {
        matrix44  node_from_base_matrix;
        matrix33  R;
        {
            if (id.is_root())
                node_from_base_matrix = relocation_node_ptr->get_world_matrix();
            else
            {
                vector3  u;
                decompose_matrix44(get_scene_node(id.get_direct_parent_id())->get_world_matrix(), u, R);
                matrix44  to_parent_space_matrix;
                compose_to_base_matrix(u, R, to_parent_space_matrix);
                node_from_base_matrix = to_parent_space_matrix * relocation_node_ptr->get_world_matrix();
            }
        }
        decompose_matrix44(node_from_base_matrix, origin, R);
        orientation = rotation_matrix_to_quaternion(R);
    }

    scn::scene_node_ptr const  node_ptr = insert_scene_node_at(id, origin, orientation);

    boost::property_tree::ptree const&  children = node_tree.find("children")->second;
    for (auto  it = children.begin(); it != children.end(); ++it)
        import_scene_node(id / it->first, it->second, nullptr);

    boost::property_tree::ptree const&  folders = node_tree.find("folders")->second;
    for (auto folder_it = folders.begin(); folder_it != folders.end(); ++folder_it)
        for (auto record_it = folder_it->second.begin(); record_it != folder_it->second.end(); ++record_it)
            if (folder_it->first == scn::get_agent_folder_name())
                load_agent(record_it->second, { id, folder_it->first, record_it->first });
            else if (folder_it->first == scn::get_device_folder_name())
                load_device(record_it->second, { id, folder_it->first, record_it->first });
            else if (folder_it->first == scn::get_sensors_folder_name())
                load_sensor(record_it->second, { id, folder_it->first, record_it->first });
            else if (folder_it->first == scn::get_collider_folder_name())
                load_collider(record_it->second, id);
            else if (folder_it->first == scn::get_rigid_body_folder_name())
                load_rigid_body(record_it->second, id);
            else if (folder_it->first == scn::get_batches_folder_name())
                insert_batch_to_scene_node(
                        record_it->first,
                        record_it->second.get<std::string>("id"),
                        record_it->second.get<std::string>("skin"),
                        get_effects_config(),
                        id
                        );
            else UNREACHABLE();

    return node_ptr;
}


void  simulator::update_retina_of_agents_from_offscreen_images(float_32_bit const  time_to_simulate_in_seconds)
{
    TMPROF_BLOCK();

    float_32_bit const  camera_FOV_angle = PI() / 2.0f;
    float_32_bit const  camera_proj_dist = 0.05f;
    float_32_bit const  camera_origin_z_shift = 0.0f;

    ai::agents const&  agents = get_ai_simulator()->get_agents();
    for (ai::agent_id  agent_id = 0U; agent_id != agents.size(); ++agent_id)
    {
        ai::agents const&  agents = get_ai_simulator()->get_agents();
        if (!agents.ready(agent_id))
            continue;

        ai::retina_ptr const  retina_ptr = agents.at(agent_id).get_blackboard()->m_retina_ptr;
        if (retina_ptr == nullptr)
            continue;

        auto  recovery_time_it = m_offscreen_recovery_times.find(agent_id);
        if (recovery_time_it == m_offscreen_recovery_times.end())
            recovery_time_it = m_offscreen_recovery_times.insert({ agent_id, 0.0f }).first;
        recovery_time_it->second += time_to_simulate_in_seconds;
        static float_32_bit  RECOVERY_PERIOD_IN_SECONDS = 0.0f;// 1.0f / 30.0f;
        if (recovery_time_it->second < RECOVERY_PERIOD_IN_SECONDS)
            continue;
        m_offscreen_recovery_times.erase(recovery_time_it);

        ai::sensory_controller_sight_ptr const  sight_ptr = agents.at(agent_id).get_sensory_controller().get_sight();
        if (sight_ptr == nullptr)
            continue;
        ai::sensory_controller_sight::camera_perspective_ptr const  camera_ptr = sight_ptr->get_camera();
        if (camera_ptr == nullptr)
            continue;

        auto  offscreen_it = m_offscreens.find(agent_id);
        if (offscreen_it == m_offscreens.cend())
            offscreen_it = m_offscreens.insert({
                    agent_id,
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
                    if (agent_ptr != nullptr && agent_ptr->get_props().m_skeleton_props != nullptr)
                        motion_templates = agent_ptr->get_props().m_skeleton_props->skeletal_motion_templates;
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

    cache_of_batches_of_colliders::boxes_map  old_boxes;  old_boxes.swap(m_cache_of_batches_of_colliders.boxes);
    cache_of_batches_of_colliders::spheres_map  old_spheres;  old_spheres.swap(m_cache_of_batches_of_colliders.spheres);
    cache_of_batches_of_colliders::capsules_map  old_capsules;  old_capsules.swap(m_cache_of_batches_of_colliders.capsules);
    cache_of_batches_of_colliders::triangles_map  old_triangle_meshes;  old_triangle_meshes.swap(m_cache_of_batches_of_colliders.triangle_meshes);

    get_scene().foreach_node(
        [this, &matrix_from_world_to_camera, &matrix_from_camera_to_clipspace, &draw_state,
         &old_boxes, &old_spheres, &old_capsules, &old_triangle_meshes](scn::scene_node_ptr const  node_ptr)
            -> bool {
                if (auto const collider_ptr = scn::get_collider(*node_ptr))
                {
                    std::vector<qtgl::batch>  batches;
                    {
                        angeo::collision_object_id const  coid = collider_ptr->id();
                        switch (angeo::get_shape_type(coid))
                        {
                        case angeo::COLLISION_SHAPE_TYPE::BOX:
                            {
                                vector3 const  key { m_collision_scene_ptr->get_box_half_sizes_along_axes(coid) };
                                auto  it = old_boxes.find(key);
                                if (it == old_boxes.end())
                                    it = old_boxes.insert({
                                                key,
                                                qtgl::create_wireframe_box(key, m_colliders_colour)
                                                }).first;
                                m_cache_of_batches_of_colliders.boxes.insert(*it);
                                batches.push_back(it->second);
                            }
                            break;
                        case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                            {
                                std::pair<float_32_bit, float_32_bit> const  key {
                                        m_collision_scene_ptr->get_capsule_half_distance_between_end_points(coid),
                                        m_collision_scene_ptr->get_capsule_thickness_from_central_line(coid)
                                        };
                                auto  it = old_capsules.find(key);
                                if (it == old_capsules.end())
                                    it = old_capsules.insert({
                                                key,
                                                qtgl::create_wireframe_capsule(key.first, key.second, 5U, m_colliders_colour)
                                                }).first;
                                m_cache_of_batches_of_colliders.capsules.insert(*it);
                                batches.push_back(it->second);
                            }
                            break;
                        case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                            {
                                float_32_bit const  key = m_collision_scene_ptr->get_sphere_radius(coid);
                                auto  it = old_spheres.find(key);
                                if (it == old_spheres.end())
                                    it = old_spheres.insert({
                                                key,
                                                qtgl::create_wireframe_sphere(key, 5U, m_colliders_colour)
                                                }).first;
                                m_cache_of_batches_of_colliders.spheres.insert(*it);
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
                                auto  it = old_triangle_meshes.find(key);
                                if (it == old_triangle_meshes.end())
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
                                m_cache_of_batches_of_colliders.triangle_meshes.insert(*it);
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

    ai::agents const&  agents = get_ai_simulator()->get_agents();

    for (ai::agent_id  agent_id = 0U; agent_id != agents.size(); ++agent_id)
    {
        if (!agents.ready(agent_id))
            continue;
        ai::sensory_controller_sight_ptr const  sight_ptr = agents.at(agent_id).get_sensory_controller().get_sight();
        if (sight_ptr == nullptr)
            continue;
        ai::sensory_controller_sight::camera_perspective_ptr const  camera_ptr = sight_ptr->get_camera();
        if (camera_ptr == nullptr)
        {
            m_cache_of_batches_of_ai_agents.sight_frustum_batches.erase(agent_id);
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

        auto  frustum_batch_it = m_cache_of_batches_of_ai_agents.sight_frustum_batches.find(agent_id);
        if (frustum_batch_it == m_cache_of_batches_of_ai_agents.sight_frustum_batches.end())
            frustum_batch_it = m_cache_of_batches_of_ai_agents.sight_frustum_batches.insert({
                    agent_id,
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

    for (ai::agent_id  agent_id = 0U; agent_id != agents.size(); ++agent_id)
    {
        if (!agents.ready(agent_id))
            continue;

        ai::sensory_controller_ray_cast_sight_ptr const  sight_ptr =
                std::dynamic_pointer_cast<ai::sensory_controller_ray_cast_sight>(
                        agents.at(agent_id).get_sensory_controller().get_sight()
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


void  simulator::accept_ai_request(ai::scene::request_ptr const  request, bool const  delay_processing_to_next_time_step)
{
    (delay_processing_to_next_time_step ? &m_ai_requests_delayed : &m_ai_requests_immediate)->push_back(request);
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
    if (node_ptr == nullptr)
        return;

    while (!node_ptr->get_children().empty())
        erase_scene_node(node_ptr->get_children().begin()->second->get_id());

    {
        std::vector<std::string>  sensor_names;
        for (auto const&  name_holder : scn::get_sensor_holders(*node_ptr))
            sensor_names.push_back(name_holder.first);
        for (auto const&  sensor_name : sensor_names)
            erase_sensor(scn::make_sensor_record_id(node_ptr->get_id(), sensor_name));
    }
    if (scn::has_device(*node_ptr))
        erase_device(scn::make_device_record_id(id));
    if (scn::has_agent(*node_ptr))
        erase_agent(scn::make_agent_record_id(id));

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


vector3  simulator::get_rigid_body_external_linear_acceleration(scn::scene_node_id const&  id) const
{
    return detail::get_rigid_body_external_acceleration(m_rigid_bodies_external_linear_accelerations, id);
}


vector3  simulator::get_rigid_body_external_angular_acceleration(scn::scene_node_id const&  id) const
{
    return detail::get_rigid_body_external_acceleration(m_rigid_bodies_external_angular_accelerations, id);
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
        std::string const&  batch_pathname_or_sketch_id,
        std::string const&  skin_name,
        qtgl::effects_config const&  effects,
        scn::scene_node_id const&  scene_node_id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(scn::has_node(get_scene(), scene_node_id));
    qtgl::batch  batch;
    if (boost::starts_with(batch_pathname_or_sketch_id, qtgl::get_sketch_id_prefix()))
        batch = detail::create_sketch_batch_from_sketch_id(batch_pathname_or_sketch_id);
    else
        batch = qtgl::batch(canonical_path(batch_pathname_or_sketch_id), effects, skin_name);
    scn::insert_batch(*get_scene_node(scene_node_id), batch_name, batch);
}

void  simulator::replace_batch_in_scene_node(
        scn::scene_node::record_name const&  batch_name,
        std::string const&  new_batch_pathname_or_sketch_id,
        std::string const&  new_skin_name,
        qtgl::effects_config const&  new_effects,
        scn::scene_node_id const&  scene_node_id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(scene_node_id);
    ASSUMPTION(node_ptr != nullptr);

    qtgl::batch const  old_batch = scn::get_batch(*node_ptr, batch_name);
    ASSUMPTION(!old_batch.empty());

    qtgl::batch  new_batch;
    if (boost::starts_with(new_batch_pathname_or_sketch_id, qtgl::get_sketch_id_prefix()))
        new_batch = detail::create_sketch_batch_from_sketch_id(new_batch_pathname_or_sketch_id);
    else
        new_batch = qtgl::batch(old_batch.get_id(), new_effects, new_skin_name);

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


void  simulator::insert_collision_box_to_scene_node(
        vector3 const&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic,
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    angeo::collision_object_id const  collider_id =
        m_collision_scene_ptr->insert_box(half_sizes_along_axes, node_ptr->get_world_matrix(), material, collision_class, as_dynamic);
    scn::insert_collider(*node_ptr, angeo::as_collision_shape_type(id.get_record_name()), collider_id, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}


void  simulator::insert_collision_sphere_to_scene_node(
        float_32_bit const  radius,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
        float_32_bit const  density_multiplier,
        bool const  as_dynamic,
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    angeo::collision_object_id const  collider_id =
            m_collision_scene_ptr->insert_sphere(radius, node_ptr->get_world_matrix(), material, collision_class, as_dynamic);
    scn::insert_collider(*node_ptr, angeo::as_collision_shape_type(id.get_record_name()), collider_id, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}

void  simulator::insert_collision_capsule_to_scene_node(
        float_32_bit const  half_distance_between_end_points,
        float_32_bit const  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
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
                    collision_class,
                    as_dynamic
                    );
    scn::insert_collider(*node_ptr, angeo::as_collision_shape_type(id.get_record_name()), collider_id, density_multiplier);

    if (scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr))
        rebuild_rigid_body_due_to_change_in_subtree(phs_node);
}

void  simulator::insert_collision_trianle_mesh_to_scene_node(
        qtgl::buffer const  vertex_buffer,
        qtgl::buffer const  index_buffer,
        angeo::COLLISION_MATERIAL_TYPE const  material,
        angeo::COLLISION_CLASS const  collision_class,
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
            collision_class,
            false,
            collider_ids
            );

    scn::insert_collider(*node_ptr, angeo::as_collision_shape_type(id.get_record_name()), collider_ids, density_multiplier);

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


void  simulator::get_collision_box_info(
        scn::scene_record_id const&  id,
        vector3&  half_sizes_along_axes,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        angeo::COLLISION_CLASS&  collision_class,
        float_32_bit&  density_multiplier,
        bool&  is_dynamic
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    half_sizes_along_axes = m_collision_scene_ptr->get_box_half_sizes_along_axes(collider->id());
    material = m_collision_scene_ptr->get_material(collider->id());
    collision_class = m_collision_scene_ptr->get_collision_class(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene_ptr->is_dynamic(collider->id());
}

void  simulator::get_collision_sphere_info(
        scn::scene_record_id const&  id,
        float_32_bit&  radius,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        angeo::COLLISION_CLASS&  collision_class,
        float_32_bit&  density_multiplier,
        bool&  is_dynamic
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    radius = m_collision_scene_ptr->get_sphere_radius(collider->id());
    material = m_collision_scene_ptr->get_material(collider->id());
    collision_class = m_collision_scene_ptr->get_collision_class(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene_ptr->is_dynamic(collider->id());
}

void  simulator::get_collision_capsule_info(
        scn::scene_record_id const&  id,
        float_32_bit&  half_distance_between_end_points,
        float_32_bit&  thickness_from_central_line,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        angeo::COLLISION_CLASS&  collision_class,
        float_32_bit&  density_multiplier,
        bool&  is_dynamic
        )
{
    scn::collider const* const  collider = scn::get_collider(*get_scene_node(id.get_node_id()));
    ASSUMPTION(collider != nullptr);
    half_distance_between_end_points = m_collision_scene_ptr->get_capsule_half_distance_between_end_points(collider->id());
    thickness_from_central_line = m_collision_scene_ptr->get_capsule_thickness_from_central_line(collider->id());
    material = m_collision_scene_ptr->get_material(collider->id());
    collision_class = m_collision_scene_ptr->get_collision_class(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene_ptr->is_dynamic(collider->id());
}


void  simulator::get_collision_triangle_mesh_info(
        scn::scene_record_id const&  id,
        qtgl::buffer&  vertex_buffer,
        qtgl::buffer&  index_buffer,
        angeo::COLLISION_MATERIAL_TYPE&  material,
        angeo::COLLISION_CLASS&  collision_class,
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
    collision_class = m_collision_scene_ptr->get_collision_class(collider->id());
    density_multiplier = collider->get_density_multiplier();
}


void  simulator::insert_rigid_body_to_scene_node(
        scn::rigid_body_props const&  props,
        bool const  auto_compute_mass_and_inertia_tensor,
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id);
    ASSUMPTION(node_ptr != nullptr && find_nearest_rigid_body_node(node_ptr) == nullptr);

    std::vector<angeo::collision_object_id>  coids;
    std::vector<scn::scene_node_ptr>  coid_nodes;
    bool  has_static_collider;
    {
        collect_colliders_in_subtree(node_ptr, coids, &coid_nodes);

        has_static_collider = false;
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

    scn::rigid_body_props  rb_props;
    if (has_static_collider)
    {
        rb_props.m_linear_velocity = vector3_zero();
        rb_props.m_angular_velocity = vector3_zero();
        rb_props.m_external_linear_acceleration = vector3_zero();
        rb_props.m_external_angular_acceleration = vector3_zero();
        rb_props.m_mass_inverted = 0.0f;
        rb_props.m_inertia_tensor_inverted = matrix33_zero();
    }
    else if (auto_compute_mass_and_inertia_tensor && !coids.empty())
    {
        rb_props = props; // But mass and inertial tesnsor will be reset below.
        angeo::mass_and_inertia_tensor_builder  builder;
        for (std::size_t i = 0UL; i != coids.size(); ++i)
        {
            angeo::collision_object_id const  coid = coids.at(i);
            scn::scene_node_ptr const  coid_node_ptr = coid_nodes.at(i);
            switch (angeo::get_shape_type(coid))
            {
            case angeo::COLLISION_SHAPE_TYPE::BOX:
                builder.insert_box(
                        m_collision_scene_ptr->get_box_half_sizes_along_axes(coid),
                        coid_node_ptr->get_world_matrix(),
                        m_collision_scene_ptr->get_material(coid),
                        1.0f // TODO!
                        );
                break;
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
        builder.run(rb_props.m_mass_inverted, rb_props.m_inertia_tensor_inverted, center_of_mass_in_world_space);

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
    else
    {
        rb_props = props;
    }

    insert_rigid_body_to_scene_node_direct(
            node_ptr,
            rb_props.m_linear_velocity,
            rb_props.m_angular_velocity,
            rb_props.m_external_linear_acceleration,
            rb_props.m_external_angular_acceleration,
            rb_props.m_mass_inverted,
            rb_props.m_inertia_tensor_inverted,
            auto_compute_mass_and_inertia_tensor,
            &coids
            );
}


void  simulator::insert_rigid_body_to_scene_node_direct(
        scn::scene_node_ptr const  node_ptr,
        vector3 const&  linear_velocity,
        vector3 const&  angular_velocity,
        vector3 const&  external_linear_acceleration,
        vector3 const&  external_angular_acceleration,
        float_32_bit const  mass_inverted,
        matrix33 const&  inertia_tensor_inverted,
        bool const  auto_compute_mass_and_inertia_tensor,
        std::vector<angeo::collision_object_id> const*  coids_ptr
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(node_ptr != nullptr && find_nearest_rigid_body_node(node_ptr) == nullptr);

    std::vector<angeo::collision_object_id>  coids;
    if (coids_ptr == nullptr)
    {
        collect_colliders_in_subtree(node_ptr, coids);
        for (std::size_t i = 0U; i < coids.size(); ++i)
            for (std::size_t j = i + 1U; j < coids.size(); ++j)
                m_collision_scene_ptr->disable_colliding(coids.at(i), coids.at(j));
        coids_ptr = &coids;
    }

    vector3  origin;
    matrix33  R;
    decompose_matrix44(node_ptr->get_world_matrix(), origin, R);
    angeo::rigid_body_id const  rb_id =
            m_rigid_body_simulator_ptr->insert_rigid_body(
                    origin,
                    rotation_matrix_to_quaternion(R),
                    mass_inverted,
                    inertia_tensor_inverted,
                    linear_velocity,
                    angular_velocity,
                    external_linear_acceleration,
                    external_angular_acceleration
                    );

    scn::insert_rigid_body(*node_ptr, rb_id, auto_compute_mass_and_inertia_tensor);
    m_binding_of_rigid_bodies[rb_id] = node_ptr;

    if (!are_equal_3d(external_linear_acceleration, vector3_zero(), 1e-5f))
        m_rigid_bodies_external_linear_accelerations[node_ptr->get_id()][{}] = external_linear_acceleration;
    if (!are_equal_3d(external_angular_acceleration, vector3_zero(), 1e-5f))
        m_rigid_bodies_external_angular_accelerations[node_ptr->get_id()][{}] = external_angular_acceleration;

    for (angeo::collision_object_id  coid : *coids_ptr)
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

        m_rigid_bodies_external_linear_accelerations.erase(id);
        m_rigid_bodies_external_angular_accelerations.erase(id);

        m_scene_selection.erase_record(scn::make_rigid_body_record_id(id));
        scn::erase_rigid_body(*node_ptr);
    }
}


void  simulator::rebuild_rigid_body_due_to_change_in_subtree(scn::scene_node_ptr const  phs_node_ptr)
{
    ASSUMPTION(phs_node_ptr != nullptr);
    auto  rb_ptr = scn::get_rigid_body(*phs_node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    bool const  auto_compute_backup = rb_ptr->auto_compute_mass_and_inertia_tensor();
    scn::rigid_body_props  rb_backup;
    rb_backup.m_linear_velocity = m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id());
    rb_backup.m_angular_velocity = m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id());
    rb_backup.m_external_linear_acceleration = m_rigid_body_simulator_ptr->get_external_linear_acceleration(rb_ptr->id());
    rb_backup.m_external_angular_acceleration = m_rigid_body_simulator_ptr->get_external_angular_acceleration(rb_ptr->id());
    rb_backup.m_mass_inverted = m_rigid_body_simulator_ptr->get_inverted_mass(rb_ptr->id());
    rb_backup.m_inertia_tensor_inverted = m_rigid_body_simulator_ptr->get_inverted_inertia_tensor_in_local_space(rb_ptr->id());
    erase_rigid_body_from_scene_node(phs_node_ptr->get_id());
    insert_rigid_body_to_scene_node(rb_backup, auto_compute_backup, phs_node_ptr->get_id());
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


bool  simulator::get_rigid_body_of_collider(angeo::collision_object_id const  coid, angeo::rigid_body_id* const  output_rigid_body_id_ptr)
{
    auto const  rb_it = m_binding_of_collision_objects.find(coid);
    if (rb_it == m_binding_of_collision_objects.cend())
        return false;
    if (output_rigid_body_id_ptr != nullptr)
        *output_rigid_body_id_ptr = rb_it->second;
    return true;
}


scn::scene_node_ptr  simulator::get_rigid_body_node(angeo::rigid_body_id const  rb_id) const
{
    auto const  it = m_binding_of_rigid_bodies.find(rb_id);
    return it == m_binding_of_rigid_bodies.cend() ? nullptr : it->second;
}


void  simulator::insert_agent(scn::scene_record_id const&  id, scn::agent_props const&  props)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr && !node_ptr->has_parent());
    ai::agent_id const  agent_id =
            get_ai_simulator()->insert_agent(
                    id,
                    props.m_skeleton_props->skeletal_motion_templates,
                    props.m_agent_kind,
                    props.m_sensor_action_map,
                    ai::make_retina(100U, 100U, true)
                    );
    scn::insert_agent(*node_ptr, agent_id, props);

    ai::object_id const  agent_oid{ ai::OBJECT_KIND::AGENT, agent_id };
    auto const  set_owner_of_sensor = [this, &agent_oid](scn::scene_node_ptr const  node_ptr) -> bool {
        for (auto const& name_holder : scn::get_sensor_holders(*node_ptr))
            get_ai_simulator()->set_owner_of_sensor(scn::as_sensor(name_holder.second)->id(), agent_oid);
        return true;
    };
    set_owner_of_sensor(node_ptr);
    node_ptr->foreach_child(set_owner_of_sensor, true);
}


void  simulator::erase_agent(scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
    ASSUMPTION(agent_ptr != nullptr);

    //ASSUMPTION(
    //    !scn::has_any_sensor(*node_ptr) && !scn::has_device(*node_ptr) &&
    //    [node_ptr]() -> bool {
    //        bool result = true;
    //        node_ptr->foreach_child(
    //            [&result](scn::scene_node_ptr const  node_ptr) -> bool {
    //                if (scn::has_any_sensor(*node_ptr) || scn::has_device(*node_ptr))
    //                    result = false;
    //                return result;
    //            },
    //            true);
    //        return result;
    //    }());

    get_ai_simulator()->erase_agent(agent_ptr->id());

    m_scene_selection.erase_record(id);
    scn::erase_agent(*node_ptr);
}


void  simulator::get_agent_info(scn::scene_node_id const& id, scn::agent_props&  props)
{
    scn::agent const* const  agent_ptr = scn::get_agent(*get_scene_node(id));
    ASSUMPTION(agent_ptr != nullptr);
    props = agent_ptr->get_props();
}


void  simulator::insert_device(scn::scene_record_id const&  id, scn::device_props const&  props)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    ai::device_id const  device_id =
            get_ai_simulator()->insert_device(
                    id,
                    props.m_skeleton_props->skeletal_motion_templates,
                    props.m_device_kind,
                    props.m_sensor_action_map
                    );
    scn::insert_device(*node_ptr, device_id, props);

    ai::object_id const  device_oid{ ai::OBJECT_KIND::DEVICE, device_id };
    auto const  set_owner_of_sensor = [this, &device_oid](scn::scene_node_ptr const  node_ptr) -> bool {
        for (auto const&  name_holder : scn::get_sensor_holders(*node_ptr))
            get_ai_simulator()->set_owner_of_sensor(scn::as_sensor(name_holder.second)->id(), device_oid);
            return true;
    };
    set_owner_of_sensor(node_ptr);
    node_ptr->foreach_child(set_owner_of_sensor, true);
}


void  simulator::erase_device(scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    scn::device const* const  device_ptr = scn::get_device(*node_ptr);
    ASSUMPTION(device_ptr != nullptr);

    //ASSUMPTION(
    //    !scn::has_any_sensor(*node_ptr) && !scn::has_agent(*node_ptr) &&
    //    [node_ptr]() -> bool {
    //        bool result = true;
    //        node_ptr->foreach_child(
    //            [&result](scn::scene_node_ptr const  node_ptr) -> bool {
    //                if (scn::has_any_sensor(*node_ptr) || scn::has_device(*node_ptr))
    //                    result = false;
    //                return result;
    //            },
    //            true);
    //        return result;
    //    }());

    get_ai_simulator()->erase_device(device_ptr->id());

    m_scene_selection.erase_record(id);
    scn::erase_device(*node_ptr);
}


void  simulator::get_device_info(scn::scene_node_id const& id, scn::device_props&  props)
{
    scn::device const* const  device_ptr = scn::get_device(*get_scene_node(id));
    ASSUMPTION(device_ptr != nullptr);
    props = device_ptr->get_props();
}


void  simulator::insert_sensor(scn::scene_record_id const&  id, scn::sensor_props const&  props)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    ai::object_id  owner_id = ai::object_id::make_invalid();
    for (scn::scene_node_ptr  owner_node_ptr = node_ptr; owner_node_ptr != nullptr; owner_node_ptr = owner_node_ptr->get_parent())
    {
        if (scn::agent const* const  agent_ptr = scn::get_agent(*owner_node_ptr))
        {
            owner_id = ai::object_id(ai::OBJECT_KIND::AGENT, agent_ptr->id());
            break;
        }
        if (scn::device const* const  device_ptr = scn::get_device(*owner_node_ptr))
        {
            owner_id = ai::object_id(ai::OBJECT_KIND::DEVICE, device_ptr->id());
            break;
        }
    }
    std::vector<scn::scene_node_id>  collider_nids;
    foreach_collider_in_subtree(
            node_ptr,
            [&collider_nids](scn::collider&, scn::scene_node_ptr const  ptr) -> void {
                collider_nids.push_back(ptr->get_id());
            });
    ai::sensor_id const  sensor_id =
            get_ai_simulator()->insert_sensor(
                    id,
                    props.m_sensor_kind,
                    owner_id,
                    props.m_enabled,
                    props.m_sensor_props,
                    collider_nids
                    );
    scn::insert_sensor(*node_ptr, id.get_record_name(), sensor_id, props);
}


void  simulator::erase_sensor(scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    scn::sensor const* const  sensor_ptr = scn::get_sensor(*node_ptr, id.get_record_name());
    ASSUMPTION(sensor_ptr != nullptr);

    get_ai_simulator()->erase_sensor(sensor_ptr->id());

    m_scene_selection.erase_record(id);
    scn::erase_sensor(*node_ptr, id.get_record_name());
}


void  simulator::get_sensor_info(scn::scene_record_id const& id, scn::sensor_props&  props)
{
    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr);
    scn::sensor const* const  sensor_ptr = scn::get_sensor(*node_ptr, id.get_record_name());
    ASSUMPTION(sensor_ptr != nullptr);
    props = sensor_ptr->get_props();
}


void  simulator::get_sensor_nodes_and_kinds_under_scene_node(
        scn::scene_node_id const&  search_root_id,
        std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> >&  output_sensor_nodes_and_kinds
        )
{
    auto const  action = [&output_sensor_nodes_and_kinds](scn::scene_node_ptr const  node_ptr) -> bool {
        for (auto const&  name_holder : scn::get_sensor_holders(*node_ptr))
            output_sensor_nodes_and_kinds.push_back({
                    scn::make_sensor_record_id(node_ptr->get_id(), name_holder.first),
                    scn::as_sensor(name_holder.second)->get_props().m_sensor_kind
                    });
        return true;
    };
    auto const  iterate_children_of = [&search_root_id](scn::scene_node_ptr const  node_ptr) -> bool {
        return !scn::has_device(*node_ptr) && !scn::has_agent(*node_ptr);
    };
    scn::scene_node_ptr const  node_ptr = get_scene_node(search_root_id);
    ASSUMPTION(node_ptr != nullptr);
    action(node_ptr);
    node_ptr->foreach_child(action, iterate_children_of);
}


void  simulator::load_collider(boost::property_tree::ptree const&  data, scn::scene_node_id const&  id)
{
    TMPROF_BLOCK();

    angeo::COLLISION_SHAPE_TYPE const  shape_type = angeo::as_collision_shape_type(data.get<std::string>("shape_type"));
    if (shape_type == angeo::COLLISION_SHAPE_TYPE::BOX)
        insert_collision_box_to_scene_node(
            vector3(data.get<float_32_bit>("half_size_along_x"),
                    data.get<float_32_bit>("half_size_along_y"),
                    data.get<float_32_bit>("half_size_along_z")),
            angeo::read_collison_material_from_string(data.get<std::string>("material")),
            angeo::read_collison_class_from_string(data.get<std::string>("collision_class")),
            data.get<float_32_bit>("density_multiplier"),
            data.get<bool>("is_dynamic"),
            scn::make_collider_record_id(id, shape_type)
        );
    else if (shape_type == angeo::COLLISION_SHAPE_TYPE::CAPSULE)
        insert_collision_capsule_to_scene_node(
                data.get<float_32_bit>("half_distance_between_end_points"),
                data.get<float_32_bit>("thickness_from_central_line"),
                angeo::read_collison_material_from_string(data.get<std::string>("material")),
                angeo::read_collison_class_from_string(data.get<std::string>("collision_class")),
                data.get<float_32_bit>("density_multiplier"),
                data.get<bool>("is_dynamic"),
                scn::make_collider_record_id(id, shape_type)
                );
    else if (shape_type == angeo::COLLISION_SHAPE_TYPE::SPHERE)
        insert_collision_sphere_to_scene_node(
                data.get<float_32_bit>("radius"),
                angeo::read_collison_material_from_string(data.get<std::string>("material")),
                angeo::read_collison_class_from_string(data.get<std::string>("collision_class")),
                data.get<float_32_bit>("density_multiplier"),
                data.get<bool>("is_dynamic"),
                scn::make_collider_record_id(id, shape_type)
                );
    else if (shape_type == angeo::COLLISION_SHAPE_TYPE::TRIANGLE)
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
                angeo::read_collison_class_from_string(data.get<std::string>("collision_class")),
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

    data.put("shape_type", angeo::as_string(angeo::get_shape_type(collider.id())));
    switch (angeo::get_shape_type(collider.id()))
    {
    case angeo::COLLISION_SHAPE_TYPE::BOX:
        data.put("half_size_along_x", m_collision_scene_ptr->get_box_half_sizes_along_axes(collider.id())(0));
        data.put("half_size_along_y", m_collision_scene_ptr->get_box_half_sizes_along_axes(collider.id())(1));
        data.put("half_size_along_z", m_collision_scene_ptr->get_box_half_sizes_along_axes(collider.id())(2));
        break;
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        data.put("half_distance_between_end_points", m_collision_scene_ptr->get_capsule_half_distance_between_end_points(collider.id()));
        data.put("thickness_from_central_line", m_collision_scene_ptr->get_capsule_thickness_from_central_line(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        data.put("radius", m_collision_scene_ptr->get_sphere_radius(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
        {
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
    data.put("collision_class", to_string(m_collision_scene_ptr->get_collision_class(collider.id())));
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

    auto const  load_matrix33 = [&data](std::string const&  key) -> matrix33 {
        matrix33  M;
        boost::property_tree::path const  key_path(key, '/');
        M(0,0) = data.get<float_32_bit>(key_path / "00");
        M(0,1) = data.get<float_32_bit>(key_path / "01");
        M(0,2) = data.get<float_32_bit>(key_path / "02");
        M(1,0) = data.get<float_32_bit>(key_path / "10");
        M(1,1) = data.get<float_32_bit>(key_path / "11");
        M(1,2) = data.get<float_32_bit>(key_path / "12");
        M(2,0) = data.get<float_32_bit>(key_path / "20");
        M(2,1) = data.get<float_32_bit>(key_path / "21");
        M(2,2) = data.get<float_32_bit>(key_path / "22");
        return M;
    };

    bool  auto_compute_mass_and_inertia = data.count("mass_inverted") == 0;

    scn::rigid_body_props  rb_props;
    rb_props.m_linear_velocity = load_vector("linear_velocity"),
    rb_props.m_angular_velocity = load_vector("angular_velocity"),
    rb_props.m_external_linear_acceleration = load_vector("external_linear_acceleration"),
    rb_props.m_external_angular_acceleration = load_vector("external_angular_acceleration"),
    rb_props.m_mass_inverted = auto_compute_mass_and_inertia ? 0.0f : data.get<float_32_bit>("mass_inverted");
    rb_props.m_inertia_tensor_inverted = auto_compute_mass_and_inertia ? matrix33_zero() : load_matrix33("inertia_tensor_inverted");

    insert_rigid_body_to_scene_node(rb_props, auto_compute_mass_and_inertia, id);
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
        data.put(key_path / "01", M(0,1));
        data.put(key_path / "02", M(0,2));
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
    scn::agent_props const  props{
        ai::as_agent_kind(data.get<std::string>("kind")),
        scn::create_skeleton_props(skeleton_dir, skeletal_motion_templates),
        as_sensor_action_map(data.get_child("sensor_action_map"), id.get_node_id())
    };
    insert_agent(id, props);
}


void  simulator::save_agent(scn::scene_node_ptr const  node_ptr, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    scn::agent* const  agent_ptr = scn::get_agent(*node_ptr);
    ASSUMPTION(agent_ptr != nullptr);
    data.put("kind", ai::as_string(agent_ptr->get_props().m_agent_kind));
    data.put("skeleton_dir", agent_ptr->get_props().m_skeleton_props->skeleton_directory.string());
    data.put_child("sensor_action_map", as_ptree(agent_ptr->get_props().m_sensor_action_map, node_ptr->get_id()));
}


void  simulator::load_device(boost::property_tree::ptree const&  data, scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    boost::filesystem::path const  skeleton_dir = data.get<std::string>("skeleton_dir");
    scn::device_props const  props{
        ai::as_device_kind(data.get<std::string>("kind")),
        scn::create_skeleton_props(
                skeleton_dir,
                skeleton_dir.empty() ? ai::skeletal_motion_templates() : ai::skeletal_motion_templates(skeleton_dir, 75U)
                ),
        as_sensor_action_map(data.get_child("sensor_action_map"), id.get_node_id())
    };
    insert_device(id, props);
}


void  simulator::save_device(scn::scene_node_ptr const  node_ptr, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    scn::device* const  device_ptr = scn::get_device(*node_ptr);
    ASSUMPTION(device_ptr != nullptr);
    data.put("kind", ai::as_string(device_ptr->get_props().m_device_kind));
    data.put("skeleton_dir", device_ptr->get_props().m_skeleton_props->skeleton_directory.string());
    data.put_child("sensor_action_map", as_ptree(device_ptr->get_props().m_sensor_action_map, node_ptr->get_id()));
}


void  simulator::load_sensor(boost::property_tree::ptree const&  data, scn::scene_record_id const&  id)
{
    TMPROF_BLOCK();

    scn::sensor_props const  props{
        as_sensor_kind(data.get<std::string>("kind")),
        data.get<bool>("enabled", true),
        as_property_map(data.get_child("property_map"))
    };
    insert_sensor(id, props);
}


void  simulator::save_sensor(scn::scene_node_ptr const  node_ptr, scn::scene_node_record_id const&  id, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    scn::sensor const* const  sensor_ptr = scn::get_sensor(*node_ptr, id.get_record_name());
    ASSUMPTION(sensor_ptr != nullptr);
    data.put("kind", as_string(sensor_ptr->get_props().m_sensor_kind));
    data.put("enabled", sensor_ptr->get_props().m_enabled);
    data.put_child("property_map", as_ptree(sensor_ptr->get_props().m_sensor_props));
}


void  simulator::clear_scene()
{
    m_scene_selection.clear();
    m_scene_edit_data.invalidate_data();
    m_cache_of_batches_of_colliders.clear();

    m_cache_of_batches_of_ai_agents.lines.release();
    m_cache_of_batches_of_ai_agents.lines_batch.release();
    m_cache_of_batches_of_ai_agents.sight_frustum_batches.clear();

    get_ai_simulator()->clear();
    m_ai_requests_immediate.clear();
    m_ai_requests_delayed.clear();

    m_rigid_body_simulator_ptr->clear();
    m_binding_of_rigid_bodies.clear();

    m_rigid_bodies_external_linear_accelerations.clear();
    m_rigid_bodies_external_angular_accelerations.clear();

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
    ASSUMPTION(node_ptr != nullptr);

    std::vector<angeo::collision_object_id>  coids;
    std::vector<scn::scene_node_ptr>  coid_nodes;
    collect_colliders_in_subtree(node_ptr, coids, &coid_nodes);
    for (std::size_t i = 0UL; i != coids.size(); ++i)
        m_collision_scene_ptr->on_position_changed(coids.at(i), coid_nodes.at(i)->get_world_matrix());

    auto const  relocate_rigid_body = [this](scn::rigid_body&  rb, scn::scene_node_ptr const  rb_node_ptr) -> void {
        vector3  origin;
        matrix33  R;
        decompose_matrix44(rb_node_ptr->get_world_matrix(), origin, R);
        m_rigid_body_simulator_ptr->set_position_of_mass_centre(rb.id(), origin);
        m_rigid_body_simulator_ptr->set_orientation(rb.id(), rotation_matrix_to_quaternion(R));
    };

    foreach_rigid_body_in_subtree(node_ptr, relocate_rigid_body);

    if (!coids.empty())
    {
        scn::scene_node_ptr const  phs_node = find_nearest_rigid_body_node(node_ptr);
        if (phs_node != nullptr && phs_node != node_ptr)
            rebuild_rigid_body_due_to_change_in_subtree(phs_node);
    }
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
