#ifndef OSI_PROVIDER_HPP_INCLUDED
#   define OSI_PROVIDER_HPP_INCLUDED

#   include <osi/opengl.hpp>
#   include <osi/keyboard_key_names.hpp>
#   include <osi/mouse_button_names.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <vector>
#   include <unordered_set>
#   include <string>

namespace osi {


void  open();
void  close();

bool  is_open();
bool  is_close_requested();

void  start_round();
void  finish_round();

natural_64_bit  round_number();

float_64_bit  seconds_openned();
float_64_bit  round_start_time();
float_32_bit  round_seconds();

bool  has_focus();
bool  focus_just_received();
bool  focus_just_lost();
natural_16_bit  window_width();
natural_16_bit  window_height();

std::string const&  typed_text();
std::unordered_set<keyboard_key_name> const&  keys_pressed();
std::unordered_set<keyboard_key_name> const&  keys_just_pressed();
std::unordered_set<keyboard_key_name> const&  keys_just_released();

float_32_bit  cursor_x();
float_32_bit  cursor_y();
float_32_bit  cursor_x_delta();
float_32_bit  cursor_y_delta();
float_32_bit  wheel_delta_x();
float_32_bit  wheel_delta_y();
std::unordered_set<mouse_button_name> const&  buttons_pressed();
std::unordered_set<mouse_button_name> const&  buttons_just_pressed();
std::unordered_set<mouse_button_name> const&  buttons_just_released();

std::string const&  error_text();


}

#endif