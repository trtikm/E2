#ifndef E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_node_id.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_selection.hpp>
#   include <scene/scene_history.hpp>
#   include <scene/scene_editing.hpp>
#   include <scene/records/collider/collider.hpp>
#   include <scene/records/rigid_body/rigid_body.hpp>
#   include <scene/records/agent/agent.hpp>
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <qtgl/batch_generators.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <ai/agents.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>
#   include <string>
#   include <list>


enum  CAMERA_CONTROLLER_TYPE
{
    CAMERA_CONTROLLER_FREE_FLY              = 0,
    CAMERA_CONTROLLER_ORBIT                 = 1,
    CAMERA_CONTROLLER_FOLLOW                = 2,
    CAMERA_CONTROLLER_LOOK_AT               = 3,
    CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT    = 4,

    NUM_CAMERA_CONTROLLER_TYPES
};


struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();

    void synchronise_with_dependent_modules();

    // Simulation

    void next_round(
            float_64_bit  seconds_from_previous_call,
            bool const  is_this_pure_redraw_request
            ) override;

    bool  paused() const { return m_paused; }

    // Background and grid

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }
    void  set_show_grid_state(bool const  state) { m_do_show_grid = state; }
    void  set_show_batches(bool const  state) { m_do_show_batches = state; }
    void  set_show_colliders(bool const  state) { m_do_show_colliders = state; }
    void  set_show_contact_normals(bool const  state) { m_do_show_contact_normals = state; }
    void  set_show_ai_action_controller_props(bool const  state) { m_do_show_ai_action_controller_props = state; }
    void  set_colliders_color(vector3 const&  colour) { m_colliders_colour = expand34(colour); }
    void  set_render_in_wireframe(bool const  state) { m_render_in_wireframe = state; }

    // Camera

    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }
    float_32_bit  get_camera_near_plane_distance() const { return m_camera->near_plane(); }
    float_32_bit  get_camera_far_plane_distance() const { return m_camera->far_plane(); }
    float_32_bit  get_camera_side_plane_minimal_distance() const;
    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }
    void  set_camera_far_plane(float_32_bit const  far_plane) { m_camera->set_far_plane(far_plane); }
    void  set_camera_speed(float_32_bit const  speed);

    // Scene

    scn::scene const&  get_scene() const { return *m_scene; }
    scn::scene&  get_scene() { return *m_scene; }

    scn::scene_node_ptr  get_scene_node(scn::scene_node_id const&  id) const
    { return get_scene().get_scene_node(id); }

    scn::scene_node_ptr  insert_scene_node(scn::scene_node_id const&  id)
    { return insert_scene_node_at(id, vector3_zero(), quaternion_identity()); }

    scn::scene_node_ptr  insert_scene_node_at(scn::scene_node_id const&  id, vector3 const&  origin, quaternion const&  orientation)
    { return get_scene().insert_scene_node(id, origin, orientation); }

    void  erase_scene_node(scn::scene_node_id const&  id);

    void  insert_batch_to_scene_node(
            scn::scene_node::record_name const&  batch_name,
            boost::filesystem::path const&  batch_pathname,
            scn::scene_node_id const&  scene_node_id
            );

    void  erase_batch_from_scene_node(
            scn::scene_node::record_name const&  batch_name,
            scn::scene_node_id const&  scene_node_id
            );

    void  insert_collision_sphere_to_scene_node(
            float_32_bit const  radius,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic,
            scn::scene_record_id const&  id
            );

    void  insert_collision_capsule_to_scene_node(
            float_32_bit const  half_distance_between_end_points,
            float_32_bit const  thickness_from_central_line,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier,
            bool const  as_dynamic,
            scn::scene_record_id const&  id
            );

    void  insert_collision_trianle_mesh_to_scene_node(
            qtgl::buffer const  vertex_buffer,
            qtgl::buffer const  index_buffer,
            angeo::COLLISION_MATERIAL_TYPE const  material,
            float_32_bit const  density_multiplier,
            scn::scene_record_id const&  id
            );

    void  erase_collision_object_from_scene_node(
            scn::scene_record_id const&  id
            );

    void  get_collision_sphere_info(
            scn::scene_record_id const&  id,
            float_32_bit&  radius,
            angeo::COLLISION_MATERIAL_TYPE&  material,
            float_32_bit&  density_multiplier,
            bool&  is_dynamic
            );

    void  get_collision_capsule_info(
            scn::scene_record_id const&  id,
            float_32_bit&  half_distance_between_end_points,
            float_32_bit&  thickness_from_central_line,
            angeo::COLLISION_MATERIAL_TYPE&  material,
            float_32_bit&  density_multiplier,
            bool&  is_dynamic
            );

    void  get_collision_triangle_mesh_info(
            scn::scene_record_id const&  id,
            qtgl::buffer&  vertex_buffer,
            qtgl::buffer&  index_buffer,
            angeo::COLLISION_MATERIAL_TYPE&  material,
            float_32_bit&  density_multiplier
            );

    void  insert_rigid_body_to_scene_node(
            vector3 const&  linear_velocity,
            vector3 const&  angular_velocity,
            vector3 const&  external_linear_acceleration,
            vector3 const&  external_angular_acceleration,
            scn::scene_node_id const&  id
            );

    void  insert_rigid_body_to_scene_node_ex(
            vector3 const&  linear_velocity,
            vector3 const&  angular_velocity,
            vector3 const&  external_linear_acceleration,
            vector3 const&  external_angular_acceleration,
            float_32_bit const  mass_inverted,
            matrix33 const&  inertia_tensor_inverted,
            scn::scene_node_id const&  id
            );

    void  erase_rigid_body_from_scene_node(
            scn::scene_node_id const&  id
            );

    void  rebuild_rigid_body_due_to_change_in_subtree(scn::scene_node_ptr const  phs_node_ptr);

    void  get_rigid_body_info(
            scn::scene_node_id const&  id,
            bool&  auto_compute_mass_and_inertia_tensor,
            scn::rigid_body_props&  props
            );

    void  insert_agent(scn::scene_record_id const&  id, scn::skeleton_props_const_ptr const  props);
    void  erase_agent(scn::scene_record_id const&  id);

    void  load_collider(boost::property_tree::ptree const&  data, scn::scene_node_id const&  id);
    void  save_collider(scn::collider const&  collider, boost::property_tree::ptree&  data);

    void  load_rigid_body(boost::property_tree::ptree const&  data, scn::scene_node_id const&  id);
    void  save_rigid_body(scn::scene_node_id const&  id, boost::property_tree::ptree&  data);

    void  load_agent(boost::property_tree::ptree const&  data, scn::scene_record_id const&  id);
    void  save_agent(scn::scene_node_ptr const  node_ptr, boost::property_tree::ptree&  data);

    scn::skeleton_props_const_ptr  get_agent_info(scn::scene_node_id const& id);

    void  clear_scene();

    scn::scene_history_ptr  get_scene_history() { return m_scene_history; }
    std::shared_ptr<angeo::collision_scene>  get_collision_scene() const { return m_collision_scene_ptr; }
    std::shared_ptr<angeo::rigid_body_simulator>  get_rigid_body_simulator() const { return m_rigid_body_simulator_ptr; }
    std::shared_ptr<ai::agents>  get_agents() const { return m_agents_ptr; }

    void  set_position_of_scene_node(scn::scene_node_id const&  id, vector3 const&  new_origin);
    void  set_orientation_of_scene_node(scn::scene_node_id const&  id, quaternion const&  new_orientation);
    void  relocate_scene_node(scn::scene_node_id const&  id, vector3 const&  new_origin, quaternion const&  new_orientation);

    void  on_relocation_of_scene_node(scn::scene_node_ptr const  node_ptr);
    void  on_relocation_of_scene_node(scn::scene_node_id const&  id)
    { on_relocation_of_scene_node(get_scene().get_scene_node(id)); }

    void  set_scene_selection(
            std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  insert_to_scene_selection(
            std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  erase_from_scene_selection(
            std::unordered_set<scn::scene_node_id> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  get_scene_selection(
            std::unordered_set<scn::scene_node_id>&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id>&  selected_records
            ) const;

    scn::SCENE_EDIT_MODE  get_scene_edit_mode() const { return m_scene_edit_data.get_mode(); }
    void  set_scene_edit_mode(scn::SCENE_EDIT_MODE const  edit_mode);

    scn::scene_edit_data const&  get_scene_edit_data() const { return m_scene_edit_data; }

    qtgl::effects_config const&  get_effects_config() const { return m_effects_config; }
    qtgl::effects_config&  effects_config_ref() { return m_effects_config; }

    scn::scene_node_id const&  get_scene_node_of_agent(ai::agent_id const  id) const { return m_binding_of_agents_to_scene.at(id); }

    scn::scene_node_ptr  find_nearest_rigid_body_node(scn::scene_node_ptr  node_ptr);
    scn::scene_node_ptr  find_nearest_rigid_body_node(scn::scene_node_id const&  id)
    { return find_nearest_rigid_body_node(get_scene().get_scene_node(id)); }
    void  find_nearest_rigid_body_nodes_in_subtree(scn::scene_node_ptr  node_ptr, std::vector<scn::scene_node_ptr>&  output);

    void  on_simulation_paused();
    void  on_simulation_resumed();

    CAMERA_CONTROLLER_TYPE  get_camera_controller_type() const
    {
        return paused() ? m_camera_controller_type_in_edit_mode :
                          get_scene_node(m_camera_target_node_id) != nullptr ? m_camera_controller_type_in_simulation_mode :
                                                                               CAMERA_CONTROLLER_FREE_FLY;
    }
    CAMERA_CONTROLLER_TYPE  get_camera_controller_type_in_simulation_mode() const { return m_camera_controller_type_in_simulation_mode; }
    void  set_camera_controller_type_in_simulation_mode(CAMERA_CONTROLLER_TYPE const  type) { m_camera_controller_type_in_simulation_mode = type; }
    scn::scene_node_id const& get_camera_target_node_id() const { return m_camera_target_node_id; }
    void  set_camera_target_node_id(scn::scene_node_id const&  id) { m_camera_target_node_id = id; m_camera_target_vector_invalidated = true; }

private:

    void  perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds);
    void  perform_simulation_micro_step(float_64_bit const  time_to_simulate_in_seconds, bool const  is_last_micro_step);
    void  render_simulation_state(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );

    void  perform_scene_update(float_64_bit const  time_to_simulate_in_seconds);
    void  select_scene_objects(float_64_bit const  time_to_simulate_in_seconds);
    void  translate_scene_selected_objects(float_64_bit const  time_to_simulate_in_seconds);
    void  rotate_scene_selected_objects(float_64_bit const  time_to_simulate_in_seconds);

    void  render_scene_batches(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );
    void  render_scene_coord_systems(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );
    void  render_colliders(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );
    void  render_contact_normals(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );
    void  render_ai_action_controller_props(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );


    void  foreach_collider_in_subtree(
                scn::scene_node_ptr const  node_ptr,
                std::function<void(scn::collider&,scn::scene_node_ptr)> const&  action
                );
    void  foreach_rigid_body_in_subtree(
                scn::scene_node_ptr const  node_ptr,
                std::function<void(scn::rigid_body&,scn::scene_node_ptr)> const&  action
                );

    void  collect_colliders_in_subtree(
                scn::scene_node_ptr const  node_ptr,
                std::vector<angeo::collision_object_id>&  output,
                std::vector<scn::scene_node_ptr>* const  output_nodes = nullptr
                );
    void  update_collider_locations_in_subtree(scn::scene_node_ptr  node_ptr);
    void  update_collider_locations_in_subtree(scn::scene_node_id const&  id)
    { update_collider_locations_in_subtree(get_scene().get_scene_node(id)); }

    // Data providing feedback loop between a human user and 3D scene in the tool

    qtgl::camera_perspective_ptr  m_camera;
    CAMERA_CONTROLLER_TYPE  m_camera_controller_type_in_edit_mode;
    CAMERA_CONTROLLER_TYPE  m_camera_controller_type_in_simulation_mode;
    qtgl::free_fly_config  m_free_fly_config;
    qtgl::free_fly_config  m_orbit_config;
    qtgl::free_fly_config  m_only_move_camera_config;
    scn::scene_node_id  m_camera_target_node_id;
    vector3  m_camera_target_vector_in_camera_space;
    bool  m_camera_target_vector_invalidated;

    qtgl::effects_config  m_effects_config;
    vector4  m_diffuse_colour;
    vector3  m_ambient_colour;
    vector4  m_specular_colour;
    vector3  m_directional_light_direction;
    vector3  m_directional_light_colour;
    vector4  m_fog_colour;
    float  m_fog_near;
    float  m_fog_far;
    qtgl::batch  m_batch_grid;
    bool  m_do_show_grid;
    bool  m_do_show_batches;
    bool  m_do_show_colliders;
    bool  m_do_show_contact_normals;
    bool  m_do_show_ai_action_controller_props;
    vector4  m_colliders_colour;
    bool  m_render_in_wireframe;

    // Common and shared data for both modes: Editing and Simulation

    bool  m_paused;
    bool  m_do_single_step;
    float_64_bit  m_fixed_time_step_in_seconds;
    scn::scene_ptr  m_scene;

    struct  cache_of_batches_of_colliders
    {
        std::unordered_map<float_32_bit, qtgl::batch>  spheres;
        std::unordered_map<std::pair<float_32_bit, float_32_bit>, qtgl::batch>  capsules;
        std::unordered_map<std::string, qtgl::batch>  triangle_meshes;

        std::unique_ptr<std::vector< std::pair<vector3, vector3> > >  collision_normals_points;
        qtgl::batch  collision_normals_batch;
    };
    cache_of_batches_of_colliders  m_cache_of_batches_of_colliders;

    struct  cache_of_batches_of_ai_agents
    {
        std::unique_ptr<std::pair<std::vector< std::pair<vector3, vector3> >, std::vector<vector4> > >  lines;
        qtgl::batch  lines_batch;
    };
    cache_of_batches_of_ai_agents  m_cache_of_batches_of_ai_agents;

    qtgl::font_mono_props  m_font_props;

    // Editing mode data

    scn::scene_selection  m_scene_selection;
    scn::scene_history_ptr  m_scene_history;
    scn::scene_edit_data  m_scene_edit_data;
    qtgl::batch  m_batch_coord_system;

    // Simulation mode data

    std::shared_ptr<angeo::collision_scene>  m_collision_scene_ptr;
    std::shared_ptr<angeo::rigid_body_simulator>  m_rigid_body_simulator_ptr;
    std::shared_ptr<ai::agents>  m_agents_ptr;

    std::unordered_map<angeo::collision_object_id, angeo::rigid_body_id>  m_binding_of_collision_objects;
    std::unordered_map<angeo::rigid_body_id, scn::scene_node_ptr>  m_binding_of_rigid_bodies;
    std::unordered_map<ai::agent_id, scn::scene_node_id>  m_binding_of_agents_to_scene;
};


#endif
