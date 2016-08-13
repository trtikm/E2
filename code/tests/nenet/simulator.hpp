#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include "./nenet.hpp"
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <string>
#   include <memory>


struct simulator : public qtgl::real_time_simulator
{
    simulator(vector3 const&  initial_clear_colour, bool const  paused);
    ~simulator();
    void next_round(float_64_bit const  seconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);

    std::shared_ptr<nenet>  nenet() const noexcept { return m_nenet; }

    void  set_clear_color(vector3 const&  colour) { qtgl::glapi().glClearColor(colour(0), colour(1), colour(2), 1.0f); }

    vector3 const&  get_camera_position() const { return m_camera->coordinate_system()->origin(); }
    quaternion const&  get_camera_orientation() const { return m_camera->coordinate_system()->orientation(); }

    void  set_camera_position(vector3 const&  position) { m_camera->coordinate_system()->set_origin(position); }
    void  set_camera_orientation(quaternion const&  orientation) { m_camera->coordinate_system()->set_orientation(orientation); }

    bool  paused() const noexcept { return m_paused; }

private:
    std::shared_ptr<::nenet>  m_nenet;
    natural_64_bit  m_nenet_num_updates;
    float_64_bit  m_nenet_max_update_duration;
    bool  m_paused;

    cell::pos_map::const_iterator  m_selected_cell;
    scalar  m_selected_rot_angle;

    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;

    qtgl::batch_ptr  m_batch_grid;
    qtgl::batch_ptr  m_batch_cell;
    qtgl::batch_ptr  m_batch_input_spot;
    qtgl::batch_ptr  m_batch_output_terminal;

    qtgl::batch_ptr  m_selected_cell_input_spot_lines;
    qtgl::batch_ptr  m_selected_cell_output_terminal_lines;
};

namespace notifications {


inline std::string  camera_position_updated() { return "CAMERA_POSITION_UPDATED"; }
inline std::string  camera_orientation_updated() { return "CAMERA_ORIENTATION_UPDATED"; }
inline std::string  paused() { return "PAUSED"; }


}


#endif
