#ifndef OSI_MOUSE_PROPS_HPP_INCLUDED
#   define OSI_MOUSE_PROPS_HPP_INCLUDED

#   include <osi/mouse_button_names.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <unordered_set>

namespace osi {


struct  mouse_props
{
    float_32_bit  cursor_x() const;
    float_32_bit  cursor_y() const;
    float_32_bit  cursor_x_delta() const;
    float_32_bit  cursor_y_delta() const;
    float_32_bit  wheel_delta_x() const;
    float_32_bit  wheel_delta_y() const;
    std::unordered_set<mouse_button_name> const&  buttons_pressed() const;
    std::unordered_set<mouse_button_name> const&  buttons_just_pressed() const;
    std::unordered_set<mouse_button_name> const&  buttons_just_released() const;
};


}

#endif
