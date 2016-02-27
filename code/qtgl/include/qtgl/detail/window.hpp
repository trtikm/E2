#ifndef QTGL_DETAIL_WINDOW_HPP_INCLUDED
#   define QTGL_DETAIL_WINDOW_HPP_INCLUDED

#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/glapi.hpp>
#   include <qtgl/window_props.hpp>
#   include <qtgl/keyboard_props.hpp>
#   include <qtgl/mouse_props.hpp>
#   include <qtgl/notification_listener.hpp>
#   include <QWindow>
#   include <QKeyEvent>
#   include <QMouseEvent>
#   include <QWheelEvent>
#   include <chrono>
#   include <memory>
#   include <functional>
#   include <unordered_map>

namespace qtgl { namespace detail {


struct window;

extern void  make_current_window(qtgl::detail::window* const  window);
extern qtgl::detail::window*  current_window();

struct make_current_window_guard
{
    make_current_window_guard(qtgl::detail::window* const  window,
                              qtgl::detail::window* const  restore = nullptr)
        : m_restore(restore)
    {
        qtgl::detail::make_current_window(window);
    }
    ~make_current_window_guard()
    {
        qtgl::detail::make_current_window(m_restore);
    }
private:
    qtgl::detail::window*  m_restore;
};


struct window : public QWindow
{
    window(std::function<std::shared_ptr<real_time_simulator>()> const  create_simulator_fn);
    ~window();

    std::shared_ptr<real_time_simulator>  simulator() { return m_simulator; }

    template<typename return_type>
    return_type  call_now(std::function<return_type()> const& fn)
    {
        make_current_window_guard const  make_current_window{this,current_window()};
        return static_cast<return_type>(fn());
    }

    void  call_later(std::function<void(real_time_simulator*)> const& fn) { m_simulator_calls.push_back(fn); }

    void register_listener(std::string const&  notification_type, notification_listener const&  listener);
    void unregister_listener(std::string const&  notification_type, notification_listener const&  listener);

    void  call_listeners(std::string const&  notification_type) const;

    opengl_context&  glcontext();
    opengl_context const&  glcontext() const;

    natural_64_bit  round_id() const { return m_round_id; }
    std::chrono::high_resolution_clock::time_point  start_time() const { return m_start_time; }
    natural_32_bit  FPS() const { return m_FPS; }

    qtgl::window_props const&  window_props() const { return m_window_props; }
    qtgl::mouse_props const&  mouse_props() const { return m_mouse_props; }
    qtgl::keyboard_props const&  keyboard_props() const { return m_keyboard_props; }

private:

    void render_now(bool const  is_this_pure_redraw_request);

    bool event(QEvent* const event);
    void exposeEvent(QExposeEvent* const event);
    void timerEvent(QTimerEvent* const event);

    void keyPressEvent(QKeyEvent* const event);
    void keyReleaseEvent(QKeyEvent* const event);

    void mouseMoveEvent(QMouseEvent* const event);
    void mousePressEvent(QMouseEvent* const event);
    void mouseReleaseEvent(QMouseEvent* const event);
    void wheelEvent(QWheelEvent* const event);

    Q_OBJECT

    std::function<std::shared_ptr<real_time_simulator>()>  m_create_simulator_fn;
    std::shared_ptr<real_time_simulator>  m_simulator;

    std::vector< std::function<void(real_time_simulator*)> >  m_simulator_calls;
    std::unordered_map< std::string, std::vector<notification_listener> >  m_notification_listeners;

    std::shared_ptr<opengl_context>  m_context;

    natural_64_bit  m_round_id;
    std::chrono::high_resolution_clock::time_point  m_start_time;
    std::chrono::high_resolution_clock::time_point  m_time_of_last_simulation_round;

    natural_32_bit  m_FPS_num_rounds;
    float_64_bit  m_FPS_time;
    natural_32_bit  m_FPS;

    bool  m_has_focus;
    int  m_idleTimerId;

    qtgl::window_props  m_window_props;
    qtgl::mouse_props  m_mouse_props;
    qtgl::keyboard_props  m_keyboard_props;

    bool  m_is_mouse_set;
    float_32_bit  m_mouse_x;
    float_32_bit  m_mouse_y;
    float_32_bit  m_mouse_previous_x;
    float_32_bit  m_mouse_previous_y;
    float_32_bit  m_mouse_wheel_delta_x;
    float_32_bit  m_mouse_wheel_delta_y;
    bool  m_mouse_lbutton_down;
    bool  m_mouse_lbutton_just_pressed;
    bool  m_mouse_lbutton_just_released;
    bool  m_mouse_rbutton_down;
    bool  m_mouse_rbutton_just_pressed;
    bool  m_mouse_rbutton_just_released;
    bool  m_mouse_mbutton_down;
    bool  m_mouse_mbutton_just_pressed;
    bool  m_mouse_mbutton_just_released;
};


}}

#endif
