#ifndef E2_TOOL_NETVIEWER_SIMULATOR_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <netlab/network.hpp>
#   include <string>
#   include <memory>


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
    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }
    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }
    bool  is_camera_network_synchronised() const { return m_camera_network_coord_system == m_camera->coordinate_system(); }
    void  set_camera_far_plane(float_32_bit const  far_plane) { m_camera->set_far_plane(far_plane); }
    void  set_camera_network_far_plane(float_32_bit const  far_plane);
    void  set_camera_network_sync_state(bool const  synchronise);

    /// Network management methods.
    std::shared_ptr<netlab::network>  network() const noexcept { return m_network; }
    bool  has_network() const { return network().operator bool(); }
    void  initiate_network_construction(std::string const&  experiment_name);
    bool  is_network_being_constructed() const;

    std::string const&  get_experiment_name() const noexcept { return m_experiment_name; }

    /// Network simulation dependent methods.
    bool  paused() const noexcept { return m_paused; }
    float_64_bit  spent_real_time() const noexcept { return m_spent_real_time; }
    float_64_bit  spent_network_time() const { return m_spent_network_time; }
    natural_64_bit  num_network_updates() const noexcept { return m_num_network_updates; }
    float_64_bit  desired_network_to_real_time_ratio() const { return m_desired_network_to_real_time_ratio; }
    void set_desired_network_to_real_time_ratio(float_64_bit const  value);

//    bool  is_selected_cell() const { return m_selected_cell != nenet()->cells().cend(); }
//    bool  is_selected_input_spot() const { return m_selected_input_spot != nenet()->input_spots().cend(); }
//    bool  is_selected_output_terminal() const { return m_selected_output_terminal != nullptr; }
//    bool  is_selected_something() const { return is_selected_cell() || is_selected_input_spot() || is_selected_output_terminal(); }

//    vector3 const&  get_position_of_selected() const;
//    cell const&  get_selected_cell() const;
//    input_spot const&  get_selected_input_spot() const;
//    output_terminal const&  get_selected_output_terminal() const;

//    scalar  update_time_step_in_seconds() const { return nenet()->get_params()->update_time_step_in_seconds(); }

//    scalar  mini_spiking_potential_magnitude() const { return nenet()->get_params()->mini_spiking_potential_magnitude(); }
//    scalar  average_mini_spiking_period_in_seconds() const { return nenet()->get_params()->average_mini_spiking_period_in_seconds(); }

//    scalar  spiking_potential_magnitude() const { return nenet()->get_params()->spiking_potential_magnitude(); }
//    scalar  resting_potential() const { return nenet()->get_params()->resting_potential(); }
//    scalar  spiking_threshold() const { return nenet()->get_params()->spiking_threshold(); }
//    scalar  after_spike_potential() const { return nenet()->get_params()->after_spike_potential(); }
//    scalar  potential_descend_coef() const { return nenet()->get_params()->potential_descend_coef(); }
//    scalar  potential_ascend_coef() const { return nenet()->get_params()->potential_ascend_coef(); }
//    scalar  max_connection_distance() const { return nenet()->get_params()->max_connection_distance(); }

//    scalar  output_terminal_velocity_max_magnitude() const { return nenet()->get_params()->output_terminal_velocity_max_magnitude(); }
//    scalar  output_terminal_velocity_min_magnitude() const { return nenet()->get_params()->output_terminal_velocity_min_magnitude(); }

//    void  set_update_time_step_in_seconds(scalar const  value) { nenet()->get_params()->set_update_time_step_in_seconds(value); }

//    void  set_mini_spiking_potential_magnitude(scalar const  value) { nenet()->get_params()->set_mini_spiking_potential_magnitude(value); }
//    void  set_average_mini_spiking_period_in_seconds(scalar const  value) { nenet()->get_params()->set_average_mini_spiking_period_in_seconds(value); }

//    void  set_spiking_potential_magnitude(scalar const  value) { nenet()->get_params()->set_spiking_potential_magnitude(value); }
//    void  set_resting_potential(scalar const  value) { nenet()->get_params()->set_resting_potential(value); }
//    void  set_spiking_threshold(scalar const  value) { nenet()->get_params()->set_spiking_threshold(value); }
//    void  set_after_spike_potential(scalar const  value) { nenet()->get_params()->set_after_spike_potential(value); }
//    void  set_potential_descend_coef(scalar const  value) { nenet()->get_params()->set_potential_descend_coef(value); }
//    void  set_potential_ascend_coef(scalar const  value) { nenet()->get_params()->set_potential_ascend_coef(value); }
//    void  set_max_connection_distance(scalar const  value) { nenet()->get_params()->set_max_connection_distance(value); }

//    void  set_output_terminal_velocity_max_magnitude(scalar const  value) { nenet()->get_params()->set_output_terminal_velocity_max_magnitude(value); }
//    void  set_output_terminal_velocity_min_magnitude(scalar const  value) { nenet()->get_params()->set_output_terminal_velocity_min_magnitude(value); }

//    std::string  get_selected_info_text() const;

private:

    void  update_network(float_64_bit const  seconds_from_previous_call);
    void  update_selection_of_network_objects(float_64_bit const  seconds_from_previous_call);

    void  render_network(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr  draw_state);
    void  render_network_spikers(
            matrix44 const&  view_projection_matrix,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state_ptr&  draw_state
            );
    void  render_network_docks(
            matrix44 const&  view_projection_matrix,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state_ptr&  draw_state
            );
    void  render_network_ships(
            matrix44 const&  view_projection_matrix,
            std::vector< std::pair<vector3,vector3> > const& clip_planes,
            qtgl::draw_state_ptr&  draw_state
            );
    void  render_separate_network_camera(matrix44 const&  view_projection_matrix, qtgl::draw_state_ptr&  draw_state);


    /// Network independed data providing feedback loop between a human user and 3D scene in the tool
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;
    qtgl::batch_ptr  m_batch_grid;
    angeo::coordinate_system_ptr  m_camera_network_coord_system;
    float_32_bit  m_camera_network_far_plane;

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
    
    /// Data for handling selection of entities in the network
    std::shared_ptr<netlab::tracked_network_object_stats>  m_selected_object_stats;
    float_32_bit  m_selected_rot_angle;

    /// Data for rendering of entities in the network
    qtgl::batch_ptr  m_batch_spiker;
    qtgl::batch_ptr  m_batch_dock;
    qtgl::batch_ptr  m_batch_ship;

    qtgl::batch_ptr  m_batch_spiker_bbox;
    qtgl::batch_ptr  m_batch_dock_bbox;
    qtgl::batch_ptr  m_batch_ship_bbox;

    qtgl::batch_ptr  m_batch_spiker_bsphere;
    qtgl::batch_ptr  m_batch_dock_bsphere;
    qtgl::batch_ptr  m_batch_ship_bsphere;

    qtgl::batch_ptr  m_batch_basis;
    qtgl::batch_ptr  m_batch_camera_frustum;

//    qtgl::batch_ptr  m_selected_cell_input_spot_lines;
//    qtgl::batch_ptr  m_selected_cell_output_terminal_lines;
};


#endif
