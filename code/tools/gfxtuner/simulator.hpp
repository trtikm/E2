#ifndef E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_SIMULATOR_HPP_INCLUDED

#   include <scene/scene.hpp>
#   include <scene/scene_selection.hpp>
#   include <scene/scene_history.hpp>
#   include <scene/scene_edit_utils.hpp>
#   include <gfxtuner/gfx_object.hpp>
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <angeo/tensor_math.hpp>
#   include <angeo/collision_scene.hpp>
#   include <utility/std_pair_hash.hpp>
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

    scn::scene_node_ptr  get_scene_node(std::string const&  name) const
    { return get_scene().get_scene_node(name); }

    scn::scene_node_ptr  insert_scene_node(std::string const&  name)
    { return insert_scene_node_at(name, vector3_zero(), quaternion_identity()); }

    scn::scene_node_ptr  insert_scene_node_at(std::string const&  name, vector3 const&  origin, quaternion const&  orientation)
    { return get_scene().insert_scene_node(name, origin, orientation, nullptr); }

    scn::scene_node_ptr  insert_child_scene_node(std::string const&  name, std::string const&  parent_name)
    { return insert_child_scene_node_at(name, vector3_zero(), quaternion_identity(), parent_name); }

    scn::scene_node_ptr  insert_child_scene_node_at(
            std::string const&  name,
            vector3 const&  origin,
            quaternion const&  orientation,
            std::string const&  parent_name
            )
    { return get_scene().insert_scene_node(name, origin, orientation, get_scene_node(parent_name)); }

    void  erase_scene_node(std::string const&  name);

    void  insert_batch_to_scene_node(std::string const&  batch_name, boost::filesystem::path const&  batch_pathname, std::string const&  scene_node_name);

    void  erase_batch_from_scene_node(std::string const&  batch_name, std::string const&  scene_node_name);

    void  clear_scene();

    scn::scene_history_ptr  get_scene_history() { return m_scene_history; }

    void  translate_scene_node(std::string const&  scene_node_name, vector3 const&  shift);
    void  rotate_scene_node(std::string const&  scene_node_name, quaternion const&  rotation);
    void  set_position_of_scene_node(std::string const&  scene_node_name, vector3 const&  new_origin);
    void  set_orientation_of_scene_node(std::string const&  scene_node_name, quaternion const&  new_orientation);
    void  relocate_scene_node(std::string const&  scene_node_name, vector3 const&  new_origin, quaternion const&  new_orientation);

    void  set_scene_selection(
            std::unordered_set<std::string> const&  selected_scene_nodes,
            std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
            );
    void  insert_to_scene_selection(
            std::unordered_set<std::string> const&  selected_scene_nodes,
            std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
            );
    void  erase_from_scene_selection(
            std::unordered_set<std::string> const&  selected_scene_nodes,
            std::unordered_set<std::pair<std::string, std::string> > const&  selected_batches
            );
    void  get_scene_selection(
        std::unordered_set<std::string>&  selected_scene_nodes,
        std::unordered_set<std::pair<std::string, std::string> >&  selected_batches
        ) const;

    scn::SCENE_EDIT_MODE  get_scene_edit_mode() const { return m_scene_edit_data.get_mode(); }
    void  set_scene_edit_mode(scn::SCENE_EDIT_MODE const  edit_mode);

    scn::scene_edit_data const&  get_scene_edit_data() const { return m_scene_edit_data; }

    qtgl::effects_config const&  get_effects_config() const { return m_effects_config; }
    qtgl::effects_config&  effects_config_ref() { return m_effects_config; }

private:

    // Simulation

    void  validate_simulation_state();
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
    void  rotate_scene_node(std::string const&  scene_node_name, float_64_bit const  time_to_simulate_in_seconds);

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

    // Utility functions

    /// Data providing feedback loop between a human user and 3D scene in the tool
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

    /// Data related to simulation
    bool  m_paused;
    bool  m_do_single_step;

    /// Scene related data
    scn::scene_ptr  m_scene;
    scn::scene_selection  m_scene_selection;
    scn::scene_history_ptr  m_scene_history;
    qtgl::batch  m_batch_coord_system;
    scn::scene_edit_data  m_scene_edit_data;

    /// Simulation related data
    std::unordered_map<std::pair<std::string, std::string>, gfx_animated_object>  m_gfx_animated_objects;
    angeo::collision_scene  m_collision_scene;

    /// Other data
};


#endif
