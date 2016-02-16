#ifndef UTILITY_MSGSTREAM_HPP_INCLUDED
#   define UTILITY_MSGSTREAM_HPP_INCLUDED

#   include <string>
#   include <sstream>


struct  msgstream
{
    template<typename T>
    msgstream&  operator<<(T const& value) { m_stream << value; return *this; }
    std::string  get() const { return m_stream.str(); }
    operator std::string() const { return get(); }
private:
    std::ostringstream  m_stream;
};


#endif