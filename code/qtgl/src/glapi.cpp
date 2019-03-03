#include <qtgl/glapi.hpp>
#include <qtgl/detail/window.hpp>
#include <utility/assumptions.hpp>
#include <utility/timeprof.hpp>

namespace qtgl { namespace detail { namespace {


static qtgl::detail::window*  s_current_window = nullptr;


void  set_current_window(qtgl::detail::window* const  window)
{
    s_current_window = window;
}

opengl_context*  current_context()
{
    ASSUMPTION(current_window() != nullptr);
    return &current_window()->glcontext();
}


}}}

namespace qtgl { namespace detail {


void  make_current_window(qtgl::detail::window* const  window)
{
    if (current_window() != window)
    {
        set_current_window(window);
        if (current_window() != nullptr)
            current_context()->makeCurrent(current_window());
    }
}

qtgl::detail::window*  current_window()
{
    return s_current_window;
}

qtgl::detail::window const&  current_window_cref()
{
    ASSUMPTION(current_window() != nullptr);
    return *current_window();
}


}}

namespace qtgl {


opengl_api&  glapi()
{
    opengl_api* const  api = dynamic_cast<opengl_api*>(detail::current_context()->versionFunctions());
    ASSUMPTION(api != nullptr);
    return *api;
}

void  swap_buffers()
{
    TMPROF_BLOCK();

    detail::current_context()->swapBuffers(detail::current_window());
}


}
