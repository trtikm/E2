#ifndef OSI_SIMULATOR_HPP_INCLUDED
#   define OSI_SIMULATOR_HPP_INCLUDED

#   include <osi/window_props.hpp>
#   include <osi/keyboard_props.hpp>
#   include <osi/mouse_props.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <string>
#   include <vector>
#   include <memory>

namespace osi {


struct  simulator
{
    virtual  ~simulator() {}

    virtual void  initialise() {}
    virtual void  round() {}
    virtual void  terminate() {}

    void  set_window_title(std::string const&  title);
    void  set_window_icon(natural_8_bit const  width, natural_8_bit const  height, std::vector<natural_8_bit> const&  pixels_rgba_8888);
    void  set_window_pos(natural_16_bit const  x, natural_16_bit const  y);
    void  set_window_size(natural_16_bit const  width, natural_16_bit const  height);
    void  restore_window();
    void  maximise_window();

    natural_64_bit  round_number() const;
    float_64_bit  seconds_openned() const;
    float_64_bit  round_start_time() const;
    float_32_bit  round_seconds() const;

    window_props const&  get_window_props() const;
    keyboard_props const&  get_keyboard_props() const;
    mouse_props const&  get_mouse_props() const;

    void  send_close_request();

    std::string const&  error_text() const;

private:

    static window_props  s_window_props;
    static keyboard_props  s_keyboard_props;
    static mouse_props  s_mouse_props;
};


void  run(std::unique_ptr<simulator>  s);


}

#endif
