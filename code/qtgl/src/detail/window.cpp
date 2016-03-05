#include <qtgl/detail/window.hpp>
#include <qtgl/detail/resource_loader.hpp>
#include <qtgl/detail/texture_cache.hpp>
#include <utility/tensor_math.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <QCoreApplication>
#include <QOpenGLPaintDevice>
#include <QEvent>
#include <QTimerEvent>
#include <QScreen>
#include <iostream>
#include <mutex>

namespace qtgl { namespace detail { namespace {


natural_32_bit  s_windows_counter = 0U;
std::mutex  s_windows_counter_mutex;


void  on_window_create()
{
    std::lock_guard<std::mutex> const  lock(s_windows_counter_mutex);
    ++s_windows_counter;
    ASSUMPTION(s_windows_counter != 0U);
}

void  on_window_destroy()
{
    std::lock_guard<std::mutex> const  lock(s_windows_counter_mutex);
    ASSUMPTION(s_windows_counter > 0U);
    --s_windows_counter;
    if (s_windows_counter == 0)
    {
        detail::resource_loader::instance().clear();
        detail::texture_cache::instance().clear(true);
    }
}


}}}

namespace qtgl { namespace detail {


window::window(std::function<std::shared_ptr<real_time_simulator>()> const  create_simulator_fn)
    : QWindow((QWindow*)nullptr)

    , m_create_simulator_fn(create_simulator_fn)
    , m_simulator()

    , m_simulator_calls()
    , m_notification_listeners()

    , m_context()

    , m_round_id(0ULL)
    , m_start_time(std::chrono::high_resolution_clock::now())
    , m_time_of_last_simulation_round(m_start_time)

    , m_FPS_num_rounds(0U)
    , m_FPS_time(0.0L)
    , m_FPS(0U)

    , m_has_focus(false)
    , m_idleTimerId(-1)

    , m_window_props(1U,1U,1.0f,1.0f,false,false)
    , m_mouse_props(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,{})
    , m_keyboard_props()

    , m_is_mouse_set(false)
    , m_mouse_x(0.0f)
    , m_mouse_y(0.0f)
    , m_mouse_previous_x(0.0f)
    , m_mouse_previous_y(0.0f)
    , m_mouse_wheel_delta_x(0.0f)
    , m_mouse_wheel_delta_y(0.0f)
    , m_mouse_lbutton_down(false)
    , m_mouse_lbutton_just_pressed(false)
    , m_mouse_lbutton_just_released(false)
    , m_mouse_rbutton_down(false)
    , m_mouse_rbutton_just_pressed(false)
    , m_mouse_rbutton_just_released(false)
    , m_mouse_mbutton_down(false)
    , m_mouse_mbutton_just_pressed(false)
    , m_mouse_mbutton_just_released(false)
{
    TMPROF_BLOCK();

    LOG(debug,"qtgl::window::window(...)");

    QSurfaceFormat  format;
    format.setMajorVersion(opengl_major_version());
    format.setMinorVersion(opengl_minor_version());
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
    format.setSwapInterval(0);
    format.setDepthBufferSize( 24 );
    format.setSamples(0);

    setSurfaceType(QWindow::OpenGLSurface);
    setFormat(format);

    m_idleTimerId = startTimer(0);

    on_window_create();
}

window::~window()
{
    TMPROF_BLOCK();

    LOG(debug,"qtgl::window::~window()");

    make_current_window_guard const  make_current_window{this};
    m_simulator.reset();
    on_window_destroy();

    std::chrono::high_resolution_clock::time_point const  current_time = std::chrono::high_resolution_clock::now();
    float_64_bit const  time_delta =
            std::chrono::duration<float_64_bit>(current_time - m_start_time).count();

    LOG(testing,"Num steps: " << m_round_id << ", Elapsed time: " << time_delta << "sec, FPS: " << m_round_id / time_delta);
    std::cout << "Num steps   : " << m_round_id << "\n";
    std::cout << "Elapsed time: " << time_delta << " sec\n";
    std::cout << "FPS         : " << m_round_id / time_delta << "\n";
}

opengl_context&  window::glcontext()
{
    ASSUMPTION(m_context.operator bool());
    return *m_context;
}

opengl_context const&  window::glcontext() const
{
    ASSUMPTION(m_context.operator bool());
    return *m_context;
}

void  window::call_listeners(std::string const& notification_type) const
{
    TMPROF_BLOCK();

    auto const  it = m_notification_listeners.find(notification_type);
    if (it != m_notification_listeners.end())
        for (auto& listener : it->second)
            listener.listener_function()();
}

void window::register_listener(std::string const&  notification_type, notification_listener const&  listener)
{
    TMPROF_BLOCK();

    auto const  it = m_notification_listeners.find(notification_type);
    if (it == m_notification_listeners.end())
        m_notification_listeners.insert({notification_type,{listener}});
    else if (std::find(it->second.begin(),it->second.end(),listener) == it->second.end())
        it->second.push_back(listener);
}

void window::unregister_listener(std::string const&  notification_type, notification_listener const&  listener)
{
    TMPROF_BLOCK();

    auto const  it = m_notification_listeners.find(notification_type);
    if (it == m_notification_listeners.end())
        return;
    auto const  elem_it = std::find(it->second.begin(),it->second.end(),listener);
    if (elem_it == it->second.end())
        return;
    it->second.erase(elem_it);
}

void window::render_now(bool const  is_this_pure_redraw_request)
{
    TMPROF_BLOCK();

    if (!isExposed())
        return;

    bool needsInitialize = false;

    if (!m_context) {
        m_context = std::shared_ptr<opengl_context>(new QOpenGLContext(this));
        ASSUMPTION(m_context.operator bool());
        m_context->setFormat(requestedFormat());
        m_context->create();

        needsInitialize = true;
    }

    detail::make_current_window_guard const  make_current_window{this};

    float_32_bit const  inch_size_in_milimeters = 25.4f;
    m_window_props = qtgl::window_props {
            (natural_32_bit)(width() * devicePixelRatio()),
            (natural_32_bit)(height() * devicePixelRatio()),
            (float_32_bit)(inch_size_in_milimeters / screen()->physicalDotsPerInchX()),
            (float_32_bit)(inch_size_in_milimeters / screen()->physicalDotsPerInchY()),
            isActive(),
            m_has_focus
    };

    m_keyboard_props = qtgl::keyboard_props {
    };

    m_mouse_props = qtgl::mouse_props{
            m_mouse_x,
            m_mouse_y,
            m_mouse_x - m_mouse_previous_x,
            m_mouse_y - m_mouse_previous_y,
            m_mouse_wheel_delta_x,
            m_mouse_wheel_delta_y,
            {
                std::make_tuple(LEFT_MOUSE_BUTTON(),
                                m_mouse_lbutton_down,
                                m_mouse_lbutton_just_pressed,
                                m_mouse_lbutton_just_released),
                std::make_tuple(RIGHT_MOUSE_BUTTON(),
                                m_mouse_rbutton_down,
                                m_mouse_rbutton_just_pressed,
                                m_mouse_rbutton_just_released),
                std::make_tuple(MIDDLE_MOUSE_BUTTON(),
                                m_mouse_mbutton_down,
                                m_mouse_mbutton_just_pressed,
                                m_mouse_mbutton_just_released),
            }
    };

    if (needsInitialize) {
        glapi().initializeOpenGLFunctions();
        glapi().glEnable(GL_DEPTH_TEST);
        glapi().glEnable(GL_CULL_FACE);
        glapi().glCullFace(GL_BACK);
        glapi().glEnable(GL_BLEND);
        glapi().glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glapi().glDepthRangef(0.0f,1.0f);
        INVARIANT(!m_simulator.operator bool());
        m_simulator = m_create_simulator_fn();
        INVARIANT(m_simulator.operator bool());
    }

    ++m_round_id;
    std::chrono::high_resolution_clock::time_point const  current_time = std::chrono::high_resolution_clock::now();
    float_64_bit const  time_delta =
            std::chrono::duration<float_64_bit>(current_time - m_time_of_last_simulation_round).count();
    m_time_of_last_simulation_round = current_time;

    for (auto& caller : m_simulator_calls)
        caller(m_simulator.get());
    m_simulator_calls.clear();
    INVARIANT(m_simulator.operator bool());
    m_simulator->next_round(time_delta,is_this_pure_redraw_request);

    ++m_FPS_num_rounds;
    m_FPS_time += time_delta;
    if (m_FPS_time >= 0.25L)
    {
        m_FPS = 4U * m_FPS_num_rounds;
        m_FPS_num_rounds = 0U;
        m_FPS_time -= 0.25L;
        call_listeners(notifications::fps_updated());
    }

    m_mouse_previous_x = m_mouse_x;
    m_mouse_previous_y = m_mouse_y;
    m_mouse_lbutton_just_pressed = false;
    m_mouse_lbutton_just_released = false;
    m_mouse_rbutton_just_pressed = false;
    m_mouse_rbutton_just_released = false;
    m_mouse_mbutton_just_pressed = false;
    m_mouse_mbutton_just_released = false;
}

bool window::event(QEvent* const event)
{
    switch (event->type())
    {
        case QEvent::UpdateRequest:
            render_now(true);
            return true;
        case QEvent::FocusIn:
            m_has_focus = true;
            return QWindow::event(event);
        case QEvent::FocusOut:
            m_has_focus = false;
            return QWindow::event(event);
        default:
            return QWindow::event(event);
    }
}

void window::exposeEvent(QExposeEvent* const event)
{
    Q_UNUSED(event);
    render_now(true);
}

void window::timerEvent(QTimerEvent* const event)
{
    if (event->timerId() == m_idleTimerId)
        render_now(false);
}

void window::keyPressEvent(QKeyEvent* const event)
{
//    if (!event->isAutoRepeat())
//    {
        //std::cout << "opengl_window::keyPressEvent(" << event->key() << ")\n";
        event->accept();
//    }
}

void window::keyReleaseEvent(QKeyEvent* const event)
{
//    if (!event->isAutoRepeat())
//    {
        //std::cout << "opengl_window::keyReleaseEvent(" << event->key() << ")\n";
        event->accept();
//    }
}

void window::mouseMoveEvent(QMouseEvent* const event)
{
    m_mouse_x = event->windowPos().x();
    m_mouse_y = event->windowPos().y();
    if (!m_is_mouse_set)
    {
        m_is_mouse_set = true;
        m_mouse_previous_x = m_mouse_x;
        m_mouse_previous_y = m_mouse_y;
    }
    event->accept();
}

void window::mousePressEvent(QMouseEvent* const event)
{
    switch (event->button())
    {
        case Qt::LeftButton:
            m_mouse_lbutton_down = true;
            m_mouse_lbutton_just_pressed = true;
            break;
        case Qt::RightButton:
            m_mouse_rbutton_down = true;
            m_mouse_rbutton_just_pressed = true;
            break;
        case Qt::MidButton:
            m_mouse_mbutton_down = true;
            m_mouse_mbutton_just_pressed = true;
            break;
        default:
            return;
    }
    event->accept();
}

void window::mouseReleaseEvent(QMouseEvent* const event)
{
    switch (event->button())
    {
        case Qt::LeftButton:
            m_mouse_lbutton_down = false;
            m_mouse_lbutton_just_released = true;
            break;
        case Qt::RightButton:
            m_mouse_rbutton_down = false;
            m_mouse_rbutton_just_released = true;
            break;
        case Qt::MidButton:
            m_mouse_mbutton_down = false;
            m_mouse_mbutton_just_released = true;
            break;
        default:
            return;
    }
    event->accept();
}

void window::wheelEvent(QWheelEvent* const event)
{
    m_mouse_wheel_delta_x = (PI() * (float_32_bit)event->angleDelta().x()) / (8.0f * 180.0f);
    m_mouse_wheel_delta_y = (PI() * (float_32_bit)event->angleDelta().y()) / (8.0f * 180.0f);
    event->accept();
}


}}
