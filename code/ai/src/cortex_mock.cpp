#include <ai/cortex_mock.hpp>
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

    context const  ctx { time_step_in_seconds };
    for (auto const&  null_and_step : get_blackboard()->m_motion_templates.get_transitions_mock())
        if (check_keys(null_and_step.second.get_child("down"), true) && check_keys(null_and_step.second.get_child("up"), false))
            for (auto const& null_and_assignment : null_and_step.second.get_child("assignments"))
            {
                std::string const  var = null_and_assignment.second.get_child("var").get_value<std::string>();
                skeletal_motion_templates::property_tree const&  expr = null_and_assignment.second.get_child("value");
                if (is_desire_scalar_variable(var))
                {
                    float_32_bit  value = evaluate_scalar_expression(expr, ctx);
                    value = std::max(0.0f, std::min(1.0f, value));
                    *get_address_of_desire_scalar_variable(var) = value;
                }
                else
                {
                    vector3  value = evaluate_vector_expression(expr, ctx);
                    if (!can_desire_vector_variable_be_zero(var))
                    {
                        float_32_bit const  len = length(value);
                        value = (len < 1e-5f) ? vector3_unit_y() : (1.0f / len) * value;
                    }
                    *get_address_of_desire_vector_variable(var) = value;
                }
            }
}


void  cortex_mock::__check_loaded_data__(skeletal_motion_templates::property_tree const&  data, std::string const&  message_prefix)
{
    std::string const  prefix = message_prefix + " [cortex_mock]: ";
    int i = 0;
    for (auto const&  null_and_child : data)
    {
        ++i;
        if (null_and_child.second.size() != 3UL)
            throw std::runtime_error(msgstream() << prefix << "In command no. " << i << ": Wrong number of keys; expecting 3, found "
                                                 << null_and_child.second.size());
        if (null_and_child.second.count("down") != 1UL)
            throw std::runtime_error(msgstream() << prefix << "In command no. " << i << ": The key 'down' must be present exactly once.");
        if (null_and_child.second.count("up") != 1UL)
            throw std::runtime_error(msgstream() << prefix << "In command no. " << i << ": The key 'up' must be present exactly once.");
        if (null_and_child.second.count("assignments") != 1UL)
            throw std::runtime_error(msgstream() << prefix << "In command no. " << i << ": The key 'assignments' must be present exactly once.");

        int j = 0;
        for (auto const& null_and_assignment : null_and_child.second.get_child("assignments"))
        {
            ++j;
            if (null_and_assignment.second.size() != 2UL)
                throw std::runtime_error(msgstream() << prefix << "In command no. " << i << " assignment no. " << j << ": "
                                                     << "Wrong number of keys; expecting 2, found " << null_and_assignment.second.size());
            if (null_and_assignment.second.count("var") != 1UL)
                throw std::runtime_error(msgstream() << prefix << "In command no. " << i << " assignment no. " << j << ": "
                                                     << "The key 'var' must be present exactly once.");
            if (null_and_assignment.second.count("value") != 1UL)
                throw std::runtime_error(msgstream() << prefix << "In command no. " << i << " assignment no. " << j << ": "
                                                     << "The key 'value' must be present exactly once.");
            std::string const  var = null_and_assignment.second.get_child("var").get_value<std::string>();
            if (!cortex::is_desire_scalar_variable(var) && !cortex::is_desire_vector_variable(var))
                throw std::runtime_error(msgstream() << prefix << "In command no. " << i << " assignment no. " << j << ": "
                                                     << "The assigned variable is neither scalar nor vector desire variable.");
        }
    }
}


}
