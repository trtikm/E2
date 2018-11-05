#ifndef E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_selection.hpp>
#   include <scene/scene_history.hpp>
#   include <scene/scene_editing.hpp>
#   include <scene/records/collider/collider.hpp>
#   include <scene/records/rigid_body/rigid_body.hpp>
#   include <gfxtuner/simulation/gfx_object.hpp>
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_scene.hpp>
#   include <angeo/rigid_body_simulator.hpp>
#   include <utility/std_pair_hash.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <vector>
#   include <unordered_map>
#   include <unordered_set>
#   include <string>
#   include <list>

struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();

    // Simulation

    void next_round(
            float_64_bit const  seconds_from_previous_call,
            bool const  is_this_pure_redraw_request
            ) override;

    bool  paused() const { return m_paused; }

    // Background and grid

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }
    void  set_show_grid_state(bool const  state) { m_do_show_grid = state; }

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

    scn::scene_node_ptr  get_scene_node(scn::scene_node_name const&  name) const
    { return get_scene().get_scene_node(name); }

    scn::scene_node_ptr  insert_scene_node(scn::scene_node_name const&  name)
    { return insert_scene_node_at(name, vector3_zero(), quaternion_identity()); }

    scn::scene_node_ptr  insert_scene_node_at(scn::scene_node_name const&  name, vector3 const&  origin, quaternion const&  orientation)
    { return get_scene().insert_scene_node(name, origin, orientation, nullptr); }

    scn::scene_node_ptr  insert_child_scene_node(scn::scene_node_name const&  name, scn::scene_node_name const&  parent_name)
    { return insert_child_scene_node_at(name, vector3_zero(), quaternion_identity(), parent_name); }

    scn::scene_node_ptr  insert_child_scene_node_at(
            scn::scene_node_name const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            std::string const&  parent_name
            )
    { return get_scene().insert_scene_node(name, origin, orientation, get_scene_node(parent_name)); }

    void  erase_scene_node(scn::scene_node_name const&  name);

    void  insert_batch_to_scene_node(
            scn::scene_node::record_name const&  batch_name,
            boost::filesystem::path const&  batch_pathname,
            scn::scene_node_name const&  scene_node_name
            );

    void  erase_batch_from_scene_node(
            scn::scene_node::record_name const&  batch_name,
            scn::scene_node_name const&  scene_node_name
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
            qtgl::buffer const  material_buffer,
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

    void  insert_rigid_body_to_scene_node(
            vector3 const&  linear_velocity,
            vector3 const&  angular_velocity,
            vector3 const&  external_linear_acceleration,
            vector3 const&  external_angular_acceleration,
            scn::scene_node_name const&  scene_node_name
            );

    void  erase_rigid_body_from_scene_node(
            scn::scene_node_name const&  scene_node_name
            );

    void  get_rigid_body_info(
            scn::scene_node_name const&  scene_node_name,
            vector3&  linear_velocity,
            vector3&  angular_velocity,
            vector3&  external_linear_acceleration,
            vector3&  external_angular_acceleration
            );

    void  load_collider(boost::property_tree::ptree const&  data, scn::scene_node_name const&  scene_node_name);
    void  save_collider(scn::collider const&  collider, boost::property_tree::ptree&  data);

    void  load_rigid_body(boost::property_tree::ptree const&  data, scn::scene_node_name const&  scene_node_name);
    void  save_rigid_body(angeo::rigid_body_id const  rb_id, boost::property_tree::ptree&  data);

    void  clear_scene();

    scn::scene_history_ptr  get_scene_history() { return m_scene_history; }

    void  translate_scene_node(scn::scene_node_name const&  scene_node_name, vector3 const&  shift);
    void  rotate_scene_node(scn::scene_node_name const&  scene_node_name, quaternion const&  rotation);
    void  set_position_of_scene_node(scn::scene_node_name const&  scene_node_name, vector3 const&  new_origin);
    void  set_orientation_of_scene_node(scn::scene_node_name const&  scene_node_name, quaternion const&  new_orientation);
    void  relocate_scene_node(scn::scene_node_name const&  scene_node_name, vector3 const&  new_origin, quaternion const&  new_orientation);

    void  on_relocation_of_scene_node(scn::scene_node_ptr const  node_ptr);
    void  on_relocation_of_scene_node(scn::scene_node_name const&  scene_node_name)
    { on_relocation_of_scene_node(get_scene().get_scene_node(scene_node_name)); }

    void  set_scene_selection(
            std::unordered_set<scn::scene_node_name> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  insert_to_scene_selection(
            std::unordered_set<scn::scene_node_name> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  erase_from_scene_selection(
            std::unordered_set<scn::scene_node_name> const&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id> const&  selected_records
            );
    void  get_scene_selection(
            std::unordered_set<scn::scene_node_name>&  selected_scene_nodes,
            std::unordered_set<scn::scene_record_id>&  selected_records
            ) const;

    scn::SCENE_EDIT_MODE  get_scene_edit_mode() const { return m_scene_edit_data.get_mode(); }
    void  set_scene_edit_mode(scn::SCENE_EDIT_MODE const  edit_mode);

    scn::scene_edit_data const&  get_scene_edit_data() const { return m_scene_edit_data; }

    qtgl::effects_config const&  get_effects_config() const { return m_effects_config; }
    qtgl::effects_config&  effects_config_ref() { return m_effects_config; }

    void  on_simulation_paused();
    void  on_simulation_resumed();

private:

    void  perform_simulation_step(float_64_bit const  time_to_simulate_in_seconds);
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

    scn::scene_node_ptr  find_nearest_rigid_body_node(scn::scene_node_ptr  node_ptr);
    scn::scene_node_ptr  find_nearest_rigid_body_node(scn::scene_node_name const&  node_name)
    { return find_nearest_rigid_body_node(get_scene().get_scene_node(node_name)); }

    void  invalidate_rigid_body_at_node(scn::scene_node_ptr  node_ptr, bool const  collider_change);
    void  invalidate_rigid_body_controling_node(scn::scene_node_ptr  node_ptr, bool const  collider_change);
    void  invalidate_rigid_body_controling_node(scn::scene_node_name const&  node_name, bool const  collider_change)
    { return invalidate_rigid_body_controling_node(get_scene().get_scene_node(node_name), collider_change); }
    void  invalidate_rigid_bodies_in_subtree(scn::scene_node_ptr  node_ptr, bool const  collider_change);
    void  invalidate_rigid_bodies_in_subtree(scn::scene_node_name const&  node_name, bool const  collider_change)
    { return invalidate_rigid_bodies_in_subtree(get_scene().get_scene_node(node_name), collider_change); }

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
    void  update_collider_locations_in_subtree(scn::scene_node_name const&  node_name)
    { update_collider_locations_in_subtree(get_scene().get_scene_node(node_name)); }

    // Data providing feedback loop between a human user and 3D scene in the tool

    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;
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

    // Common and shared data for both modes: Editing and Simulation

    bool  m_paused;
    bool  m_do_single_step;
    scn::scene_ptr  m_scene;
    std::unordered_set<scn::scene_node_name>  m_scene_nodes_relocated_during_simulation;
    std::unordered_set<scn::scene_record_id>  m_scene_records_inserted_during_simulation;
    std::unordered_set<scn::scene_record_id>  m_scene_records_erased_during_simulation;

    // Editing mode data

    scn::scene_selection  m_scene_selection;
    scn::scene_history_ptr  m_scene_history;
    scn::scene_edit_data  m_scene_edit_data;
    qtgl::batch  m_batch_coord_system;
    std::unordered_map<scn::scene_node_name, bool>  m_invalidated_nodes_of_rigid_bodies;

    // Simulation mode data

    angeo::collision_scene  m_collision_scene;
    angeo::rigid_body_simulator  m_rigid_body_simulator;
    std::unordered_map<angeo::collision_object_id, angeo::rigid_body_id>  m_binding_of_collision_objects;
    std::unordered_map<angeo::rigid_body_id, scn::scene_node_ptr>  m_binding_of_rigid_bodies;

    struct  static_rigid_body_backup
    {
        vector3  m_linear_velocity;
        vector3  m_angular_velocity;
        vector3  m_external_linear_acceleration;
        vector3  m_external_angular_acceleration;
    };
    std::unordered_map<angeo::rigid_body_id, static_rigid_body_backup>  m_static_rigid_body_backups;

    // TODO: The member below should be removed at some point.
    std::unordered_map<scn::scene_record_id, gfx_animated_object>  m_gfx_animated_objects;
};


#endif
