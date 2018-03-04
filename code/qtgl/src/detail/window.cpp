#include <qtgl/detail/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <angeo/tensor_math.hpp>
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
}


std::unordered_map<int,keyboard_key_name> const&  qtkey_to_keyname()
{
    static std::unordered_map<int,keyboard_key_name> const  ktn {
        { Qt::Key_Escape,       KEY_ESCAPE() },
        { Qt::Key_Tab,          KEY_TAB() },
        { Qt::Key_Backspace,    KEY_BACKSPACE() },
        { Qt::Key_Return,       KEY_RETURN() },
        { Qt::Key_Enter,        KEY_RETURN() },
        { Qt::Key_Insert,       KEY_INSERT() },
        { Qt::Key_Delete,       KEY_DELETE() },
        { Qt::Key_Pause,        KEY_PAUSE() },
        { Qt::Key_Home,         KEY_HOME() },
        { Qt::Key_End,          KEY_END() },
        { Qt::Key_Left,         KEY_LEFT() },
        { Qt::Key_Up,           KEY_UP() },
        { Qt::Key_Right,        KEY_RIGHT() },
        { Qt::Key_Down,         KEY_DOWM() },
        { Qt::Key_PageUp,       KEY_PAGEUP() },
        { Qt::Key_PageDown,     KEY_PAGEDOWN() },
        { Qt::Key_CapsLock,     KEY_CAPSLOCK() },
        { Qt::Key_Shift,        KEY_LSHIFT() },
        { Qt::Key_Control,      KEY_LCTRL() },
        { Qt::Key_Alt,          KEY_LALT() },
        { Qt::Key_NumLock,      KEY_NUMLOCK() },
        { Qt::Key_F1,           KEY_F1() },
        { Qt::Key_F2,           KEY_F2() },
        { Qt::Key_F3,           KEY_F3() },
        { Qt::Key_F4,           KEY_F4() },
        { Qt::Key_F5,           KEY_F5() },
        { Qt::Key_F6,           KEY_F6() },
        { Qt::Key_F7,           KEY_F7() },
        { Qt::Key_F8,           KEY_F8() },
        { Qt::Key_F9,           KEY_F9() },
        { Qt::Key_F10,          KEY_F10() },
        { Qt::Key_F11,          KEY_F11() },
        { Qt::Key_F12,          KEY_F12() },
        { Qt::Key_Space,        KEY_SPACE() },
        { Qt::Key_Comma,        KEY_COMMA() },
        { Qt::Key_Period,       KEY_DOT() },
        { Qt::Key_Slash,        KEY_SLASH() },
        { Qt::Key_0,            KEY_0() },
        { Qt::Key_1,            KEY_1() },
        { Qt::Key_2,            KEY_2() },
        { Qt::Key_3,            KEY_3() },
        { Qt::Key_4,            KEY_4() },
        { Qt::Key_5,            KEY_5() },
        { Qt::Key_6,            KEY_6() },
        { Qt::Key_7,            KEY_7() },
        { Qt::Key_8,            KEY_8() },
        { Qt::Key_9,            KEY_9() },
        { Qt::Key_Semicolon,    KEY_SEMICOLON() },
        { Qt::Key_A,            KEY_A() },
        { Qt::Key_B,            KEY_B() },
        { Qt::Key_C,            KEY_C() },
        { Qt::Key_D,            KEY_D() },
        { Qt::Key_E,            KEY_E() },
        { Qt::Key_F,            KEY_F() },
        { Qt::Key_G,            KEY_G() },
        { Qt::Key_H,            KEY_H() },
        { Qt::Key_I,            KEY_I() },
        { Qt::Key_J,            KEY_J() },
        { Qt::Key_K,            KEY_K() },
        { Qt::Key_L,            KEY_L() },
        { Qt::Key_M,            KEY_M() },
        { Qt::Key_N,            KEY_N() },
        { Qt::Key_O,            KEY_O() },
        { Qt::Key_P,            KEY_P() },
        { Qt::Key_Q,            KEY_Q() },
        { Qt::Key_R,            KEY_R() },
        { Qt::Key_S,            KEY_S() },
        { Qt::Key_T,            KEY_T() },
        { Qt::Key_U,            KEY_U() },
        { Qt::Key_V,            KEY_V() },
        { Qt::Key_W,            KEY_W() },
        { Qt::Key_X,            KEY_X() },
        { Qt::Key_Y,            KEY_Y() },
        { Qt::Key_Z,            KEY_Z() },
        { Qt::Key_Backslash,    KEY_BACKSLASH() },
        { Qt::Key_Underscore,   KEY_UNDERSCORE() },
    };
    return ktn;
}


}}}

namespace qtgl { namespace detail {


window::window(std::function<std::shared_ptr<real_time_simulator>()> const  create_simulator_fn,
               bool const  enable_gl_debug_mode)
    : QWindow((QWindow*)nullptr)

    , m_create_simulator_fn(create_simulator_fn)
    , m_simulator()

    , m_simulator_calls()
    , m_notification_listeners()

    , m_context()
    , m_is_gl_debug_mode_enabled(enable_gl_debug_mode)
    , m_gl_logger()

    , m_round_id(0ULL)
    , m_start_time(std::chrono::high_resolution_clock::now())
    , m_time_of_last_simulation_round(m_start_time)
    , m_total_simulation_time(0.0)

    , m_FPS_num_rounds(0U)
    , m_FPS_time(0.0L)
    , m_FPS(0U)

    , m_initialised(false)

    , m_just_resized(false)
    , m_just_focus_changed(false)
    , m_has_focus(false)
    , m_idleTimerId(-1)

    , m_window_props(1U,1U,1.0f,1.0f,false,false,false)
    , m_mouse_props(0.0f,0.0f,0.0f,0.0f,0.0f,0.0f,{})
    , m_keyboard_props()

    , m_keyboard_text()
    , m_keyboard_pressed()
    , m_keyboard_just_pressed()
    , m_keyboard_just_released()

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
    if (m_is_gl_debug_mode_enabled)
        format.setOption(QSurfaceFormat::DebugContext);
    format.setSwapInterval(0);
    format.setDepthBufferSize( 24 );
    format.setSamples(0);

    m_context = std::shared_ptr<opengl_context>(new QOpenGLContext(this));
    ASSUMPTION(m_context.operator bool());
    m_context->setFormat(format);
    m_context->create();

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

//    if (!m_context) {
//        m_context = std::shared_ptr<opengl_context>(new QOpenGLContext(this));
//        ASSUMPTION(m_context.operator bool());
//        m_context->setFormat(requestedFormat());
//        m_context->create();

//        needsInitialize = true;
//    }

    detail::make_current_window_guard const  make_current_window{this};

    float_32_bit const  inch_size_in_milimeters = 25.4f;
    m_window_props = qtgl::window_props {
            (natural_32_bit)(width() * devicePixelRatio()),
            (natural_32_bit)(height() * devicePixelRatio()),
            (float_32_bit)(inch_size_in_milimeters / screen()->physicalDotsPerInchX()),
            (float_32_bit)(inch_size_in_milimeters / screen()->physicalDotsPerInchY()),
            isActive(),
            m_has_focus,
            m_just_resized
    };

    if (m_just_focus_changed)
    {
        m_keyboard_text.clear();
        m_keyboard_pressed.clear();
        m_keyboard_just_pressed.clear();
        m_keyboard_just_released.clear();
        
        m_is_mouse_set = false;
        m_mouse_x = 0.0f;
        m_mouse_y = 0.0f;
        m_mouse_previous_x = m_mouse_x;
        m_mouse_previous_y = m_mouse_y;
        m_mouse_wheel_delta_x = 0.0f;
        m_mouse_wheel_delta_y = 0.0f;
        m_mouse_lbutton_down = false;
        m_mouse_lbutton_just_pressed = false;
        m_mouse_lbutton_just_released = false;
        m_mouse_rbutton_down = false;
        m_mouse_rbutton_just_pressed = false;
        m_mouse_rbutton_just_released = false;
        m_mouse_mbutton_down = false;
        m_mouse_mbutton_just_pressed = false;
        m_mouse_mbutton_just_released = false;
    }

    m_keyboard_props = qtgl::keyboard_props {
            m_keyboard_text,
            m_keyboard_pressed,
            m_keyboard_just_pressed,
            m_keyboard_just_released
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

    if (!m_initialised) {

        //glapi().initializeOpenGLFunctions();

        if (m_is_gl_debug_mode_enabled
            && !m_gl_logger.operator bool()
            && m_context->hasExtension(QByteArrayLiteral("GL_KHR_debug")))
        {
            m_gl_logger = std::make_shared<QOpenGLDebugLogger>(this);
            ASSUMPTION(m_gl_logger.operator bool());
            m_gl_logger->initialize();
        }

        glapi().glEnable(GL_DEPTH_TEST);
        glapi().glEnable(GL_CULL_FACE);
        glapi().glCullFace(GL_BACK);
        glapi().glDisable(GL_BLEND);
        glapi().glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
        glapi().glDepthRangef(0.0f,1.0f);

        INVARIANT(!m_simulator.operator bool());
        m_simulator = m_create_simulator_fn();
        INVARIANT(m_simulator.operator bool());

        m_initialised = true;

        for (auto const  &notification_listeners : m_notification_listeners)
            for (auto& listener : notification_listeners.second)
                listener.listener_function()();
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

    m_total_simulation_time += time_delta;

    ++m_FPS_num_rounds;
    m_FPS_time += time_delta;
    if (m_FPS_time >= 0.25L)
    {
        m_FPS = 4U * m_FPS_num_rounds;
        m_FPS_num_rounds = 0U;
        m_FPS_time -= 0.25L;
        call_listeners(notifications::fps_updated());
    }

    m_just_resized = false;
    m_just_focus_changed = false;

    m_keyboard_text.clear();
    m_keyboard_just_pressed.clear();
    m_keyboard_just_released.clear();

    m_mouse_previous_x = m_mouse_x;
    m_mouse_previous_y = m_mouse_y;
    m_mouse_lbutton_just_pressed = false;
    m_mouse_lbutton_just_released = false;
    m_mouse_rbutton_just_pressed = false;
    m_mouse_rbutton_just_released = false;
    m_mouse_mbutton_just_pressed = false;
    m_mouse_mbutton_just_released = false;

    if (m_gl_logger.operator bool())
    {
        const QList<QOpenGLDebugMessage> messages = m_gl_logger->loggedMessages();
        for (const QOpenGLDebugMessage &message : messages)
            LOG(error,qtgl::to_string(message.message()));
    }
}

bool window::event(QEvent* const event)
{
    switch (event->type())
    {
    case QEvent::Resize:
            m_just_resized = true;
            return QWindow::event(event);
    case QEvent::UpdateRequest:
            render_now(true);
            return true;
        case QEvent::FocusIn:
            m_has_focus = true;
            m_just_focus_changed = true;
            return QWindow::event(event);
        case QEvent::FocusOut:
            m_has_focus = false;
            m_just_focus_changed = true;
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
    auto const  it = qtkey_to_keyname().find(event->key());
    if (it != qtkey_to_keyname().cend())
    {
        m_keyboard_pressed.insert(it->second);
        if (!event->isAutoRepeat())
            m_keyboard_just_pressed.insert(it->second);
    }
//    else
//        std::cout << "opengl_window::keyPressEvent(" << event->key() << ")\n";
    m_keyboard_text.push_back(to_string(event->text()));
    event->accept();
}

void window::keyReleaseEvent(QKeyEvent* const event)
{
    auto const  it = qtkey_to_keyname().find(event->key());
    if (it != qtkey_to_keyname().cend())
    {
        m_keyboard_pressed.erase(it->second);
        m_keyboard_just_released.insert(it->second);
    }
//    else
//        std::cout << "opengl_window::keyReleaseEvent(" << event->key() << ")\n";
    event->accept();
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
