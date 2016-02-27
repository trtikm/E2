#include <qtgl/notification_listener.hpp>

namespace qtgl {

std::string  make_listener_function_id(void (*listener_function)())
{
    std::ostringstream ostr;
    ostr << std::hex << (void*)listener_function;
    return ostr.str();
}


}
