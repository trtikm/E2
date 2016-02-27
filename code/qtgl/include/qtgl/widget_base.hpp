#ifndef QTGL_WIDGET_BASE_HPP_INCLUDED
#   define QTGL_WIDGET_BASE_HPP_INCLUDED

#   include <qtgl/window.hpp>
#   include <qtgl/notification_listener.hpp>
#   include <boost/function_types/result_type.hpp>
#   include <functional>
#   include <string>
#   include <sstream>
#   include <array>

namespace qtgl {


template<typename derived_type__, typename window_type__>
struct widget_base
{
    typedef  derived_type__  derived_type;
    typedef  window_type__   window_type;

    widget_base(window_type* window) : m_window(window) {}
    virtual ~widget_base() {}

    template<typename fn_type, typename... arg_types>
    typename boost::function_types::result_type<fn_type>::type  call_now(fn_type  fn, arg_types...  args)
    {
        return static_cast<typename boost::function_types::result_type<fn_type>::type>(m_window->call_now(fn,args...));
    }

    template<typename fn_type, typename... arg_types>
    void call_later(fn_type  fn, arg_types...  args)
    {
        m_window->call_later(fn,args...);
    }

    void register_listener(std::string const&  notification_type, void (derived_type::* listener)())
    {
        m_window->register_listener(
                    notification_type,
                    notification_listener(
                            make_listener_method_id(listener,dynamic_cast<derived_type*>(this)),
                            [this,listener]() { (dynamic_cast<derived_type*>(this)->*listener)(); }
                            )
                    );
    }

    void unregister_listener(std::string const&  notification_type, void (derived_type::* listener)())
    {
        m_window->unregister_listener(
                    notification_type,
                    notification_listener(
                            make_listener_method_id(listener,dynamic_cast<derived_type*>(this)),
                            [this,listener]() { (dynamic_cast<derived_type*>(this)->*listener)(); }
                            )
                    );
    }

private:
    std::string  make_unique_id(void (derived_type::* listener)());

    window_type*  m_window;
};


}

#endif
