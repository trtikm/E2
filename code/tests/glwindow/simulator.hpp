#ifndef SIMULATOR_HPP_INCLUDED
#   define SIMULATOR_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/camera.hpp>
#   include <qtgl/free_fly.hpp>
#   include <utility/basic_numeric_types.hpp>


struct simulator : public qtgl::real_time_simulator
{
    simulator();
    ~simulator();
    void next_round(float_64_bit const  miliseconds_from_previous_call,
                    bool const  is_this_pure_redraw_request);

private:
    qtgl::camera_perspective_ptr  m_camera;
    qtgl::free_fly_config  m_free_fly_config;
};


#endif
