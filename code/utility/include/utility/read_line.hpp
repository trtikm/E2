#ifndef UTILITY_READ_LINE_HPP_INCLUDED
#   define UTILITY_READ_LINE_HPP_INCLUDED

#   include <utility/basic_numeric_types.hpp>
#   include <iosfwd>
#   include <string>


struct  per_line_buffer_reader
{
    per_line_buffer_reader(
            natural_8_bit const*  start,
            natural_8_bit const*  end,
            std::string const&  buffer_name,
            natural_64_bit  start_line_number = 1UL
            );

    bool  eof() const { return m_cursor == m_end; }

    std::string  get_next_line(bool const  allow_empty_lines = false, bool const  allow_read_on_eof = false);

private:

    void  goto_end_of_line();
    void  goto_next_line();

    natural_8_bit const*  m_cursor;
    natural_8_bit const*  m_end;
    natural_64_bit  m_line_number;
    std::string  m_buffer_name;
};

bool  is_empty_line(natural_8_bit const*  begin, natural_8_bit const* const  end);
inline bool  is_empty_line(std::string const&  line)
{
    return is_empty_line(reinterpret_cast<natural_8_bit const*>(line.data()),
                         reinterpret_cast<natural_8_bit const*>(line.data()) + line.size());
}


// A call to any function below leads to reading a next line in the input stream,
// while empty lines (including those with only white characters) are skipped and
// line-termination characters '\n' and '\r' are removed from the result.
// The empty string returned means the read in the end of the stream.
// In case you want to read a raw line from a stream use function 'std::getline' instead.

std::ifstream&  read_line(std::ifstream&  istr, std::string&  line);
std::string  read_line(std::ifstream&  istr);


#endif
