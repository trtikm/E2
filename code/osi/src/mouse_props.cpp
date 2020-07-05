#include <osi/mouse_props.hpp>
#include <osi/provider.hpp>

namespace osi {


float_32_bit  mouse_props::cursor_x() const { return osi::cursor_x(); }
float_32_bit  mouse_props::cursor_y() const { return osi::cursor_y(); }
float_32_bit  mouse_props::cursor_x_delta() const { return osi::cursor_x_delta(); }
float_32_bit  mouse_props::cursor_y_delta() const { return osi::cursor_y_delta(); }
float_32_bit  mouse_props::wheel_delta_x() const { return osi::wheel_delta_x(); }
float_32_bit  mouse_props::wheel_delta_y() const { return osi::wheel_delta_y(); }
std::unordered_set<mouse_button_name> const&  mouse_props::buttons_pressed() const { return osi::buttons_pressed(); }
std::unordered_set<mouse_button_name> const&  mouse_props::buttons_just_pressed() const { return osi::buttons_just_pressed(); }
std::unordered_set<mouse_button_name> const&  mouse_props::buttons_just_released() const { return osi::buttons_just_released(); }


}
