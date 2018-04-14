#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include "./nenet.hpp"
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <qtgl/effects_config.hpp>
#   include <string>
#   include <memory>


struct simulator : public qtgl::real_time_simulator
{
    simulator(vector3 const&  initial_clear_colour, bool const  paused, nenet::params_ptr const  params,
              float_64_bit const  desired_number_of_simulated_seconds_per_real_time_second);
    ~simulator();
    void next_round(float_64_bit const  seconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);

    bool  is_selected_cell() const { return m_selected_cell != nenet()->cells().cend(); }
    bool  is_selected_input_spot() const { return m_selected_input_spot != nenet()->input_spots().cend(); }
    bool  is_selected_output_terminal() const { return m_selected_output_terminal != nullptr; }
    bool  is_selected_something() const { return is_selected_cell() || is_selected_input_spot() || is_selected_output_terminal(); }

    vector3 const&  get_position_of_selected() const;
    cell const&  get_selected_cell() const;
    input_spot const&  get_selected_input_spot() const;
    output_terminal const&  get_selected_output_terminal() const;

    std::shared_ptr<nenet>  nenet() const noexcept { return m_nenet; }

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }

    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }

    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }

    float_64_bit  spent_real_time() const noexcept { return m_spent_real_time; }
    natural_64_bit  nenet_num_updates() const noexcept { return nenet()->num_passed_updates(); }
    float_64_bit  spent_simulation_time() const { return nenet_num_updates() * nenet()->get_params()->update_time_step_in_seconds(); }
    float_64_bit  desired_number_of_simulated_seconds_per_real_time_second() const { return m_desired_number_of_simulated_seconds_per_real_time_second; }
    void set_desired_number_of_simulated_seconds_per_real_time_second(float_64_bit const  value);

    bool  paused() const noexcept { return m_paused; }

    scalar  update_time_step_in_seconds() const { return nenet()->get_params()->update_time_step_in_seconds(); }

    scalar  mini_spiking_potential_magnitude() const { return nenet()->get_params()->mini_spiking_potential_magnitude(); }
    scalar  average_mini_spiking_period_in_seconds() const { return nenet()->get_params()->average_mini_spiking_period_in_seconds(); }

    scalar  spiking_potential_magnitude() const { return nenet()->get_params()->spiking_potential_magnitude(); }
    scalar  resting_potential() const { return nenet()->get_params()->resting_potential(); }
    scalar  spiking_threshold() const { return nenet()->get_params()->spiking_threshold(); }
    scalar  after_spike_potential() const { return nenet()->get_params()->after_spike_potential(); }
    scalar  potential_descend_coef() const { return nenet()->get_params()->potential_descend_coef(); }
    scalar  potential_ascend_coef() const { return nenet()->get_params()->potential_ascend_coef(); }
    scalar  max_connection_distance() const { return nenet()->get_params()->max_connection_distance(); }

    scalar  output_terminal_velocity_max_magnitude() const { return nenet()->get_params()->output_terminal_velocity_max_magnitude(); }
    scalar  output_terminal_velocity_min_magnitude() const { return nenet()->get_params()->output_terminal_velocity_min_magnitude(); }

    void  set_update_time_step_in_seconds(scalar const  value) { nenet()->get_params()->set_update_time_step_in_seconds(value); }

    void  set_mini_spiking_potential_magnitude(scalar const  value) { nenet()->get_params()->set_mini_spiking_potential_magnitude(value); }
    void  set_average_mini_spiking_period_in_seconds(scalar const  value) { nenet()->get_params()->set_average_mini_spiking_period_in_seconds(value); }

    void  set_spiking_potential_magnitude(scalar const  value) { nenet()->get_params()->set_spiking_potential_magnitude(value); }
    void  set_resting_potential(scalar const  value) { nenet()->get_params()->set_resting_potential(value); }
    void  set_spiking_threshold(scalar const  value) { nenet()->get_params()->set_spiking_threshold(value); }
    void  set_after_spike_potential(scalar const  value) { nenet()->get_params()->set_after_spike_potential(value); }
    void  set_potential_descend_coef(scalar const  value) { nenet()->get_params()->set_potential_descend_coef(value); }
    void  set_potential_ascend_coef(scalar const  value) { nenet()->get_params()->set_potential_ascend_coef(value); }
    void  set_max_connection_distance(scalar const  value) { nenet()->get_params()->set_max_connection_distance(value); }

    void  set_output_terminal_velocity_max_magnitude(scalar const  value) { nenet()->get_params()->set_output_terminal_velocity_max_magnitude(value); }
    void  set_output_terminal_velocity_min_magnitude(scalar const  value) { nenet()->get_params()->set_output_terminal_velocity_min_magnitude(value); }

    std::string  get_selected_info_text() const;

private:
    std::shared_ptr<::nenet>  m_nenet;
    float_64_bit  m_spent_real_time;
    bool  m_paused;
    bool  m_do_single_step;
    float_64_bit  m_desired_number_of_simulated_seconds_per_real_time_second;

    cell::pos_map::const_iterator  m_selected_cell;
    input_spot::pos_map::const_iterator  m_selected_input_spot;
    output_terminal const*  m_selected_output_terminal;
    scalar  m_selected_rot_angle;

    std::unique_ptr<stats_of_cell>  m_selected_cell_stats;
    std::unique_ptr<stats_of_input_spot>  m_selected_input_spot_stats;
    std::unique_ptr<stats_of_output_terminal>  m_selected_output_terminal_stats;

    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;

    qtgl::effects_config  m_effects_config;

    qtgl::batch  m_batch_grid;
    qtgl::batch  m_batch_cell;
    qtgl::batch  m_batch_input_spot;
    qtgl::batch  m_batch_output_terminal;

    qtgl::batch  m_selected_cell_input_spot_lines;
    qtgl::batch  m_selected_cell_output_terminal_lines;
};

namespace notifications {


inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }
inline std::string  paused() { return "PAUSED"; }
inline std::string  selection_changed() { return "SELECTION_CHANGED"; }


}


#endif
