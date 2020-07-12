#include <osi/simulator.hpp>
#include <osi/provider.hpp>
#include <utility/async_resource_load.hpp>

namespace osi {


window_props  simulator::s_window_props;
keyboard_props  simulator::s_keyboard_props;
mouse_props  simulator::s_mouse_props;

void  simulator::set_window_title(std::string const&  title) { return osi::set_window_title(title); }
void  simulator::set_window_pos(natural_16_bit const  x, natural_16_bit const  y) { return osi::set_window_pos(x, y); }
void  simulator::set_window_size(natural_16_bit const  width, natural_16_bit const  height) { osi::set_window_size(width, height); }

natural_64_bit  simulator::round_number() const { return osi::round_number(); }
float_64_bit  simulator::seconds_openned() const { return osi::seconds_openned(); }
float_64_bit  simulator::round_start_time() const { return osi::round_start_time(); }
float_32_bit  simulator::round_seconds() const { return osi::round_seconds(); }

window_props const&  simulator::get_window_props() const { return s_window_props; }
keyboard_props const&  simulator::get_keyboard_props() const { return s_keyboard_props; }
mouse_props const&  simulator::get_mouse_props() const { return s_mouse_props; }

std::string const&  simulator::error_text() const { return osi::error_text(); }


void  run(std::unique_ptr<simulator>  s)
{
    if (s == nullptr)
        return;

    try
    {
        osi::open();
        if (!osi::is_open())
            return;

        s->initialise();

        while (!osi::is_close_requested())
        {
            osi::start_round();

            s->round();

            osi::finish_round();
        }

        async::terminate();

        s->terminate();
        s.reset();

        osi::close();
    }
    catch (...)
    {
        s->terminate();
        s.reset();

        osi::close();
    }
}


}
