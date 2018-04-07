#include <utility/read_line.hpp>
#include <fstream>


std::ifstream&  read_line(std::ifstream&  istr, std::string&  line)
{
    while (std::getline(istr,line))
        if (!line.empty())
        {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ' || line.back() == '\t'))
                line.pop_back();
            if (!line.empty())
                break;
        }
    return istr;
}


std::string  read_line(std::ifstream&  istr)
{
    std::string  line;
    read_line(istr, line);
    return line;
}
