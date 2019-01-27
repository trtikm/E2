#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <gfxtuner/program_options.hpp>
#include <scene/scene_utils.hpp>
#include <scene/scene_utils_specialised.hpp>
#include <angeo/collide.hpp>
#include <angeo/mass_and_inertia_tensor.hpp>
#include <angeo/skeleton_kinematics.hpp>
#include <angeo/utility.hpp>
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
    , m_do_show_batches(true)
    , m_do_show_colliders(true)
    , m_do_show_contact_normals(false)
    , m_colliders_colour{ 0.75f, 0.75f, 1.0f, 1.0f }
    , m_render_in_wireframe(false)

    // Common and shared data for both modes: Editing and Simulation

    , m_paused(true)
    , m_do_single_step(false)
    , m_fixed_time_step_in_seconds(1.0 / 25.0)
    , m_scene(new scn::scene)
    , m_scene_nodes_relocated_during_simulation()
    , m_scene_records_inserted_during_simulation()
    , m_scene_records_erased_during_simulation()
    , m_cache_of_batches_of_colliders()
    , m_skeletons()

    // Editing mode data

    , m_scene_selection(m_scene)
    , m_scene_history(new scn::scene_history)
    , m_scene_edit_data(scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT)
    , m_batch_coord_system(qtgl::create_basis_vectors())
    , m_invalidated_nodes_of_rigid_bodies()

    // Simulation mode data

    , m_collision_scene()
    , m_rigid_body_simulator()
    , m_binding_of_collision_objects()
    , m_binding_of_rigid_bodies()
    , m_static_rigid_body_backups()

    , m_gfx_animated_objects()

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


void  __agent_look_at_object(
        std::string const&  agent_name,
        std::string const&  object_name,
        scn::scene const&  scene,
        float_64_bit const  seconds_from_previous_call
        )
{
    std::vector<scn::scene_node_ptr> const  agent_nodes{                                                                                // bone:
        scene.get_scene_node(scn::scene_node_id(agent_name) / "lower_body" / "middle_body" / "upper_body" / "neck"),                    // 0
        scene.get_scene_node(scn::scene_node_id(agent_name) / "lower_body" / "middle_body" / "upper_body" / "neck" / "head"),           // 1
        scene.get_scene_node(scn::scene_node_id(agent_name) / "lower_body" / "middle_body" / "upper_body" / "neck" / "head" / "eye.L"), // 2
        scene.get_scene_node(scn::scene_node_id(agent_name) / "lower_body" / "middle_body" / "upper_body" / "neck" / "head" / "eye.R"), // 3
    };
    std::vector<integer_32_bit> const  parents {
            // bone:
        -1, // 0
         0, // 1
         1, // 2
         1, // 3
    };
    std::vector<angeo::coordinate_system>  frames;
    for (auto const&  node_ptr : agent_nodes)
    {
        if (node_ptr == nullptr)
            return;
        frames.push_back(*node_ptr->get_coord_system());
    }
    std::vector<std::vector<angeo::joint_rotation_props> > const  rotation_props {
        { // bone 0
            // TODO!
        },
        { // bone 1
            // TODO!
        },
        { // bone 2
            {
                vector3_unit_y(),               // m_axis
                true,                           // m_axis_in_parent_space
                PI() * 2.0f,                    // m_max_angular_speed
                
                vector3_unit_x(),               // m_zero_angle_direction
                vector3_unit_y(),               // m_direction

                PI() * 50.0f / 180.0f           // m_max_angle
            },
            {
                vector3_unit_x(),               // m_axis
                false,                          // m_axis_in_parent_space
                PI() * 2.0f,                    // m_max_angular_speed

                vector3_unit_y(),               // m_zero_angle_direction
                vector3_unit_z(),               // m_direction
                
                PI() * 30.0f / 180.0f           // m_max_angle
            }
        },
        { // bone 3
            {
                vector3_unit_y(),               // m_axis
                true,                           // m_axis_in_parent_space
                PI() * 2.0f,                    // m_max_angular_speed
                
                vector3_unit_x(),               // m_zero_angle_direction
                vector3_unit_y(),               // m_direction

                PI() * 50.0f / 180.0f           // m_max_angle
            },
            {
                vector3_unit_x(),               // m_axis
                false,                          // m_axis_in_parent_space
                PI() * 2.0f,                    // m_max_angular_speed

                vector3_unit_y(),               // m_zero_angle_direction
                vector3_unit_z(),               // m_direction
                
                PI() * 30.0f / 180.0f           // m_max_angle
            }
        },
    };
    scn::scene_node_const_ptr const  target_node = scene.get_scene_node(scn::scene_node_id(object_name));
    if (target_node == nullptr)
        return;
    vector3 const  target =
        transform_point(vector3_zero(), inverse44(agent_nodes.at(0)->get_parent()->get_world_matrix()) * target_node->get_world_matrix());
    angeo::bone_look_at_targets const  look_at_targets {
        { 2, { vector3_unit_y(), target } },
        { 3, { vector3_unit_y(), target } },
    };

    angeo::skeleton_bones_move_towards_targets(frames, parents, rotation_props, look_at_targets, seconds_from_previous_call);

    for (natural_32_bit  i = 0U; i != agent_nodes.size(); ++i)
        agent_nodes.at(i)->relocate(frames.at(i).origin(), frames.at(i).orientation());
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
                m_paused = !m_paused;
            m_do_single_step = true;
        }

        if (!m_do_single_step && keyboard_props().was_just_released(qtgl::KEY_PAUSE()))
            m_paused = !m_paused;

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

    if (m_do_show_colliders)
        render_colliders(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);
    if (m_do_show_contact_normals)
        render_contact_normals(matrix_from_world_to_camera, matrix_from_camera_to_clipspace, draw_state);

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
    TMPROF_BLOCK();

    for (auto const&  rbid_and_backup : m_static_rigid_body_backups)
        if (m_rigid_body_simulator.contains(rbid_and_backup.first))
        {
            m_rigid_body_simulator.set_linear_velocity(rbid_and_backup.first, rbid_and_backup.second.m_linear_velocity);
            m_rigid_body_simulator.set_angular_velocity(rbid_and_backup.first, rbid_and_backup.second.m_angular_velocity);
            m_rigid_body_simulator.set_external_linear_acceleration(rbid_and_backup.first, rbid_and_backup.second.m_external_linear_acceleration);
            m_rigid_body_simulator.set_external_angular_acceleration(rbid_and_backup.first, rbid_and_backup.second.m_external_angular_acceleration);
        }

    call_listeners(simulator_notifications::paused());

    // TODO: process these before clear:
    //    m_scene_nodes_relocated_during_simulation
    //    m_scene_records_inserted_during_simulation
    //    m_scene_records_erased_during_simulation

    m_scene_nodes_relocated_during_simulation.clear();
    m_scene_records_inserted_during_simulation.clear();
    m_scene_records_erased_during_simulation.clear();
}


void  simulator::on_simulation_resumed()
{
    TMPROF_BLOCK();

    call_listeners(simulator_notifications::resumed());

    for (auto const&  node_name_and_collider_change : m_invalidated_nodes_of_rigid_bodies)
        if (auto  node_ptr = get_scene().get_scene_node(node_name_and_collider_change.first))
        {
            auto  rb_ptr = scn::get_rigid_body(*node_ptr);
            if (rb_ptr == nullptr)
                continue;

            std::vector<angeo::collision_object_id>  coids;
            std::vector<scn::scene_node_ptr>  coid_nodes;
            collect_colliders_in_subtree(node_ptr, coids, &coid_nodes);

            for (std::size_t  i = 0UL; i != coids.size(); ++i)
                m_collision_scene.on_position_changed(coids.at(i), coid_nodes.at(i)->get_world_matrix());
            
            if (node_name_and_collider_change.second)
            {
                m_binding_of_rigid_bodies.erase(rb_ptr->id());
                m_static_rigid_body_backups.erase(rb_ptr->id());

                vector3  linear_velocity = m_rigid_body_simulator.get_linear_velocity(rb_ptr->id());
                vector3  angular_velocity = m_rigid_body_simulator.get_angular_velocity(rb_ptr->id());
                vector3  external_linear_acceleration = m_rigid_body_simulator.get_external_linear_acceleration(rb_ptr->id());
                vector3  external_angular_acceleration = m_rigid_body_simulator.get_external_angular_acceleration(rb_ptr->id());

                static_rigid_body_backup  rb_backup;

                m_rigid_body_simulator.erase_rigid_body(rb_ptr->id());

                float_32_bit  inverted_mass = 0.0f;
                matrix33  inverted_inertia_tensor_in_local_space = matrix33_zero();

                bool  has_static_collider = false;
                for (angeo::collision_object_id  coid : coids)
                    if (!m_collision_scene.is_dynamic(coid))
                    {
                        has_static_collider = true;
                        break;
                    }
                if (!has_static_collider)
                {
                    for (std::size_t i = 0U; i < coids.size(); ++i)
                        for (std::size_t j = i + 1U; j < coids.size(); ++j)
                            m_collision_scene.disable_colliding(coids.at(i), coids.at(j));

                    angeo::mass_and_inertia_tensor_builder  builder;
                    for (std::size_t i = 0UL; i != coids.size(); ++i)
                    {
                        angeo::collision_object_id const  coid = coids.at(i);
                        scn::scene_node_ptr const  coid_node_ptr = coid_nodes.at(i);
                        switch (angeo::get_shape_type(coid))
                        {
                        case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
                            builder.insert_capsule(
                                    m_collision_scene.get_capsule_half_distance_between_end_points(coid),
                                    m_collision_scene.get_capsule_thickness_from_central_line(coid),
                                    coid_node_ptr->get_world_matrix(),
                                    m_collision_scene.get_material(coid),
                                    1.0f // TODO!
                                    );
                            break;
                        case angeo::COLLISION_SHAPE_TYPE::SPHERE:
                            builder.insert_sphere(
                                    translation_vector(coid_node_ptr->get_world_matrix()),
                                    m_collision_scene.get_sphere_radius(coid),
                                    m_collision_scene.get_material(coid),
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
                    if (node_ptr->has_parent())
                    {
                        vector3 const  origin_shift_in_parent_space =
                                transform_vector(origin_shift_in_world_space,
                                                 inverse44(node_ptr->get_parent()->get_world_matrix()));
                        node_ptr->translate(origin_shift_in_parent_space);
                    }
                    else
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
                    rb_backup.m_linear_velocity = linear_velocity;
                    rb_backup.m_angular_velocity = angular_velocity;
                    rb_backup.m_external_linear_acceleration = external_linear_acceleration;
                    rb_backup.m_external_angular_acceleration = external_angular_acceleration;
                }

                matrix44 const  world_matrix = node_ptr->get_world_matrix();

                rb_ptr->set_id(
                        m_rigid_body_simulator.insert_rigid_body(
                                    translation_vector(world_matrix),
                                    rotation_matrix_to_quaternion(rotation_matrix(world_matrix)),
                                    inverted_mass,
                                    inverted_inertia_tensor_in_local_space,
                                    linear_velocity,
                                    angular_velocity,
                                    external_linear_acceleration,
                                    external_angular_acceleration
                                    )
                        );
                m_binding_of_rigid_bodies[rb_ptr->id()] = node_ptr;

                for (angeo::collision_object_id coid : coids)
                    m_binding_of_collision_objects[coid] = rb_ptr->id();

                if (has_static_collider)
                    m_static_rigid_body_backups.insert({rb_ptr->id(), rb_backup});
            }
            else
            {
                matrix44 const  world_matrix = node_ptr->get_world_matrix();
                m_rigid_body_simulator.set_position_of_mass_centre(rb_ptr->id(), translation_vector(world_matrix));
                m_rigid_body_simulator.set_orientation(rb_ptr->id(), rotation_matrix_to_quaternion(rotation_matrix(world_matrix))); 
            }
        }
    m_invalidated_nodes_of_rigid_bodies.clear();

    for (auto const& rbid_and_backup : m_static_rigid_body_backups)
        if (m_rigid_body_simulator.contains(rbid_and_backup.first))
        {
            m_rigid_body_simulator.set_linear_velocity(rbid_and_backup.first, vector3_zero());
            m_rigid_body_simulator.set_angular_velocity(rbid_and_backup.first, vector3_zero());
            m_rigid_body_simulator.set_external_linear_acceleration(rbid_and_backup.first, vector3_zero());
            m_rigid_body_simulator.set_external_angular_acceleration(rbid_and_backup.first, vector3_zero());
        }

    // TODO: The code below should be removed at some point.

    std::unordered_set<scn::scene_record_id>  to_remove;
    for (auto& elem : m_gfx_animated_objects)
        if (!scn::has_record(get_scene(), elem.first))
            to_remove.insert(elem.first);
    for (auto const&  key : to_remove)
        m_gfx_animated_objects.erase(key);

    get_scene().foreach_node(
        [this](scn::scene_node_ptr const  node_ptr) -> bool {
                for (auto const& name_holder : scn::get_batch_holders(*node_ptr))
                {
                    qtgl::batch const  batch = scn::as_batch(name_holder.second);
                    if (batch.ready() && batch.get_available_resources().skeletal() != nullptr)
                    {
                        scn::scene_record_id const  key = scn::make_batch_record_id(node_ptr->get_id(), name_holder.first);
                        if (m_gfx_animated_objects.count(key) == 0UL)
                            m_gfx_animated_objects.emplace(key, gfx_animated_object(batch));
                    }
                }
                return true;
            },
        false
        );
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

    constexpr float_64_bit  min_micro_time_step_in_seconds = 0.001;
    constexpr float_64_bit  max_micro_time_step_in_seconds = 0.04;
    static float_64_bit  time_buffer_in_seconds = 0.0f;
    static float_64_bit  duration_of_last_simulation_step_in_seconds = 0.01f;

    time_buffer_in_seconds += time_to_simulate_in_seconds;
    float_64_bit  max_computation_time_in_seconds = time_to_simulate_in_seconds / 4.0f;
    while (time_buffer_in_seconds >= min_micro_time_step_in_seconds)
    {
        natural_32_bit const  num_estimated_sub_steps =
                std::max(1U, (natural_32_bit)(max_computation_time_in_seconds / duration_of_last_simulation_step_in_seconds));
        float_32_bit const  micro_time_step_in_seconds =
                std::min(
                    max_micro_time_step_in_seconds,
                    std::max(min_micro_time_step_in_seconds, time_buffer_in_seconds / num_estimated_sub_steps)
                    );
        bool const  is_last_micro_step = time_buffer_in_seconds - micro_time_step_in_seconds < min_micro_time_step_in_seconds;
        std::chrono::high_resolution_clock::time_point const  start_time_point =
                std::chrono::high_resolution_clock::now();


        perform_simulation_micro_step(micro_time_step_in_seconds, is_last_micro_step);


        time_buffer_in_seconds -= micro_time_step_in_seconds;
        duration_of_last_simulation_step_in_seconds =
                std::chrono::duration<float_64_bit>(std::chrono::high_resolution_clock::now() - start_time_point).count();
        max_computation_time_in_seconds -= duration_of_last_simulation_step_in_seconds;
    }

    // TODO: The code below should be removed at some point.

    for (auto&  elem : m_gfx_animated_objects)
        elem.second.next_round(time_to_simulate_in_seconds);
}


void  simulator::perform_simulation_micro_step(float_64_bit const  time_to_simulate_in_seconds, bool const  is_last_micro_step)
{
    TMPROF_BLOCK();

    m_collision_scene.compute_contacts_of_all_dynamic_objects(
            [this, is_last_micro_step](
                angeo::contact_id const& cid,
                vector3 const& contact_point,
                vector3 const& unit_normal,
                float_32_bit  penetration_depth) -> bool {
                    if (is_last_micro_step && m_do_show_contact_normals)
                        m_cache_of_batches_of_colliders.collision_normals_points->push_back({
                                contact_point, contact_point + 0.25f * unit_normal
                                });

                    angeo::collision_object_id const  coid_1 = angeo::get_object_id(angeo::get_first_collider_id(cid));
                    auto const  rb_1_it = m_binding_of_collision_objects.find(coid_1);
                    if (rb_1_it == m_binding_of_collision_objects.cend())
                        return true;
                    angeo::collision_object_id const  coid_2 = angeo::get_object_id(angeo::get_second_collider_id(cid));
                    auto const  rb_2_it = m_binding_of_collision_objects.find(coid_2);
                    if (rb_2_it == m_binding_of_collision_objects.cend())
                        return true;
                    vector3  unit_tangent, unit_bitangent;
                    angeo::compute_tangent_space_of_unit_vector(unit_normal, unit_tangent, unit_bitangent);
                    angeo::rigid_body_simulator::contact_friction_constraints_info  friction_info {
                            { unit_tangent, unit_bitangent },
                            false,
                            0.001f
                    };
                    m_rigid_body_simulator.insert_contact_constraints(
                            rb_1_it->second,
                            rb_2_it->second,
                            cid,
                            contact_point,
                            unit_normal,
                            m_collision_scene.get_material(coid_1),
                            m_collision_scene.get_material(coid_2),
                            &friction_info,
                            penetration_depth,
                            20.0f,
                            nullptr
                            );

                    return true;
                },
            true
            );
    m_rigid_body_simulator.do_simulation_step(time_to_simulate_in_seconds, time_to_simulate_in_seconds);
    for (auto const&  rb_id_and_node_ptr : m_binding_of_rigid_bodies)
    {
        angeo::rigid_body_id const  rb_id = rb_id_and_node_ptr.first;
        if (m_static_rigid_body_backups.count(rb_id) != 0UL)
            continue; // The rigid body 'rb_id' is static => does not move => no need to update positions of its node and colliders.

        scn::scene_node_ptr const  rb_node_ptr = rb_id_and_node_ptr.second;

        m_scene_nodes_relocated_during_simulation.insert({ rb_node_ptr->get_id(), *rb_node_ptr->get_coord_system()});

        if (rb_node_ptr->has_parent())
        {
            matrix44  rb_world_matrix;
            compose_from_base_matrix(
                    m_rigid_body_simulator.get_position_of_mass_centre(rb_id),
                    quaternion_to_rotation_matrix(m_rigid_body_simulator.get_orientation(rb_id)),
                    rb_world_matrix
                    );
            matrix44 const  parent_to_base_matrix = inverse44(rb_node_ptr->get_parent()->get_world_matrix());
            matrix44 const  from_rb_parent_matrix = parent_to_base_matrix * rb_world_matrix;
            rb_node_ptr->relocate(
                    translation_vector(from_rb_parent_matrix),
                    rotation_matrix_to_quaternion(rotation_matrix(from_rb_parent_matrix))
                    );
        }
        else
            rb_node_ptr->relocate(
                    m_rigid_body_simulator.get_position_of_mass_centre(rb_id),
                    m_rigid_body_simulator.get_orientation(rb_id)
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

        constexpr float_32_bit  RANGE_FROM_CLOSEST_IN_METERS = 2.0f;

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
            std::pair<qtgl::batch, std::vector<std::pair<scn::scene_node_ptr, gfx_animated_object const*> > > >
        batches;
    get_scene().foreach_node(
        [this, &batches](scn::scene_node_ptr const  node_ptr) -> bool {
                for (auto const& name_holder : scn::get_batch_holders(*node_ptr))
                {
                    qtgl::batch const  batch = scn::as_batch(name_holder.second);
                    auto&  record = batches[batch.path_component_of_uid()];
                    if (record.first.empty())
                        record.first = batch;
                    INVARIANT(record.first == batch);
                    //auto const  it = m_gfx_animated_objects.find(scn::make_batch_record_id(node_ptr->get_id(), name_holder.first));
                    //INVARIANT(it == m_gfx_animated_objects.cend() || it->second.get_batch() == record.first);
                    record.second.push_back({
                        node_ptr,
                        nullptr //it != m_gfx_animated_objects.cend() ? &it->second : nullptr
                        });
                }
                return true;
            },
        false
        );
    for (auto const& elem : batches)
        if (qtgl::make_current(elem.second.first, draw_state))
        {
            skeleton  bones;
            if (elem.second.first.get_available_resources().skeletal() != nullptr)
            {
                TMPROF_BLOCK();

                boost::filesystem::path const  skeleton_directory =
                        boost::filesystem::path(elem.second.first.get_available_resources().data_root_dir())
                                / "animations"
                                / "skeletal"
                                / elem.second.first.get_available_resources().skeletal()->skeleton_name()
                                ;
                auto const  skeleton_iter = m_skeletons.find(skeleton_directory.string());
                bones = (skeleton_iter != m_skeletons.cend()) ?
                                skeleton_iter->second :
                                m_skeletons.insert({skeleton_directory.string(), skeleton(skeleton_directory)}).first->second;
            }
            for (auto const& node_and_anim : elem.second.second)
                if (!bones.loaded_successfully()) // || node_and_anim.second == nullptr)
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
                    //node_and_anim.second->get_transformations(
                    //        frame,
                    //        matrix_from_world_to_camera * node_and_anim.first->get_world_matrix()
                    //        );
                    std::vector<matrix44>  frame;
                    {
                        std::vector<scn::scene_node_id> const&  relative_node_ids = bones.get_relative_node_ids();
                        std::vector<angeo::coordinate_system> const&  coord_systems = elem.second.first.get_modelspace().get_coord_systems();
                        INVARIANT(relative_node_ids.size() == coord_systems.size());
                        for (natural_32_bit  bone = 0U; bone != relative_node_ids.size(); ++bone)
                            if (scn::scene_node_ptr const  bone_node_ptr = node_and_anim.first->find_child(relative_node_ids.at(bone)))
                                frame.push_back(matrix_from_world_to_camera * bone_node_ptr->get_world_matrix());
                            else
                            {
                                matrix44 M;
                                angeo::from_base_matrix(coord_systems.at(bone), M);
                                frame.push_back(matrix_from_world_to_camera * node_and_anim.first->get_world_matrix() * M);
                            }
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

    if (!qtgl::make_current(m_batch_coord_system, draw_state))
        return;

    //auto const  old_depth_test_state = qtgl::glapi().glIsEnabled(GL_DEPTH_TEST);
    //qtgl::glapi().glDisable(GL_DEPTH_TEST);

    std::unordered_set<scn::scene_node_id>  nodes_to_draw = m_scene_selection.get_nodes();
    scn::get_nodes_of_selected_records(m_scene_selection, nodes_to_draw);
    if (scn::has_node(get_scene(), scn::get_pivot_node_id())) // The pivot may be missing, if the scene is not completely initialised yet.
        nodes_to_draw.insert(scn::get_pivot_node_id());
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
                                        m_collision_scene.get_capsule_half_distance_between_end_points(coid),
                                        m_collision_scene.get_capsule_thickness_from_central_line(coid)
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
                                float_32_bit const  key = m_collision_scene.get_sphere_radius(coid);
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
                                    m_collision_scene.get_triangle_points_getter(coid)
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


void  simulator::erase_scene_node(scn::scene_node_id const&  id)
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene().get_scene_node(id);

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

void  simulator::invalidate_rigid_body_at_node(scn::scene_node_ptr  node_ptr, bool const  collider_change)
{
    TMPROF_BLOCK();

    auto const  invalidation_it = m_invalidated_nodes_of_rigid_bodies.find(node_ptr->get_id());
    if (invalidation_it != m_invalidated_nodes_of_rigid_bodies.end())
        invalidation_it->second = invalidation_it->second || collider_change;
    else
        m_invalidated_nodes_of_rigid_bodies.insert({ node_ptr->get_id(), collider_change });
}


void  simulator::invalidate_rigid_body_controling_node(scn::scene_node_ptr  node_ptr, bool const  collider_change)
{
    if (auto const  rb_node_ptr = find_nearest_rigid_body_node(node_ptr))
        invalidate_rigid_body_at_node(rb_node_ptr, collider_change);
}

void  simulator::invalidate_rigid_bodies_in_subtree(scn::scene_node_ptr  node_ptr, bool const  collider_change)
{
    TMPROF_BLOCK();

    foreach_rigid_body_in_subtree(
            node_ptr,
            [this, collider_change](scn::rigid_body&, scn::scene_node_ptr const  rb_node_ptr) {
                    invalidate_rigid_body_at_node(rb_node_ptr, collider_change);
                }
            );
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
                        m_collision_scene.on_position_changed(coid, node_ptr->get_world_matrix());
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
    angeo::collision_object_id const  collider_id =
            m_collision_scene.insert_sphere(radius, node_ptr->get_world_matrix(), material, as_dynamic);
    scn::insert_collider(*node_ptr, id.get_record_name(), collider_id, density_multiplier);
    invalidate_rigid_body_controling_node(node_ptr, true);
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
    angeo::collision_object_id const  collider_id =
            m_collision_scene.insert_capsule(
                    half_distance_between_end_points,
                    thickness_from_central_line,
                    node_ptr->get_world_matrix(),
                    material,
                    as_dynamic
                    );
    scn::insert_collider(*node_ptr, id.get_record_name(), collider_id, density_multiplier);
    invalidate_rigid_body_controling_node(node_ptr, true);
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
    std::vector<angeo::collision_object_id>  collider_ids;
    m_collision_scene.insert_triangle_mesh(
            index_buffer.num_primitives(),
            detail::collider_triangle_mesh_vertex_getter(vertex_buffer, index_buffer),
            node_ptr->get_world_matrix(),
            material,
            false,
            collider_ids
            );
    scn::insert_collider(*node_ptr, id.get_record_name(), collider_ids, density_multiplier);
    invalidate_rigid_body_controling_node(node_ptr, true);
}

void  simulator::erase_collision_object_from_scene_node(
        scn::scene_record_id const&  id
        )
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene_node(id.get_node_id());
    if (auto const  collider_ptr = scn::get_collider(*node_ptr))
    {
        for (auto  coid : collider_ptr->ids()) {
            m_collision_scene.erase_object(coid);
            m_binding_of_collision_objects.erase(coid);
        }
        m_scene_selection.erase_record(id);
        scn::erase_collider(*node_ptr);

        invalidate_rigid_body_controling_node(node_ptr, true);
    }
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
    radius = m_collision_scene.get_sphere_radius(collider->id());
    material = m_collision_scene.get_material(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene.is_dynamic(collider->id());
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
    half_distance_between_end_points = m_collision_scene.get_capsule_half_distance_between_end_points(collider->id());
    thickness_from_central_line = m_collision_scene.get_capsule_thickness_from_central_line(collider->id());
    material = m_collision_scene.get_material(collider->id());
    density_multiplier = collider->get_density_multiplier();
    is_dynamic = m_collision_scene.is_dynamic(collider->id());
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
            m_collision_scene.get_triangle_points_getter(collider->id()).target<detail::collider_triangle_mesh_vertex_getter>();
    vertex_buffer = vertices_getter_ptr->get_vertex_buffer();
    index_buffer = vertices_getter_ptr->get_index_buffer();
    material = m_collision_scene.get_material(collider->id());
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

    ASSUMPTION(find_nearest_rigid_body_node(node_ptr->get_parent()) == nullptr);

    angeo::rigid_body_id const  rb_id =
            m_rigid_body_simulator.insert_rigid_body(
                    vector3_zero(),
                    quaternion_identity(),
                    1.0f,
                    matrix33_identity(),
                    linear_velocity,
                    angular_velocity,
                    external_linear_acceleration,
                    external_angular_acceleration
                    );

    scn::insert_rigid_body(*node_ptr, rb_id);
    m_binding_of_rigid_bodies[rb_id] = node_ptr;

    invalidate_rigid_body_controling_node(node_ptr, true);
}

void  simulator::erase_rigid_body_from_scene_node(
        scn::scene_node_id const&  id
        )
{
    TMPROF_BLOCK();

    auto const  node_ptr = get_scene_node(id);
    if (auto const  rb_ptr = scn::get_rigid_body(*node_ptr))
    {
        std::vector<angeo::collision_object_id>  coids;
        collect_colliders_in_subtree(node_ptr, coids);
        for (angeo::collision_object_id coid : coids)
            m_binding_of_collision_objects.erase(coid);
        bool  has_static_collider = false;
        for (angeo::collision_object_id coid : coids)
            if (!m_collision_scene.is_dynamic(coid))
            {
                has_static_collider = true;
                break;
            }
        if (!has_static_collider)
            for (std::size_t i = 0U; i < coids.size(); ++i)
                for (std::size_t j = i + 1U; j < coids.size(); ++j)
                    m_collision_scene.enable_colliding(coids.at(i), coids.at(j));

        m_rigid_body_simulator.erase_rigid_body(rb_ptr->id());
        m_binding_of_rigid_bodies.erase(rb_ptr->id());
        m_static_rigid_body_backups.erase(rb_ptr->id());

        m_scene_selection.erase_record(scn::make_rigid_body_record_id(id));
        scn::erase_rigid_body(*node_ptr);

        invalidate_rigid_body_controling_node(node_ptr, true);
    }
}


void  simulator::get_rigid_body_info(
        scn::scene_node_id const&  id,
        vector3&  linear_velocity,
        vector3&  angular_velocity,
        vector3&  external_linear_acceleration,
        vector3&  external_angular_acceleration
        )
{
    scn::scene_node_const_ptr const  node_ptr = get_scene_node(id);
    scn::rigid_body const* const  rb_ptr = scn::get_rigid_body(*node_ptr);
    ASSUMPTION(rb_ptr != nullptr);
    linear_velocity = m_rigid_body_simulator.get_linear_velocity(rb_ptr->id());
    angular_velocity = m_rigid_body_simulator.get_angular_velocity(rb_ptr->id());
    external_linear_acceleration = m_rigid_body_simulator.get_external_linear_acceleration(rb_ptr->id());
    external_angular_acceleration = m_rigid_body_simulator.get_external_angular_acceleration(rb_ptr->id());
}


void  simulator::load_collider(boost::property_tree::ptree const&  data, scn::scene_node_id const&  id)
{
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
        qtgl::buffer  vertex_buffer(buffers_dir / "vertices.txt");
        qtgl::buffer  index_buffer(buffers_dir / "indices.txt");
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
    switch (angeo::get_shape_type(collider.id()))
    {
    case angeo::COLLISION_SHAPE_TYPE::CAPSULE:
        data.put("shape_type", "capsule");
        data.put("half_distance_between_end_points", m_collision_scene.get_capsule_half_distance_between_end_points(collider.id()));
        data.put("thickness_from_central_line", m_collision_scene.get_capsule_thickness_from_central_line(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::SPHERE:
        data.put("shape_type", "sphere");
        data.put("radius", m_collision_scene.get_sphere_radius(collider.id()));
        break;
    case angeo::COLLISION_SHAPE_TYPE::TRIANGLE:
        {
            data.put("shape_type", "triangle mesh");
            detail::collider_triangle_mesh_vertex_getter const* const  vertices_getter_ptr =
                m_collision_scene.get_triangle_points_getter(collider.id()).target<detail::collider_triangle_mesh_vertex_getter>();
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

    data.put("material", to_string(m_collision_scene.get_material(collider.id())));
    data.put("is_dynamic", m_collision_scene.is_dynamic(collider.id()));
    data.put("density_multiplier", collider.get_density_multiplier());
}


void  simulator::load_rigid_body(
        boost::property_tree::ptree const&  data,
        scn::scene_node_id const&  id
        )
{
    auto const  load_vector = [&data](std::string const&  key) -> vector3 {
        boost::property_tree::path const  key_path(key, '/');
        return vector3(data.get<float_32_bit>(key_path / "x"),
                       data.get<float_32_bit>(key_path / "y"),
                       data.get<float_32_bit>(key_path / "z"));
    };

    insert_rigid_body_to_scene_node(
            load_vector("linear_velocity"),
            load_vector("angular_velocity"),
            load_vector("external_linear_acceleration"),
            load_vector("external_angular_acceleration"),
            id
            );
}

void  simulator::save_rigid_body(angeo::rigid_body_id const  rb_id, boost::property_tree::ptree&  data)
{
    auto const  save_vector = [&data](std::string const&  key, vector3 const&  u) -> void {
        boost::property_tree::path const  key_path(key, '/');
        data.put(key_path / "x", u(0));
        data.put(key_path / "y", u(1));
        data.put(key_path / "z", u(2));
    };

    save_vector("linear_velocity", m_rigid_body_simulator.get_linear_velocity(rb_id));
    save_vector("angular_velocity", m_rigid_body_simulator.get_angular_velocity(rb_id));
    save_vector("external_linear_acceleration", m_rigid_body_simulator.get_external_linear_acceleration(rb_id));
    save_vector("external_angular_acceleration", m_rigid_body_simulator.get_external_angular_acceleration(rb_id));
}


void  simulator::clear_scene()
{
    m_scene_selection.clear();
    m_scene_edit_data.invalidate_data();
    m_invalidated_nodes_of_rigid_bodies.clear();
    m_cache_of_batches_of_colliders.capsules.clear();
    m_cache_of_batches_of_colliders.spheres.clear();
    m_cache_of_batches_of_colliders.triangle_meshes.clear();
    m_cache_of_batches_of_colliders.collision_normals_points.release();
    m_cache_of_batches_of_colliders.collision_normals_batch.release();
    m_skeletons.clear();

    get_scene().clear();

    m_collision_scene.clear();
    m_rigid_body_simulator.clear();
    m_binding_of_collision_objects.clear();
    m_binding_of_rigid_bodies.clear();
    m_gfx_animated_objects.clear();
}

void  simulator::translate_scene_node(scn::scene_node_id const&  id, vector3 const&  shift)
{
    auto const  node_ptr = get_scene_node(id);
    node_ptr->translate(shift);
    m_scene_edit_data.invalidate_data();
    on_relocation_of_scene_node(node_ptr);
}

void  simulator::rotate_scene_node(scn::scene_node_id const&  id, quaternion const&  rotation)
{
    auto const  node_ptr = get_scene_node(id);
    node_ptr->rotate(rotation);
    m_scene_edit_data.invalidate_data();
    on_relocation_of_scene_node(node_ptr);
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
    scn::scene_node_ptr const  phs_node_ptr = find_nearest_rigid_body_node(node_ptr);
    if (phs_node_ptr != nullptr)
    {
        if (phs_node_ptr == node_ptr)
            invalidate_rigid_body_at_node(phs_node_ptr, false);
        else
        {
            std::vector<angeo::collision_object_id>  coids;
            collect_colliders_in_subtree(node_ptr, coids);
            if (!coids.empty())
                invalidate_rigid_body_at_node(phs_node_ptr, true);
        }
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
