#include <qtgl/real_time_simulator.hpp>
#include <qtgl/window.hpp>

namespace qtgl { namespace detail {


extern qtgl::detail::window const&  current_window_cref();


}}

namespace qtgl {


void  real_time_simulator::call_listeners(std::string const&  notification_type) const
{
    detail::current_window_cref().call_listeners(notification_type);
}

qtgl::window_props const&  real_time_simulator::window_props() const
{
    return detail::current_window_cref().window_props();
}

qtgl::mouse_props const&  real_time_simulator::mouse_props() const
{
    return detail::current_window_cref().mouse_props();
}

qtgl::keyboard_props const&  real_time_simulator::keyboard_props() const
{
    return detail::current_window_cref().keyboard_props();
}


natural_64_bit  real_time_simulator::round_id() const
{
    return detail::current_window_cref().round_id();
}

std::chrono::high_resolution_clock::time_point  real_time_simulator::start_time() const
{
    return detail::current_window_cref().start_time();
}

natural_32_bit  real_time_simulator::FPS() const
{
    return detail::current_window_cref().FPS();
}


}
