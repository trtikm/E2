#ifndef UTILITY_READ_LINE_HPP_INCLUDED
#   define UTILITY_READ_LINE_HPP_INCLUDED

#   include <iosfwd>
#   include <string>

// A call to any function below leads to reading a next line in the input stream,
// while empty lines (including those with only white characters) are skipped and
// line-termination characters '\n' and '\r' are removed from the result.
// The empty string returned means the read in the end of the stream.
// In case you want to read a raw line from a stream use function 'std::getline' instead.

std::ifstream&  read_line(std::ifstream&  istr, std::string&  line);
std::string  read_line(std::ifstream&  istr);


#endif
