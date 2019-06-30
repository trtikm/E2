#include <qtgl/free_fly.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace free_fly_controler {


controler_fn   AND(std::vector<controler_fn> const&  controlers)
{
    return [controlers](mouse_props const&  mouse_info, keyboard_props const&  keybord_props) {
                    for (auto const& elem : controlers)
                        if (!elem(mouse_info,keybord_props))
                            return false;
                    return true;
                    };
}

controler_fn   OR(std::vector<controler_fn> const&  controlers)
{
    return [controlers](mouse_props const&  mouse_info, keyboard_props const&  keybord_props) {
                    for (auto const& elem : controlers)
                        if (elem(mouse_info,keybord_props))
                            return true;
                    return false;
                    };
}

controler_fn  NOT(controler_fn const  controler)
{
    return [controler](mouse_props const&  mouse_info, keyboard_props const&  keybord_props) {
                    return !controler(mouse_info,keybord_props);
                    };
}


controler_fn  mouse_button_pressed(mouse_button_name const&  button)
{
    return [button](mouse_props const&  mouse_info, keyboard_props const&) {
                    return mouse_info.is_pressed(button);
                    };
}

controler_fn  keyboard_key_pressed(keyboard_key_name const&  key)
{
    return [key](mouse_props const&, keyboard_props const& keyboard_info) {
                    return keyboard_info.is_pressed(key);
                    };
}


}}

namespace qtgl {


free_fly_action::free_fly_action(
        bool const  do_rotation,
        bool const  use_world_axis,
        natural_8_bit const  axis_index,
        natural_8_bit const  mouse_move_axis,
        float_32_bit const  action_value,
        free_fly_controler::controler_fn  const&  controler
        )
    : m_do_rotation(do_rotation)
    , m_use_world_axis(use_world_axis)
    , m_axis_index(axis_index)
    , m_mouse_move_axis(mouse_move_axis)
    , m_action_value(action_value)
    , m_controler(controler)
{
    ASSUMPTION(m_axis_index <= 2U);
    ASSUMPTION(m_mouse_move_axis <= 2U);
}


free_fly_report  free_fly(angeo::coordinate_system&  coord_system,
               free_fly_config const&  config,
               float_64_bit const  seconds_from_previous_call,
               mouse_props const&  mouse_info,
               keyboard_props const&  keyboard_info)
{
    TMPROF_BLOCK();

    bool  translated = false;
    bool  rotated = false;
    for (free_fly_action const& action : config)
        if (action.controler()(mouse_info,keyboard_info))
        {
            vector3  action_axis;
            {
                if (action.use_world_axis())
                    switch (action.axis_index())
                    {
                    case 0U: action_axis = vector3_unit_x(); break;
                    case 1U: action_axis = vector3_unit_y(); break;
                    default: action_axis = vector3_unit_z(); break;
                    }
                else
                    action_axis = axis(coord_system,action.axis_index());
            }

            scalar  action_value;
            {
                switch (action.mouse_move_axis())
                {
                case 0U: action_value = (scalar)mouse_info.x_delta(); break;
                case 1U: action_value = (scalar)mouse_info.y_delta(); break;
                default: action_value = (scalar)seconds_from_previous_call; break;
                }
                action_value *= action.action_value();
            }

            if (action.do_rotation())
            {
                angeo::rotate(coord_system,angle_axis_to_quaternion(action_value,action_axis));
                rotated = true;
            }
            else
            {
                angeo::translate(coord_system,action_value * action_axis);
                translated = true;
            }
        }
    return {translated,rotated};
}


}
