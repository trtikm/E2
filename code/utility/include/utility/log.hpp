#ifndef UTILITY_LOG_HPP_INCLUDED
#   define UTILITY_LOG_HPP_INCLUDED

#   include <utility/config.hpp>
#   include <boost/log/sources/record_ostream.hpp>
#   include <boost/log/sources/logger.hpp>
#   include <boost/log/attributes/constant.hpp>
#   include <string>

#   define LOG(LVL,MSG) \
        if ((LVL) >= BUILD_RELEASE() * 2)\
        {\
            boost::log::sources::logger lg;\
            namespace attrs = boost::log::attributes;\
            lg.add_attribute("File",attrs::constant<std::string>(__FILE__));\
            lg.add_attribute("Line",attrs::constant<unsigned int>(__LINE__));\
            lg.add_attribute("Level",attrs::constant<std::string>(logging_severity_level_name(LVL)));\
            BOOST_LOG(lg) << MSG;\
        }
#   define LOG_INITIALISE(log_file_name)\
        namespace E2 { namespace utility { namespace detail { namespace { namespace {\
            ::logging_setup_caller __logger_setup_caller_instance(log_file_name);\
        }}}}}


enum logging_severity_level
{
    debug = 0,
    info = 1,
    warning = 2,
    error = 3,
};
std::string const& logging_severity_level_name(logging_severity_level const level);

struct logging_setup_caller
{
    logging_setup_caller(std::string const& log_file_name);
    ~logging_setup_caller();
private:
    std::string m_log_file_name;
};

#endif
