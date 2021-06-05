#ifndef UTILITY_LOG_HPP_INCLUDED
#   define UTILITY_LOG_HPP_INCLUDED

#   include <utility/config.hpp>
#   include <utility/msgstream.hpp>
#   include <boost/log/sources/record_ostream.hpp>
#   include <boost/log/sources/logger.hpp>
#   include <boost/log/attributes/constant.hpp>
#   include <string>
#   include <deque>

#   define LOG(LVL,MSG) \
        if ((LVL) >= BUILD_RELEASE() * 2 && (LVL) >= get_minimal_severity_level())\
        {\
            boost::log::sources::logger lg;\
            namespace attrs = boost::log::attributes;\
            lg.add_attribute("File",attrs::constant<std::string>(__FILE__));\
            lg.add_attribute("Line",attrs::constant<unsigned int>(__LINE__));\
            lg.add_attribute("Level",attrs::constant<std::string>(::logging_severity_level_name(LVL)));\
            BOOST_LOG(lg) << MSG;\
        }
#   define LOG_INITIALISE(log_file_path_name,add_creation_timestamp_to_filename,add_default_file_extension,\
                          minimal_severity_level)\
        namespace E2 { namespace utility { namespace detail { namespace { namespace {\
            ::logging_setup_caller __logger_setup_caller_instance(log_file_path_name,\
                                                                  add_creation_timestamp_to_filename,\
                                                                  add_default_file_extension,\
                                                                  minimal_severity_level);\
        }}}}}
#   define SLOG(MSG) screen_text_logger::instance().append(msgstream() << MSG)
#   define CLOG(MSG) continuous_text_logger::instance().append(msgstream() << MSG)


enum logging_severity_level
{
    debug = 0,
    info = 1,
    warning = 2,
    error = 3,
    testing = 4,
};
std::string const&  logging_severity_level_name(logging_severity_level const level);
logging_severity_level  get_minimal_severity_level();
void  set_minimal_severity_level(logging_severity_level const level);

struct logging_setup_caller
{
    logging_setup_caller(std::string const& log_file_name = "./log.html",
                         bool const add_creation_timestamp_to_filename = true,
                         bool const add_default_file_extension = true,
                         logging_severity_level const minimal_severity_level = debug);
    ~logging_setup_caller();
private:
    std::string m_log_file_name;
};


struct  screen_text_logger
{
    static screen_text_logger&  instance();

    void  set_max_text_size(std::size_t const  max_size);

    void  clear() { m_text.clear(); }
    void  append(std::string const&  text);
    std::string const&  text() const { return m_text;}

private:
    screen_text_logger();

    std::string  m_text;
    std::size_t  m_max_size;
};


struct  continuous_text_logger
{
    static continuous_text_logger&  instance();

    void  set_line_max_size(std::size_t const  max_size) { m_max_line_size = max_size; }
    void  set_max_num_lines(std::size_t const  max_size) { m_max_num_line = max_size; }

    void  clear() { m_lines.clear(); }
    void  append(std::string const&  line);
    std::deque<std::string> const&  lines() const { return m_lines; }
    std::string  text() const;

private:
    continuous_text_logger();

    std::deque<std::string>  m_lines;
    std::size_t  m_max_line_size;
    std::size_t  m_max_num_line;
};


#endif
