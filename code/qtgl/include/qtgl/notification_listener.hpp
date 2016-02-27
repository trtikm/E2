#ifndef QTGL_NOTIFICATION_LISTENER_HPP_INCLUDED
#   define QTGL_NOTIFICATION_LISTENER_HPP_INCLUDED

#   include <functional>
#   include <string>
#   include <sstream>
#   include <array>

namespace qtgl {


std::string  make_listener_function_id(void (*listener_function)());

template<typename object_type>
std::string  make_listener_method_id(void (object_type::* method)(), object_type* const object);


struct notification_listener
{
    notification_listener(std::string const&  unique_id, std::function<void()> const&  listener_function)
        : m_unique_id(unique_id)
        , m_listener_function(listener_function)
    {}
    notification_listener(void (*listener_function)())
        : m_unique_id(make_listener_function_id(listener_function))
        , m_listener_function(listener_function)
    {}
    template<typename object_type>
    notification_listener(void (object_type::* method)(), object_type* const object)
        : m_unique_id(make_listener_method_id(method,object))
        , m_listener_function(std::bind(method,object))
    {}
    std::string  unique_id() const { return m_unique_id; }
    std::function<void()>  listener_function() const { return m_listener_function; }
private:
    std::string  m_unique_id;
    std::function<void()>  m_listener_function;
};

inline bool  operator==(notification_listener const& l, notification_listener const& r)
{
    return l.unique_id() == r.unique_id();
}


template<typename object_type>
std::string  make_listener_method_id(void (object_type::* method)(), object_type* const object)
{
    std::ostringstream ostr;

    ostr << std::hex << (void*)object << "::";

    union PtrUnion
    {
        void (object_type::* f)();
        std::array<unsigned char, sizeof(method)> buf;
    };
    PtrUnion u;
    u.f = method;
    for (auto c : u.buf)
        ostr << std::hex << (natural_32_bit)c;

    return ostr.str();
}


}

#endif
