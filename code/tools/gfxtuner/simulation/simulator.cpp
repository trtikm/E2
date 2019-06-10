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
#include <ai/skeleton_utils.hpp>
#include <ai/action_controller_human.hpp>
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
        return vector3(
            ((float_32_bit const*)vertex_buffer.data().data())
            + 3U * *(((natural_32_bit const*)index_buffer.data().data()) + 3U * triangle_index + vertex_index)
            );
    }

    qtgl::buffer  get_vertex_buffer() const { return vertex_buffer; }
    qtgl::buffer  get_index_buffer() const { return index_buffer; }

private:
    qtgl::buffer  vertex_buffer;
    qtgl::buffer  index_buffer;
};


void  skeleton_enumerate_nodes_of_bones(
        scn::scene_node_ptr const  agent_node_ptr,
        ai::skeleton_composition const&  skeleton_composition,
        std::function<
                bool(    // Continue in the enumeration of the remaining bones? I.e. 'false' early terminates the enumeration.
                    natural_32_bit,         // bone index
                    scn::scene_node_ptr,    // the corresponding node in the scene; can be 'nullptr', if the node is not in the scene.
                    bool                    // True if the bone has a parent bone, and False otherwise.
                    )> const&  callback // Called for each bone, in the fixed increasing order of the bone index: 0,1,2,...
        )
{
    std::vector<scn::scene_node_ptr>  nodes{ agent_node_ptr };
    for (natural_32_bit bone = 0U; bone != skeleton_composition.pose_frames.size(); ++bone)
    {
        scn::scene_node_ptr  child_node_ptr = nullptr;
        {
            scn::scene_node_ptr const  parent_node_ptr = nodes.at(skeleton_composition.parents.at(bone) + 1);
            if (parent_node_ptr != nullptr)
            {
                auto const  child_node_it = parent_node_ptr->find_child(skeleton_composition.names.at(bone));
                if (child_node_it != parent_node_ptr->get_children().cend())
                    child_node_ptr = child_node_it->second;
            }
        }
        if (callback(bone, child_node_ptr, skeleton_composition.parents.at(bone) >= 0) == false)
            break;
        nodes.push_back(child_node_ptr);
    }
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
    , m_camera_target_node_id()
    , m_camera_target_vector_in_camera_space(vector3_zero())
    , m_camera_target_vector_invalidated(true)

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
    , m_do_show_ai_action_controller_props(false)
    , m_colliders_colour{ 0.75f, 0.75f, 1.0f, 1.0f }
    , m_render_in_wireframe(false)

    // Common and shared data for both modes: Editing and Simulation

    , m_paused(true)
    , m_do_single_step(false)
    , m_fixed_time_step_in_seconds(1.0 / 25.0)
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
    , m_agents_ptr(std::make_shared<ai::agents>(std::make_shared<bind_ai_scene_to_simulator>(this)))

    , m_binding_of_collision_objects()
    , m_binding_of_rigid_bodies()
    , m_binding_of_agents_to_scene()

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


void  __agent_look_at_object(
        std::string const&  agent_name,
        std::string const&  object_name,
        scn::scene const&  scene,
        float_64_bit const  seconds_from_previous_call
        )
{
    TMPROF_BLOCK();

    static bool  initialise_static_data = true;

    scn::scene_node_ptr const  agent_node_ptr = scene.get_scene_node(scn::scene_node_id(agent_name));
    scn::scene_node_const_ptr const  target_node = scene.get_scene_node(scn::scene_node_id(object_name));
    if (agent_node_ptr == nullptr || target_node == nullptr)
    {
        initialise_static_data = true;
        return;
    }
    scn::scene_node_ptr const  ske_root_ptr = agent_node_ptr->find_child(scn::scene_node_id() / "lower_body" / "middle_body" / "upper_body");
    if (ske_root_ptr == nullptr)
        return;
    std::vector<scn::scene_node_ptr> const  agent_nodes{                                // bone:
        ske_root_ptr->find_child(scn::scene_node_id() / "neck"),                      // 0
        ske_root_ptr->find_child(scn::scene_node_id() / "neck" / "head"),             // 1
        ske_root_ptr->find_child(scn::scene_node_id() / "neck" / "head" / "eye.L"),   // 2
        ske_root_ptr->find_child(scn::scene_node_id() / "neck" / "head" / "eye.R"),   // 3
    };
    for (auto const& node_ptr : agent_nodes)
        if (node_ptr == nullptr)
            return;

    static std::vector<angeo::coordinate_system>  pose_frames;
    static std::vector<integer_32_bit>  parents;
    static std::vector<std::vector<integer_32_bit> >  children;
    static std::vector<std::vector<angeo::joint_rotation_props> > rotation_props;

    if (initialise_static_data)
    {
        initialise_static_data = false;

        parents = {
               // bone:
           -1, // 0
            0, // 1
            1, // 2
            1, // 3
        };

        children.clear();
        angeo::skeleton_compute_child_bones(parents, children);

        pose_frames.clear();
        for (auto const& node_ptr : agent_nodes)
            pose_frames.push_back(*node_ptr->get_coord_system());

        auto vector_to_bone_space = [agent_node_ptr, ske_root_ptr, &agent_nodes](vector3 const&  v, integer_32_bit const  bone) -> vector3 {
            matrix44 const M = inverse44((bone < 0 ? ske_root_ptr : agent_nodes.at(bone))->get_world_matrix()) * agent_node_ptr->get_world_matrix();
            return normalised(transform_vector(v, M));
        };
        rotation_props = {
            { // bone 0
                {
                    vector_to_bone_space(vector3_unit_z(), parents.at(0)),    // m_axis
                    true,                           // m_axis_in_parent_space
                    vector_to_bone_space(-vector3_unit_y(), parents.at(0)),    // m_zero_angle_direction
                    vector3_unit_x(),               // m_direction
                    PI() * 30.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 1.0f,                    // m_max_angular_speed
                },
                {
                    vector3_unit_z(),               // m_axis
                    false,                          // m_axis_in_parent_space
                    vector_to_bone_space(vector3{0.0f, -0.2f, 1.0f}, parents.at(0)),   // m_zero_angle_direction
                    vector3_unit_y(),               // m_direction
                    PI() * 30.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 1.0f,                    // m_max_angular_speed
                }
            },
            { // bone 1
                {
                    vector_to_bone_space(vector3_unit_z(), parents.at(1)),    // m_axis
                    true,                           // m_axis_in_parent_space
                    vector_to_bone_space(-vector3_unit_y(), parents.at(1)),    // m_zero_angle_direction
                    vector3_unit_x(),               // m_direction
                    PI() * 120.0f / 180.0f,         // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 2.0f,                    // m_max_angular_speed
                },
                {
                    vector3_unit_z(),               // m_axis
                    false,                          // m_axis_in_parent_space
                    normalised(vector3{ -0.207483f, 0.978231f, -0.00315839f }),   // m_zero_angle_direction
                    vector3_unit_y(),               // m_direction
                    PI() * 80.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 2.0f,                    // m_max_angular_speed
                }
            },
            { // bone 2
                {
                    vector3_unit_y(),               // m_axis
                    true,                           // m_axis_in_parent_space
                    vector3_unit_x(),               // m_zero_angle_direction
                    vector3_unit_y(),               // m_direction
                    PI() * 50.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 4.0f,                    // m_max_angular_speed
                },
                {
                    vector3_unit_x(),               // m_axis
                    false,                          // m_axis_in_parent_space
                    vector3_unit_y(),               // m_zero_angle_direction
                    vector3_unit_z(),               // m_direction
                    PI() * 30.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 4.0f,                    // m_max_angular_speed
                }
            },
            { // bone 3
                {
                    vector3_unit_y(),               // m_axis
                    true,                           // m_axis_in_parent_space
                    vector3_unit_x(),               // m_zero_angle_direction
                    vector3_unit_y(),               // m_direction
                    PI() * 50.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 4.0f,                    // m_max_angular_speed
                },
                {
                    vector3_unit_x(),               // m_axis
                    false,                          // m_axis_in_parent_space
                    vector3_unit_y(),               // m_zero_angle_direction
                    vector3_unit_z(),               // m_direction
                    PI() * 30.0f / 180.0f,          // m_max_angle
                    1.0f,                           // m_stiffness_with_parent_bone
                    PI() * 4.0f,                    // m_max_angular_speed
                }
            },
        };
    }

    vector3 const  target =
        transform_point(vector3_zero(), inverse44(agent_nodes.at(0)->get_parent()->get_world_matrix()) * target_node->get_world_matrix());
    angeo::bone_look_at_targets const  look_at_targets {
        { 2, { vector3_unit_y(), target } },
        { 3, { vector3_unit_y(), target } },
    };

    std::vector<angeo::coordinate_system>  target_frames;
    std::unordered_map<integer_32_bit, std::vector<natural_32_bit> >  bones_to_rotate;
    angeo::skeleton_look_at(target_frames, look_at_targets, pose_frames, parents, rotation_props, &bones_to_rotate);

    std::vector<angeo::coordinate_system>  frames;
    for (auto const& node_ptr : agent_nodes)
        frames.push_back(*node_ptr->get_coord_system());

    angeo::skeleton_rotate_bones_towards_target_pose(frames, target_frames, rotation_props, bones_to_rotate, seconds_from_previous_call);

    for (natural_32_bit  i = 0U; i != agent_nodes.size(); ++i)
        agent_nodes.at(i)->relocate(frames.at(i).origin(), frames.at(i).orientation());
}


void  simulator::next_round(float_64_bit  seconds_from_previous_call,
                            bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    seconds_from_previous_call = std::min(seconds_from_previous_call, m_fixed_time_step_in_seconds);

    bool  is_simulation_round = false;
    if (!is_this_pure_redraw_request)
    {
        qtgl::adjust(*m_camera,window_props());

        bool  camera_controller_changed = false;
        std::pair<bool, bool>  translated_rotated{false, false};
        if (paused())
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

                translated_rotated =
                    qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                   seconds_from_previous_call, mouse_props(), keyboard_props());
                if (translated_rotated.second == true)
                    translated_rotated.first = true;

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
                translated_rotated = qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
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
                translated_rotated = qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                                                    seconds_from_previous_call, mouse_props(), keyboard_props());
            else
            {
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
                        translated_rotated = qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                                            seconds_from_previous_call, mouse_props(), keyboard_props());
                        if (translated_rotated.second == true)
                            translated_rotated.first = true;

                        matrix44 F;
                        angeo::from_base_matrix(*m_camera->coordinate_system(), F);

                        m_camera->coordinate_system()->set_origin(center - transform_vector(m_camera_target_vector_in_camera_space, F));
                    }
                    break;
                case CAMERA_CONTROLLER_FOLLOW:
                    {
                        translated_rotated = qtgl::free_fly(*m_camera->coordinate_system(), m_orbit_config,
                                                            seconds_from_previous_call, mouse_props(), keyboard_props());

                        translated_rotated.first = true;
                        vector3  d = m_camera->coordinate_system()->origin() - center;
                        float_32_bit const  d_len = length(d);
                        if (d_len > 0.0001f)
                            d *= length(m_camera_target_vector_in_camera_space) / d_len;

                        m_camera->coordinate_system()->set_origin(center + d);
                    }
                    break;
                case CAMERA_CONTROLLER_LOOK_AT:
                    {
                        translated_rotated.second = true;
                        vector3  d = m_camera->coordinate_system()->origin() - center;
                        qtgl::look_at(*m_camera->coordinate_system(), center, length(d));
                    }
                    break;
                case CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT:
                    {
                        translated_rotated.first = true;
                        translated_rotated.second = true;
                        qtgl::look_at(*m_camera->coordinate_system(), center, length(m_camera_target_vector_in_camera_space));
                }
                    break;
                default: UNREACHABLE(); break;
                }
            }
        }
        else
            translated_rotated = qtgl::free_fly(*m_camera->coordinate_system(), m_free_fly_config,
                                                seconds_from_previous_call, mouse_props(), keyboard_props());

        if (camera_controller_changed)
            call_listeners(simulator_notifications::camera_controller_changed());
        if (translated_rotated.first)
            call_listeners(simulator_notifications::camera_position_updated());
        if (translated_rotated.second)
            call_listeners(simulator_notifications::camera_orientation_updated());

        bool const  old_paused = paused();

        if (keyboard_props().was_just_released(qtgl::KEY_SPACE()))
        {
            if (paused())
                m_paused = !m_paused;
            m_do_single_step = true;
        }

        if (!m_do_single_step && keyboard_props().was_just_released(qtgl::KEY_PAUSE()))
            m_paused = !m_paused;

        get_collision_scene()->get_statistics().on_next_frame();

//__agent_look_at_object("agent", "apple", get_scene(), seconds_from_previous_call);

        if (!paused())
        {
            is_simulation_round = true;
            if (old_paused != paused())
                on_simulation_resumed();
            perform_simulation_step(m_do_single_step ? m_fixed_time_step_in_seconds : seconds_from_previous_call);
        }
        else
        {
            if (old_paused != paused())
                on_simulation_paused();
            perform_scene_update(seconds_from_previous_call);
        }

        if (m_do_single_step)
        {
            m_paused = true;
            on_simulation_paused();
            m_do_single_step = false;
        }
    }

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

    if (is_simulation_round)
        render_simulation_state(matrix_from_world_to_camera, matrix_from_camera_to_clipspace ,draw_state);
    else
    {
        if (m_do_show_batches)
            render_scene_batches(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
        render_scene_coord_systems(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
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

    m_cache_of_batches_of_colliders.capsules.clear();
    m_cache_of_batches_of_colliders.spheres.clear();
    m_cache_of_batches_of_colliders.triangle_meshes.clear();
    m_cache_of_batches_of_colliders.collision_normals_points.release();
    m_cache_of_batches_of_colliders.collision_normals_batch.release();

    m_cache_of_batches_of_ai_agents.lines.release();
    m_cache_of_batches_of_ai_agents.lines_batch.release();

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
            if (m_agents_ptr->at(agent_id_and_node_id.first).ready())
            {
                ai::action_controller const* const  action_controller_ptr =
                        &m_agents_ptr->at(agent_id_and_node_id.first).get_action_controller();
                if (auto const  ach_ptr = dynamic_cast<ai::action_controller_human const*>(action_controller_ptr))
                {
                    scn::scene_node_ptr const  node_ptr = get_scene_node(ach_ptr->get_motion_object_node_id());
                    if (auto const  rb_ptr = scn::get_rigid_body(*node_ptr))
                    {
                        matrix44  motion_object_from_base_matrix;
                        angeo::from_base_matrix(*node_ptr->get_coord_system(), motion_object_from_base_matrix);

                        vector3 const  motion_object_forward_direction_in_world_space =
                            transform_vector(ach_ptr->get_blackboard()->m_skeleton_composition->forward_direction_in_anim_space, motion_object_from_base_matrix);
                        vector3 const  motion_object_up_direction_in_world_space =
                            transform_vector(ach_ptr->get_blackboard()->m_skeleton_composition->up_direction_in_anim_space, motion_object_from_base_matrix);

                        m_cache_of_batches_of_ai_agents.lines->first.push_back({
                                node_ptr->get_coord_system()->origin(),
                                node_ptr->get_coord_system()->origin() + motion_object_forward_direction_in_world_space
                                });
                        m_cache_of_batches_of_ai_agents.lines->second.push_back({0.25f, 0.75f, 0.75f, 1.0f});

                        m_cache_of_batches_of_ai_agents.lines->first.push_back({
                                node_ptr->get_coord_system()->origin(),
                                node_ptr->get_coord_system()->origin() + motion_object_up_direction_in_world_space
                                });
                        m_cache_of_batches_of_ai_agents.lines->second.push_back({0.25f, 0.5f, 0.75f, 1.0f});

                        m_cache_of_batches_of_ai_agents.lines->first.push_back({
                                node_ptr->get_coord_system()->origin(),
                                node_ptr->get_coord_system()->origin() + m_rigid_body_simulator_ptr->get_linear_velocity(rb_ptr->id())
                                });
                        m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 1.0f, 0.25f, 1.0f});

                        m_cache_of_batches_of_ai_agents.lines->first.push_back({
                                node_ptr->get_coord_system()->origin(),
                                node_ptr->get_coord_system()->origin() + m_rigid_body_simulator_ptr->get_angular_velocity(rb_ptr->id())
                                });
                        m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 0.5f, 0.25f, 1.0f});
                    }

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin() + 0.005f * vector3_unit_z(),
                            node_ptr->get_coord_system()->origin() + 0.005f * vector3_unit_z()
                                + ach_ptr->get_desired_props().forward_unit_vector_in_world_space
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 1.0f, 1.0f, 1.0f});

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin() + 0.0075f * vector3_unit_z(),
                            node_ptr->get_coord_system()->origin() + 0.0075f * vector3_unit_z()
                                + 0.75f * ach_ptr->get_desired_props().linear_velocity_unit_direction_in_world_space
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({1.0f, 0.5f, 1.0f, 1.0f});

                    m_cache_of_batches_of_ai_agents.lines->first.push_back({
                            node_ptr->get_coord_system()->origin() + 0.01f * vector3_unit_z(),
                            node_ptr->get_coord_system()->origin() + 0.01f * vector3_unit_z()
                                + ach_ptr->get_desired_props().linear_speed
                                  * ach_ptr->get_desired_props().linear_velocity_unit_direction_in_world_space
                            });
                    m_cache_of_batches_of_ai_agents.lines->second.push_back({0.75f, 0.25f, 0.75f, 1.0f});
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

    std::unordered_map<
            std::string,    // batch ID
            std::pair<qtgl::batch, std::vector<scn::scene_node_ptr> > >
        batches;
    get_scene().foreach_node(
        [this, &batches](scn::scene_node_ptr const  node_ptr) -> bool {
                for (auto const& name_holder : scn::get_batch_holders(*node_ptr))
                {
                    qtgl::batch const  batch = scn::as_batch(name_holder.second);
                    if (!batch.loaded_successfully())
                        continue;
                    auto&  record = batches[batch.path_component_of_uid()];
                    if (record.first.empty())
                        record.first = batch;
                    INVARIANT(record.first == batch);
                    record.second.push_back(node_ptr);
                }
                return true;
            },
        false
        );
    for (auto const& elem : batches)
    {
        bool const  use_instancing =
                elem.second.first.get_available_resources().skeletal() == nullptr &&
                elem.second.first.has_instancing_data() &&
                elem.second.second.size() > 1UL
                ;
        if (!qtgl::make_current(elem.second.first, draw_state, use_instancing))
            continue;

        if (use_instancing)
        {
            qtgl::vertex_shader_instanced_data_provider  instanced_data_provider(elem.second.first);
            for (auto const& node_ptr : elem.second.second)
                instanced_data_provider.insert_from_model_to_camera_matrix(
                        matrix_from_world_to_camera * node_ptr->get_world_matrix()
                        );
            qtgl::render_batch(
                    elem.second.first,
                    instanced_data_provider,
                    qtgl::vertex_shader_uniform_data_provider(
                            elem.second.first,
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
            for (auto const& node_ptr : elem.second.second)
            {
                scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
                if (agent_ptr == nullptr)
                {
                    qtgl::render_batch(
                            elem.second.first,
                            qtgl::vertex_shader_uniform_data_provider(
                                    elem.second.first,
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
                    scn::agent const* const  agent_ptr = scn::get_agent(*node_ptr);
                    ai::skeleton_composition_const_ptr const  skeleton_composition = agent_ptr->get_skeleton_props()->skeleton_composition;
                    detail::skeleton_enumerate_nodes_of_bones(
                            node_ptr,
                            *skeleton_composition,
                            [&frame, &matrix_from_world_to_camera, skeleton_composition](
                                natural_32_bit const bone, scn::scene_node_ptr const  bone_node_ptr, bool const  has_parent) -> bool
                                {
                                    if (bone_node_ptr != nullptr)
                                        frame.push_back(matrix_from_world_to_camera * bone_node_ptr->get_world_matrix());
                                    else
                                    {
                                        matrix44 M;
                                        angeo::from_base_matrix(skeleton_composition->pose_frames.at(bone), M);
                                        frame.push_back(matrix_from_world_to_camera * bone_node_ptr->get_world_matrix() * M);
                                    }
                                    return true;
                                }
                            );
                }

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
                    qtgl::batch  batch;
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
                                batch = it->second;
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
                                batch = it->second;
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
                                    it = m_cache_of_batches_of_colliders.triangle_meshes.insert({
                                                key,
                                                qtgl::create_triangle_mesh(
                                                        vertices_getter_ptr->get_vertex_buffer(),
                                                        vertices_getter_ptr->get_index_buffer(),
                                                        m_colliders_colour
                                                        )
                                                }).first;
                                batch = it->second;
                            }
                            break;
                        default:
                            NOT_IMPLEMENTED_YET();
                        }
                    }
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
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    ASSUMPTION(scn::has_node(get_scene(), id));
    auto const  batch = qtgl::batch(canonical_path(batch_pathname), get_effects_config());
    scn::insert_batch(*get_scene_node(id), batch_name, batch);
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
    std::vector<angeo::collision_object_id>  collider_ids;
    m_collision_scene_ptr->insert_triangle_mesh(
            index_buffer.num_primitives(),
            detail::collider_triangle_mesh_vertex_getter(vertex_buffer, index_buffer),
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


void  simulator::insert_agent(scn::scene_record_id const&  id, scn::skeleton_props_ptr const  props)
{
    TMPROF_BLOCK();

    scn::scene_node_ptr const  node_ptr = get_scene_node(id.get_node_id());
    ASSUMPTION(node_ptr != nullptr && !node_ptr->has_parent());
    ai::agent_id const  agent_id =
            m_agents_ptr->insert(
                    id.get_node_id(),
                    props->skeleton_composition,
                    props->skeletal_motion_templates
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
    std::string  error_msg;
    ai::skeleton_composition_ptr const  skeleton_composition =
        ai::load_skeleton_composition(skeleton_dir, error_msg);
    ASSUMPTION(skeleton_composition != nullptr && error_msg.empty());
    ai::skeletal_motion_templates_ptr const skeletal_motion_templates =
        ai::load_skeletal_motion_templates(skeleton_dir, error_msg);
    ASSUMPTION(skeletal_motion_templates != nullptr && error_msg.empty());
    scn::skeleton_props_ptr const  props =
        scn::create_skeleton_props(skeleton_dir, skeleton_composition, skeletal_motion_templates);
    insert_agent(id, props);
}


void  simulator::save_agent(scn::scene_node_ptr const  node_ptr, boost::property_tree::ptree&  data)
{
    TMPROF_BLOCK();

    scn::agent* const  agent_ptr = scn::get_agent(*node_ptr);
    ASSUMPTION(agent_ptr != nullptr);
    data.put("skeleton_dir", agent_ptr->get_skeleton_props()->skeleton_directory.string());
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
