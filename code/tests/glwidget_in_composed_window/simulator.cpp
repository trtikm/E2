#include "./simulator.hpp"
#include <qtgl/glapi.hpp>
#include <utility/random.hpp>
#include <utility/timeprof.hpp>

simulator::simulator()
    : qtgl::real_time_simulator()
{
    qtgl::glapi().glClearColor(0.25f, 0.25f, 0.25f, 1.0f);
}

simulator::~simulator()
{
}

void simulator::next_round(float_64_bit const  miliseconds_from_previous_call,
                           bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    qtgl::glapi().glClearColor(get_random_natural_32_bit_in_range(0U,1000U)/1000.0f,
                               get_random_natural_32_bit_in_range(0U,1000U)/1000.0f,
                               get_random_natural_32_bit_in_range(0U,1000U)/1000.0f,
                               1.0f);
    qtgl::glapi().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    qtgl::glapi().glViewport(0, 0, window_props().width_in_pixels(), window_props().height_in_pixels());

    qtgl::swap_buffers();
}
