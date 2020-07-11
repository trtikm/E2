#ifndef OSI_SIMULATOR_HPP_INCLUDED
#   define OSI_SIMULATOR_HPP_INCLUDED

#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <memory>

namespace osi {


struct  simulator
{
    virtual  ~simulator() {}

    virtual void  initialise() {}
    virtual void  round() {}
    virtual void  terminate() {}

    natural_64_bit  round_number() const;
    float_64_bit  seconds_openned() const;
    float_64_bit  round_start_time() const;
    float_32_bit  round_seconds() const;

    window_props const&  get_window_props() const;
    keyboard_props const&  get_keyboard_props() const;
    mouse_props const&  get_mouse_props() const;

    std::string const&  error_text() const;

private:

    static window_props  s_window_props;
    static keyboard_props  s_keyboard_props;
    static mouse_props  s_mouse_props;
};


void  run(std::unique_ptr<simulator>  s);


}

#endif
