#include <gfx/free_fly.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace gfx { namespace free_fly_controler {


controler_fn   AND(std::vector<controler_fn> const&  controlers)
{
    return [controlers](osi::mouse_props const&  mouse_info, osi::keyboard_props const&  keybord_props) {
                    for (auto const& elem : controlers)
                        if (!elem(mouse_info,keybord_props))
                            return false;
                    return true;
                    };
}

controler_fn   OR(std::vector<controler_fn> const&  controlers)
{
    return [controlers](osi::mouse_props const&  mouse_info, osi::keyboard_props const&  keybord_props) {
                    for (auto const& elem : controlers)
                        if (elem(mouse_info,keybord_props))
                            return true;
                    return false;
                    };
}

controler_fn  NOT(controler_fn const  controler)
{
    return [controler](osi::mouse_props const&  mouse_info, osi::keyboard_props const&  keybord_props) {
                    return !controler(mouse_info,keybord_props);
                    };
}


controler_fn  mouse_button_pressed(osi::mouse_button_name const&  button)
{
    return [button](osi::mouse_props const&  mouse_info, osi::keyboard_props const&) {
                    return mouse_info.buttons_pressed().count(button) != 0UL;
                    };
}

controler_fn  keyboard_key_pressed(osi::keyboard_key_name const&  key)
{
    return [key](osi::mouse_props const&, osi::keyboard_props const& keyboard_info) {
                    return keyboard_info.keys_pressed().count(key) != 0UL;
                    };
}


}}

namespace gfx {


free_fly_action::free_fly_action(
        bool const  do_rotation,
        bool const  use_world_axis,
        natural_8_bit const  axis_index,
        natural_8_bit const  mouse_move_axis,
        float_32_bit const  action_value,
        free_fly_controler::controler_fn  const&  controler,
        std::string const&  help_
        )
    : m_do_rotation(do_rotation)
    , m_use_world_axis(use_world_axis)
    , m_axis_index(axis_index)
    , m_mouse_move_axis(mouse_move_axis)
    , m_action_value(action_value)
    , m_controler(controler)
    , m_help(help_)
{
    ASSUMPTION(m_axis_index <= 2U);
    ASSUMPTION(m_mouse_move_axis <= 2U);
}


free_fly_config  default_free_fly_config(float_32_bit const  pixel_width_mm, float_32_bit const  pixel_height_mm)
{
    using namespace  gfx::free_fly_controler;
    return
        {
            {
                false,
                false,
                2U,
                2U,
                -15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_W()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "W - forward"
            },
            {
                false,
                false,
                2U,
                2U,
                15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_S()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "S - backward"
            },
            {
                false,
                false,
                0U,
                2U,
                -15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_A()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "A - left"
            },
            {
                false,
                false,
                0U,
                2U,
                15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_D()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "D - right"
            },
            {
                false,
                false,
                1U,
                2U,
                -15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_Q()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "Q - down"
            },
            {
                false,
                false,
                1U,
                2U,
                15.0f,
                AND({
                    keyboard_key_pressed(osi::KEY_E()),
                    NOT(
                        OR({
                            keyboard_key_pressed(osi::KEY_LALT()),
                            keyboard_key_pressed(osi::KEY_RALT()),
                            keyboard_key_pressed(osi::KEY_LCTRL()),
                            keyboard_key_pressed(osi::KEY_RCTRL()),
                            keyboard_key_pressed(osi::KEY_LSHIFT()),
                            keyboard_key_pressed(osi::KEY_RSHIFT()),
                            })
                        )
                    }),
                "E - up"
            },
            {
                true,
                true,
                2U,
                0U,
                -(10.0f * PI()) * (pixel_width_mm / 1000.0f),
                mouse_button_pressed(osi::MIDDLE_MOUSE_BUTTON()),
                "MOUSE_MODDLE - rotate"
            },
            {
                true,
                false,
                0U,
                1U,
                -(10.0f * PI()) * (pixel_height_mm / 1000.0f),
                mouse_button_pressed(osi::MIDDLE_MOUSE_BUTTON()),
                ""
            },
        };
}


free_fly_report  free_fly(angeo::coordinate_system&  coord_system,
               free_fly_config const&  config,
               float_64_bit const  seconds_from_previous_call,
               osi::mouse_props const&  mouse_info,
               osi::keyboard_props const&  keyboard_info)
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
                case 0U: action_value = (scalar)mouse_info.cursor_x_delta(); break;
                case 1U: action_value = (scalar)mouse_info.cursor_y_delta(); break;
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
