#ifndef QTGL_WINDOW_HPP_INCLUDED
#   define QTGL_WINDOW_HPP_INCLUDED

#   include <qtgl/notification_listener.hpp>
#   include <qtgl/real_time_simulator.hpp>
#   include <qtgl/detail/window.hpp>
#   include <utility/assumptions.hpp>
#   include <boost/function_types/result_type.hpp>
#   include <QWidget>
#   include <functional>
#   include <string>
#   include <memory>

namespace qtgl {


using  make_current_window_guard = std::shared_ptr<detail::make_current_window_guard>;


/**
 * Represents an OpenGL window/widget inside the Qt5 GUI system.
 * Implement your own simulator (i.e. derive from qtgl::real_time_simulator).
 * Then pass construction parameters of your simulator into the constuctor of this window.
 * Your simulator will be then constructed, called, and destroyed in proper times.
 * This strategy was used to avoid the non-standard Qt initialisation/termination pipeline.
 * The simulator is always guaranteed to have full access to OpenGL library (through qtgl::glapi()
 * function) and OpenGL context associated with the window (the context is also set 'current').
 */
template<typename simulator_type__>
struct window
{
    typedef simulator_type__  simulator_type;

    static_assert(std::is_base_of<qtgl::real_time_simulator,simulator_type>::value,"");

    template<typename... arg_types>
    window(arg_types... args_for_constructor_of_simulator)
        : m_window()
    {
        m_window = std::shared_ptr<detail::window>( new detail::window(
                        std::bind(
                            [](arg_types... args) {
                                return std::shared_ptr<real_time_simulator>(new simulator_type(args...));
                                },
                            args_for_constructor_of_simulator...
                            )
                        ));
    }

    QWidget* create_widget_container() const { return QWidget::createWindowContainer(static_cast<QWindow*>(m_window.get())); }

    template<typename fn_type, typename... arg_types>
    typename boost::function_types::result_type<fn_type>::type  call_now(fn_type  fn, arg_types...  args)
    {
        typedef typename boost::function_types::result_type<fn_type>::type  return_type;
        return static_cast<return_type>(m_window->call_now<return_type>(std::bind(fn,simulator_ptr(),args...)));
    }

    template<typename fn_type, typename... arg_types>
    void  call_later(fn_type  fn, arg_types...  args)
    {
        m_window->call_later(
                std::bind(
                    [fn](real_time_simulator* const  ptr, arg_types...  args){
                        (dynamic_cast<simulator_type*>(ptr)->*fn)(args...);
                        },
                    std::placeholders::_1,
                    args...
                    )
                );
    }

    void register_listener(std::string const&  notification_type, notification_listener const&  listener)
    {
        m_window->register_listener(notification_type,listener);
    }

    void unregister_listener(std::string const&  notification_type, notification_listener const&  listener)
    {
        m_window->unregister_listener(notification_type,listener);
    }

    std::shared_ptr<detail::make_current_window_guard>   make_me_current() { return m_window->make_me_current(); }

    QWindow& qtbase() { return *m_window.get(); }

    bool  has_simulator() const { return m_window->simulator() != nullptr; }

private:

    simulator_type*  simulator_ptr()
    {
        simulator_type* const  ptr = std::dynamic_pointer_cast<simulator_type>(m_window->simulator()).get();
        ASSUMPTION(ptr != nullptr);
        return ptr;
    }

    std::shared_ptr<detail::window>  m_window;
};


}

#endif
