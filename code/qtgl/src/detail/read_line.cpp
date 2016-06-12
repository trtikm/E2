#include <qtgl/detail/read_line.hpp>
#include <fstream>

namespace qtgl { namespace detail {


std::ifstream&  read_line(std::ifstream&  istr, std::string&  line)
{
    while (std::getline(istr,line))
        if (!line.empty())
        {
            if (line.back() == '\r')
                line.pop_back();
            break;
        }
    return istr;
}


}}
