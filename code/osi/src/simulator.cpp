#include <osi/simulator.hpp>
#include <osi/provider.hpp>
#include <utility/async_resource_load.hpp>
#include <utility/config.hpp>
#if PLATFORM() == PLATFORM_WEBASSEMBLY()
#   include <emscripten.h> // For emscripten_get_device_pixel_ratio()
#   include <emscripten/html5.h> // For Emscripten HTML5 WebGL context creation API
#endif
#include <stdexcept>

namespace osi {


window_props  simulator::s_window_props;
keyboard_props  simulator::s_keyboard_props;
mouse_props  simulator::s_mouse_props;

void  simulator::set_window_title(std::string const&  title) { return osi::set_window_title(title); }
void  simulator::set_window_icon(natural_8_bit const  width, natural_8_bit const  height,
                                 std::vector<natural_8_bit> const&  pixels_rgba_8888)
{ osi::set_window_icon(width, height, pixels_rgba_8888); }
void  simulator::set_window_pos(natural_16_bit const  x, natural_16_bit const  y) { return osi::set_window_pos(x, y); }
void  simulator::set_window_size(natural_16_bit const  width, natural_16_bit const  height) { osi::set_window_size(width, height); }
void  simulator::restore_window() { osi::restore_window(); }
void  simulator::maximise_window() { osi::maximise_window(); }

natural_64_bit  simulator::round_number() const { return osi::round_number(); }
float_64_bit  simulator::seconds_openned() const { return osi::seconds_openned(); }
float_64_bit  simulator::round_start_time() const { return osi::round_start_time(); }
float_32_bit  simulator::round_seconds() const { return osi::round_seconds(); }

window_props const&  simulator::get_window_props() const { return s_window_props; }
keyboard_props const&  simulator::get_keyboard_props() const { return s_keyboard_props; }
mouse_props const&  simulator::get_mouse_props() const { return s_mouse_props; }

void  simulator::send_close_request() { osi::send_close_request(); }

std::string const&  simulator::error_text() const { return osi::error_text(); }

#if PLATFORM() == PLATFORM_WEBASSEMBLY()
static std::unique_ptr<simulator> s_sim_ptr = nullptr;
static void loop(void)
{
    if (false) // should terminate game?
    {
        async::terminate();

        s_sim_ptr->terminate();
        s_sim_ptr.reset();

        osi::close();

        emscripten_cancel_main_loop();
        return;
    }

    osi::start_round();

    s_sim_ptr->round();

    osi::finish_round();
}
#endif

void  run(std::unique_ptr<simulator>  s)
{
    if (s == nullptr)
        return;

#if BUILD_RELEASE() == 1
    try
#endif
    {
        osi::open();
        if (!osi::is_open())
            return;

        s->initialise();

#if PLATFORM() == PLATFORM_WEBASSEMBLY()
        s_sim_ptr = std::move(s);
        emscripten_set_main_loop(&loop, 0 , 0);
#else
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
#endif
    }
#if BUILD_RELEASE() == 1
    catch (std::exception const&  e)
    {
        s->terminate();
        s.reset();

        osi::close();

        throw e;
    }
#endif
}


}
