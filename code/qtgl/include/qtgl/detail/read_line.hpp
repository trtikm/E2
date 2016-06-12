#ifndef QTGL_DETAIL_READ_LINE_HPP_INCLUDED
#   define QTGL_DETAIL_READ_LINE_HPP_INCLUDED

#   include <iosfwd>
#   include <string>

namespace qtgl { namespace detail {


std::ifstream&  read_line(std::ifstream&  istr, std::string&  line);


}}

#endif
