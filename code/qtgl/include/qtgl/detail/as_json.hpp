#ifndef QTGL_DETAIL_AS_JSON_HPP_INCLUDED
#   define QTGL_DETAIL_AS_JSON_HPP_INCLUDED

#   include <qtgl/effects_config.hpp>
#   include <angeo/tensor_math.hpp>
#   include <boost/algorithm/string.hpp>
#   include <string>
#   include <sstream>

namespace qtgl { namespace detail {


template<typename T>
inline std::string  as_json(T const  value)
{
    return std::to_string(value);
}


inline std::string  as_json(std::string const&  key)
{
    return "\"" + boost::replace_all_copy(boost::replace_all_copy(key, "\"", "\\\""), "\n", " ") + "\"";
}


inline std::string  as_json(char const* const  key)
{
    return as_json(std::string(key));
}


inline std::string  as_json(vector3 const&  v)
{
    return "[" + std::to_string(v(0)) + "," + std::to_string(v(1)) + "," + std::to_string(v(2)) + "]";
}


inline std::string  as_json(vector4 const& v)
{
    return "[" + std::to_string(v(0)) + "," + std::to_string(v(1)) + "," + std::to_string(v(2)) + "," + std::to_string(v(3)) + "]";
}


inline std::string  as_json(FOG_TYPE const  fog_type)
{
    return name(fog_type);
}


}}

#endif
