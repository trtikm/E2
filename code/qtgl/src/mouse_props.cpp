#include <qtgl/mouse_props.hpp>
#include <utility/assumptions.hpp>

namespace qtgl {


mouse_button_name  LEFT_MOUSE_BUTTON() { return "LMOUSE"; }
mouse_button_name  RIGHT_MOUSE_BUTTON() { return "RMOUSE"; }
mouse_button_name  MIDDLE_MOUSE_BUTTON() { return "MMOUSE"; }


mouse_props::mouse_props()
    : m_x(0.0f)
    , m_y(0.0f)
    , m_x_delta(0.0f)
    , m_y_delta(0.0f)
    , m_wheel_delta_x(0.0f)
    , m_wheel_delta_y(0.0f)
    , m_pressed()
    , m_just_pressed()
    , m_just_released()
{}


mouse_props::mouse_props(
        float_32_bit const  x,
        float_32_bit const  y,
        float_32_bit const  x_delta,
        float_32_bit const  y_delta,
        float_32_bit const  wheel_delta_x,
        float_32_bit const  wheel_delta_y,
        std::vector< std::tuple<mouse_button_name,    // button name
                                bool,               // is pressed?
                                bool,               // was just pressed?
                                bool>               // was just released?
            > const&  button_props
        )
    : m_x(x)
    , m_y(y)
    , m_x_delta(x_delta)
    , m_y_delta(y_delta)
    , m_wheel_delta_x(wheel_delta_x)
    , m_wheel_delta_y(wheel_delta_y)
    , m_pressed()
    , m_just_pressed()
    , m_just_released()
{
    for (auto const& elem : button_props)
    {
        ASSUMPTION(
                std::get<0>(elem) == LEFT_MOUSE_BUTTON() ||
                std::get<0>(elem) == RIGHT_MOUSE_BUTTON() ||
                std::get<0>(elem) == MIDDLE_MOUSE_BUTTON()
                );
        if (std::get<1>(elem))
            m_pressed.insert(std::get<0>(elem));
        if (std::get<2>(elem))
            m_just_pressed.insert(std::get<0>(elem));
        if (std::get<3>(elem))
            m_just_released.insert(std::get<0>(elem));
    }
}


}
