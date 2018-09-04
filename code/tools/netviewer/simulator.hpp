#ifndef E2_TOOL_NETVIEWER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <netlab/network.hpp>
#   include <string>
#   include <memory>

#   include <netviewer/dbg/dbg_network_camera.hpp>
#   include <netviewer/dbg/dbg_frustum_sector_enumeration.hpp>
#   include <netviewer/dbg/dbg_raycast_sector_enumeration.hpp>
#   include <netviewer/dbg/dbg_draw_movement_areas.hpp>


struct simulator : public qtgl::real_time_simulator
{
    simulator(
            vector3 const&  initial_clear_colour,
            bool const  paused,
            float_64_bit const  desired_network_to_real_time_ratio
            );
    ~simulator();

    void next_round(
            float_64_bit const  seconds_from_previous_call,
            bool const  is_this_pure_redraw_request
            );

    /// Network independed methods.
    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }
    void  set_show_grid_state(bool const  state) { m_do_show_grid = state; }

    /// Scene camera related methods.
    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }
    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }
    void  set_camera_far_plane(float_32_bit const  far_plane) { m_camera->set_far_plane(far_plane); }
    void  set_camera_speed(float_32_bit const  speed);
    void  on_look_at_selected();

    /// Network management methods.
    std::string const&  get_experiment_name() const;
    std::shared_ptr<netlab::network>  network() const noexcept { return m_network; }
    bool  has_network() const { return network().operator bool(); }
    natural_64_bit  get_network_update_id() const;
    void  initiate_network_construction(std::string const&  experiment_name, bool const  pause_applies_to_network_open);
    bool  is_network_being_constructed() const;
    std::string  get_constructed_network_progress_text() const;
    natural_64_bit  get_num_network_construction_steps() const;
    void  acquire_constructed_network();
    void  enable_usage_of_queues_in_update_of_ships_in_network(bool const  enable_state);
    void  destroy_network();

    /// Network simulation dependent methods.
    bool  paused() const noexcept { return m_paused; }
    float_64_bit  spent_real_time() const noexcept { return m_spent_real_time; }
    float_64_bit  spent_network_time() const { return m_spent_network_time; }
    natural_64_bit  num_network_updates() const noexcept { return m_num_network_updates; }
    float_64_bit  desired_network_to_real_time_ratio() const { return m_desired_network_to_real_time_ratio; }
    void set_desired_network_to_real_time_ratio(float_64_bit const  value);

    /// Miscelanous methods
    std::shared_ptr<netlab::tracked_network_object_stats>  get_selected_object_stats() const { return m_selected_object_stats; }
    std::string  get_network_info_text() const;
    std::string  get_selected_info_text() const;
    std::string  get_network_performance_text() const;
    void  on_select_owner_spiker();
    bool  renders_only_chosen_layer() const noexcept { return m_render_only_chosen_layer; }
    void  enable_rendering_of_only_chosen_layer() { m_render_only_chosen_layer = true; }
    void  disable_rendering_of_only_chosen_layer() { m_render_only_chosen_layer = false; }
    netlab::layer_index_type  get_layer_index_of_chosen_layer_to_render() const noexcept { return m_layer_index_of_chosen_layer_to_render; }
    bool  set_layer_index_of_chosen_layer_to_render(netlab::layer_index_type const  layer_index);

    qtgl::effects_config const&  get_effects_config() const { return m_effects_config; }
    qtgl::effects_config&  effects_config_ref() { return m_effects_config; }

    /// Debugging stuff
    void  dbg_set_camera_far_plane(float_32_bit const  far_plane)
    {
        m_dbg_network_camera.set_far_plane(far_plane);
        m_dbg_frustum_sector_enumeration.invalidate();
        m_dbg_draw_movement_areas.invalidate();
    }
    void  dbg_set_camera_sync_state(bool const  sync)
    {
        if (sync) m_dbg_network_camera.disable(); else m_dbg_network_camera.enable(m_camera);
        m_dbg_frustum_sector_enumeration.invalidate();
        m_dbg_draw_movement_areas.invalidate();
    }
    void  dbg_enable_frustum_sector_enumeration(bool const  state)
    { if (state) m_dbg_frustum_sector_enumeration.enable(); else m_dbg_frustum_sector_enumeration.disable(); }
    void  dbg_enable_raycast_sector_enumeration(bool const  state)
    { if (state) m_dbg_raycast_sector_enumeration.enable(); else m_dbg_raycast_sector_enumeration.disable(); }
    void  dbg_enable_draw_movement_areas(bool const  state)
    { if (state) m_dbg_draw_movement_areas.enable(); else m_dbg_draw_movement_areas.disable(); }

private:

    void  update_network(float_64_bit const  seconds_from_previous_call);
    void  update_selection_of_network_objects(float_64_bit const  seconds_from_previous_call);

    void  render_network(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state  draw_state
            );
    void  render_network_spikers(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state&  draw_state
            );
    void  render_network_docks(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state&  draw_state
            );
    void  render_network_ships(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state&  draw_state
            );
    void  render_selected_network_object(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state&  draw_state
            );

    void  render_constructed_network(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            qtgl::draw_state  draw_state
            );
    void  render_spikers_of_constructed_network(
            matrix44 const&  matrix_from_world_to_camera,
            matrix44 const&  matrix_from_camera_to_clipspace,
            std::vector< std::pair<vector3,vector3> > const&  clip_planes,
            qtgl::draw_state&  draw_state
            );


    /// Network independed data providing feedback loop between a human user and 3D scene in the tool
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;
    qtgl::batch  m_batch_grid;
    bool  m_do_show_grid;

    /// THE NETWORK!
    std::shared_ptr<netlab::network>  m_network;
    std::string  m_experiment_name;

    /// Data related to updating of the network
    bool  m_paused;
    bool  m_do_single_step;
    float_64_bit  m_spent_real_time;
    float_64_bit  m_spent_network_time;
    natural_64_bit  m_num_network_updates;
    float_64_bit  m_desired_network_to_real_time_ratio;

    std::shared_ptr<netlab::tracked_network_object_stats>  m_selected_object_stats;

    /// Data for rendering of entities in the network and selection related stuff
    qtgl::effects_config  m_effects_config;

    qtgl::batch  m_batch_spiker;
    qtgl::batch  m_batch_dock;
    qtgl::batch  m_batch_ship;

    qtgl::batch  m_batch_spiker_bbox;
    qtgl::batch  m_batch_dock_bbox;
    qtgl::batch  m_batch_ship_bbox;

    qtgl::batch  m_batch_spiker_bsphere;
    qtgl::batch  m_batch_dock_bsphere;
    qtgl::batch  m_batch_ship_bsphere;

    std::vector<qtgl::batch>  m_batches_selection;

    bool m_render_only_chosen_layer;
    netlab::layer_index_type  m_layer_index_of_chosen_layer_to_render;

    /// Debugging stuff
    dbg_network_camera  m_dbg_network_camera;
    dbg_frustum_sector_enumeration  m_dbg_frustum_sector_enumeration;
    dbg_raycast_sector_enumeration  m_dbg_raycast_sector_enumeration;
    dbg_draw_movement_areas  m_dbg_draw_movement_areas;
};


#endif
