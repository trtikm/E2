#ifndef QTGL_REAL_TIME_SIMULATOR_HPP_INCLUDED
#   define QTGL_REAL_TIME_SIMULATOR_HPP_INCLUDED

#   include <qtgl/window_props.hpp>
#   include <qtgl/keyboard_props.hpp>
#   include <qtgl/mouse_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <chrono>
#   include <string>

namespace qtgl {


struct real_time_simulator
{
    virtual ~real_time_simulator() {}

    virtual void synchronise_with_dependent_modules() {}

    virtual void next_round(float_64_bit const  seconds_from_previous_call,
                            bool const  is_this_pure_redraw_request) {}

    void  call_listeners(std::string const&  notification_type) const;

    qtgl::window_props const&  window_props() const;
    qtgl::mouse_props const&  mouse_props() const;
    qtgl::keyboard_props const&  keyboard_props() const;

    natural_64_bit  round_id() const;
    std::chrono::high_resolution_clock::time_point  start_time() const;
    float_64_bit  total_simulation_time() const;
    natural_32_bit  FPS() const;
};


}

namespace qtgl { namespace notifications {


inline std::string  fps_updated() { return "QTGL_NOTIFICATION_FPS_UPDATED"; }


}}

#endif
