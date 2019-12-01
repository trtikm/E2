#include <ai/cortex_mock.hpp>
#include <ai/detail/expression_evaluator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


void  cortex_mock::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    auto const  parse_keys =
        [this](
            std::string const&  keys_pattern,
            std::vector<qtgl::keyboard_key_name>&  output_keyboard_keys,
            std::vector<qtgl::mouse_button_name>&  output_mouse_buttons
        ) -> void {
            ASSUMPTION(!keys_pattern.empty());
            if (keys_pattern == qtgl::LEFT_MOUSE_BUTTON() || keys_pattern == qtgl::RIGHT_MOUSE_BUTTON() || keys_pattern == qtgl::MIDDLE_MOUSE_BUTTON())
                output_mouse_buttons.push_back(keys_pattern);
            else if (keys_pattern.front() == '?')
            {
                output_keyboard_keys.push_back('L' + keys_pattern.substr(1));
                output_keyboard_keys.push_back('R' + keys_pattern.substr(1));
            }
            else
                output_keyboard_keys.push_back(keys_pattern);
        };

    auto const  check_keys = [this, &parse_keys](skeletal_motion_templates::property_tree const&  patterns, bool const  must_be_pressed) -> bool {
        for (auto const& null_and_key_pattern : patterns)
        {
            std::vector<qtgl::keyboard_key_name>  keys;
            std::vector<qtgl::mouse_button_name>  buttons;
            parse_keys(null_and_key_pattern.second.get_value<std::string>(), keys, buttons);
            bool  success = false;
            for (auto const&  key : keys)
                if (m_input_devices->keyboard.is_pressed(key) == must_be_pressed)
                {
                    success = true;
                    break;
                }
            for (auto const&  button : buttons)
                if (m_input_devices->mouse.is_pressed(button) == must_be_pressed)
                {
                    success = true;
                    break;
                }
            if (success == false)
                return false;
        }
        return true;
    };

    detail::eval::context const  ctx { &m_motion_desire_props, get_blackboard()->m_motion_templates.directions(), time_step_in_seconds };
    for (auto const&  null_and_step : get_blackboard()->m_motion_templates.get_transitions_mock())
        if (check_keys(null_and_step.second.get_child("down"), true) && check_keys(null_and_step.second.get_child("up"), false))
            for (auto const& null_and_assignment : null_and_step.second.get_child("assignments"))
            {
                std::string const  var = null_and_assignment.second.get_child("var").get_value<std::string>();
                skeletal_motion_templates::property_tree const&  expr = null_and_assignment.second.get_child("value");
                if (detail::eval::is_desire_scalar_variable(var))
                {
                    float_32_bit  value = evaluate_scalar_expression(expr, ctx);
                    value = std::max(0.0f, std::min(1.0f, value));
                    *detail::eval::get_address_of_desire_scalar_variable(var, m_motion_desire_props) = value;
                }
                else
                {
                    vector3  value = evaluate_vector_expression(expr, ctx);
                    if (detail::eval::vector_variable_has_to_be_normalised(var))
                    {
                        float_32_bit const  len = length(value);
                        value = (len < 1e-5f) ? vector3_unit_y() : (1.0f / len) * value;
                    }
                    *detail::eval::get_address_of_desire_vector_variable(var, m_motion_desire_props) = value;
                }
            }
}


}
