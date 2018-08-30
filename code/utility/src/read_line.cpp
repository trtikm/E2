#include <utility/read_line.hpp>
#include <utility/msgstream.hpp>
#include <utility/timeprof.hpp>
#include <utility/assumptions.hpp>
#include <fstream>


per_line_buffer_reader::per_line_buffer_reader(
        natural_8_bit const*  start,
        natural_8_bit const*  end,
        std::string const&  buffer_name,
        natural_64_bit  start_line_number
        )
    : m_cursor(start)
    , m_end(end)
    , m_line_number(start_line_number)
{
    ASSUMPTION(m_cursor < m_end && *(m_end - 1UL) == 0U);
}


std::string  per_line_buffer_reader::get_next_line(bool const  allow_empty_lines, bool const  allow_read_on_eof)
{
    TMPROF_BLOCK();

    goto_next_line();
    if (!allow_read_on_eof && eof())
        throw std::runtime_error(msgstream() << "per_line_buffer_reader::get_next_line() : Attempt to read at EOF "
                                             << " (i.e. below the last line " << m_line_number << ") in the buffer '"
                                             << m_buffer_name << "'.");
    natural_8_bit const* const  begin = m_cursor;
    goto_end_of_line();
    if (!allow_empty_lines && is_empty_line(begin,m_cursor))
        throw std::runtime_error(msgstream() << "per_line_buffer_reader::get_next_line() : The line " << m_line_number
                                             << " in the buffer '" << m_buffer_name << "' is empty.");
    return std::string(begin, m_cursor);
}


void  per_line_buffer_reader::goto_end_of_line()
{
    while (m_cursor != m_end && *m_cursor != '\r' && *m_cursor != '\n')
        ++m_cursor;
}


void  per_line_buffer_reader::goto_next_line()
{
    while (m_cursor != m_end && (*m_cursor == '\r' || *m_cursor == '\n'))
        ++m_cursor;
    if (m_cursor != m_end)
        ++m_line_number;
}


void  per_line_buffer_reader::skip_spaces()
{
    while (m_cursor != m_end && (*m_cursor == ' ' || *m_cursor == '\t'))
        ++m_cursor;
}


bool  is_empty_line(natural_8_bit const*  begin, natural_8_bit const* const  end)
{
    for ( ; begin != end; ++begin)
        if (*begin != ' ' && *begin != '\t')
            return false;
    return true;
}


std::ifstream&  read_line(std::ifstream&  istr, std::string&  line)
{
    TMPROF_BLOCK();
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
