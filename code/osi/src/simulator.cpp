#include <osi/simulator.hpp>
#include <osi/provider.hpp>

namespace osi {


float_64_bit  simulator::seconds_openned() const { return osi::seconds_openned(); }
float_64_bit  simulator::round_start_time() const { return osi::round_start_time(); }
float_32_bit  simulator::round_seconds() const { return osi::round_seconds(); }

bool  simulator::has_focus() const { return osi::has_focus(); }
bool  simulator::focus_just_received() const { return osi::focus_just_received(); }
bool  simulator::focus_just_lost() const { return osi::focus_just_lost(); }
natural_16_bit  simulator::window_width() const { return osi::window_width(); }
natural_16_bit  simulator::window_height() const { return osi::window_height(); }

std::string const&  simulator::typed_text() const { return osi::typed_text(); }
std::unordered_set<keyboard_key_name> const&  simulator::keys_pressed() const { return osi::keys_pressed(); }
std::unordered_set<keyboard_key_name> const&  simulator::keys_just_pressed() const { return osi::keys_just_pressed(); }
std::unordered_set<keyboard_key_name> const&  simulator::keys_just_released() const { return osi::keys_just_released(); }

float_32_bit  simulator::cursor_x() const { return osi::cursor_x(); }
float_32_bit  simulator::cursor_y() const { return osi::cursor_y(); }
float_32_bit  simulator::cursor_x_delta() const { return osi::cursor_x_delta(); }
float_32_bit  simulator::cursor_y_delta() const { return osi::cursor_y_delta(); }
float_32_bit  simulator::wheel_delta_x() const { return osi::wheel_delta_x(); }
float_32_bit  simulator::wheel_delta_y() const { return osi::wheel_delta_y(); }
std::unordered_set<mouse_button_name> const&  simulator::buttons_pressed() const { return osi::buttons_pressed(); }
std::unordered_set<mouse_button_name> const&  simulator::buttons_just_pressed() const { return osi::buttons_just_pressed(); }
std::unordered_set<mouse_button_name> const&  simulator::buttons_just_released() const { return osi::buttons_just_released(); }

std::string const&  simulator::error_text() const { return osi::error_text(); }


void  run(simulator&  s)
{
    try
    {
        osi::open();
        if (!osi::is_open())
            return;

        s.initialise();

        while (!osi::is_close_requested())
        {
            osi::start_round();

            s.round();

            osi::finish_round();
        }

        s.terminate();

        osi::close();
    }
    catch (...)
    {
        s.terminate();

        osi::close();
    }
}


}
