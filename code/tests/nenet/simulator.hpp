#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include "./nenet.hpp"
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <qtgl/draw.hpp>
#   include <memory>

struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();
    void next_round(float_64_bit const  seconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);

    std::shared_ptr<nenet>  nenet() const noexcept { return m_nenet; }

private:
    std::shared_ptr<::nenet>  m_nenet;
    natural_64_bit  m_nenet_num_updates;
    float_64_bit  m_nenet_max_update_duration;

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


#endif
