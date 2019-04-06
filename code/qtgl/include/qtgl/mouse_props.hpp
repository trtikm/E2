#ifndef QTGL_MOUSE_PROPS_HPP_INCLUDED
#   define QTGL_MOUSE_PROPS_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <unordered_set>
#   include <string>
#   include <vector>
#   include <tuple>

namespace qtgl {


typedef std::string  mouse_button_name;

mouse_button_name  LEFT_MOUSE_BUTTON();
mouse_button_name  RIGHT_MOUSE_BUTTON();
mouse_button_name  MIDDLE_MOUSE_BUTTON();


struct mouse_props
{
    mouse_props();
    mouse_props(
            float_32_bit const  x,                      // Current mouse x-position
            float_32_bit const  y,                      // Current mouse y-position
            float_32_bit const  x_delta,                // A change along x-axis from the previous step
            float_32_bit const  y_delta,                // A change along y-axis from the previous step
            float_32_bit const  wheel_delta_x,          // A change along x-axis from the previous step
            float_32_bit const  wheel_delta_y,          // A change along y-axis from the previous step
            std::vector< std::tuple<mouse_button_name,  // button name
                                    bool,               // is pressed?
                                    bool,               // was just pressed?
                                    bool>               // was just released?
                > const&  button_props
            );
    float_32_bit  x() const { return m_x; }
    float_32_bit  y() const { return m_y; }
    float_32_bit  x_delta() const { return m_x_delta; }
    float_32_bit  y_delta() const { return m_y_delta; }
    float_32_bit  wheel_delta_x() const { return m_wheel_delta_x; }
    float_32_bit  wheel_delta_y() const { return m_wheel_delta_y; }
    bool  is_pressed(mouse_button_name const&  part_name) const { return m_pressed.count(part_name) != 0ULL; }
    bool  was_just_pressed(mouse_button_name const&  part_name) const { return m_just_pressed.count(part_name) != 0ULL; }
    bool  was_just_released(mouse_button_name const&  part_name) const { return m_just_released.count(part_name) != 0ULL; }
private:
    float_32_bit  m_x;
    float_32_bit  m_y;
    float_32_bit  m_x_delta;
    float_32_bit  m_y_delta;
    float_32_bit  m_wheel_delta_x;
    float_32_bit  m_wheel_delta_y;
    std::unordered_set<mouse_button_name>  m_pressed;
    std::unordered_set<mouse_button_name>  m_just_pressed;
    std::unordered_set<mouse_button_name>  m_just_released;
};


}

#endif
