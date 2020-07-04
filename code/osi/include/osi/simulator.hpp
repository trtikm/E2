#ifndef OSI_SIMULATOR_HPP_INCLUDED
#   define OSI_SIMULATOR_HPP_INCLUDED

#   include <osi/keyboard_key_names.hpp>
#   include <osi/mouse_button_names.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>
#   include <string>

namespace osi {


struct  simulator
{
    virtual  ~simulator() {}

    virtual void  initialise() {}
    virtual void  round() {}
    virtual void  terminate() {}

    float_64_bit  seconds_openned() const;
    float_64_bit  round_start_time() const;
    float_32_bit  round_seconds() const;

    bool  has_focus() const;
    bool  focus_just_received() const;
    bool  focus_just_lost() const;
    natural_16_bit  window_width() const;
    natural_16_bit  window_height() const;

    std::string const&  typed_text() const;
    std::unordered_set<keyboard_key_name> const&  keys_pressed() const;
    std::unordered_set<keyboard_key_name> const&  keys_just_pressed() const;
    std::unordered_set<keyboard_key_name> const&  keys_just_released() const;

    float_32_bit  cursor_x() const;
    float_32_bit  cursor_y() const;
    float_32_bit  cursor_x_delta() const;
    float_32_bit  cursor_y_delta() const;
    float_32_bit  wheel_delta_x() const;
    float_32_bit  wheel_delta_y() const;
    std::unordered_set<mouse_button_name> const&  buttons_pressed() const;
    std::unordered_set<mouse_button_name> const&  buttons_just_pressed() const;
    std::unordered_set<mouse_button_name> const&  buttons_just_released() const;

    std::string const&  error_text() const;
};


void  run(simulator&  s);


}

#endif
