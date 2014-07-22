#ifndef UTILITY_LOG_HPP_INCLUDED
#   define UTILITY_LOG_HPP_INCLUDED

#   include <boost/log/sources/record_ostream.hpp>
#   include <boost/log/sources/severity_logger.hpp>
#   include <boost/log/attributes/constant.hpp>
#   include <string>

#   define LOG_TO(LGR,LVL,MSG) \
        (LGR).add_attribute("File",boost::log::attributes::constant<std::string>(__FILE__)); \
        (LGR).add_attribute("Line",boost::log::attributes::constant<unsigned int>(__LINE__)); \
        (LGR).add_attribute("Level",boost::log::attributes::constant<std::string>(logging_severity_level_name(LVL))); \
        BOOST_LOG_SEV(LGR,LVL) << MSG
#   define LOG(LVL,MSG) { logger lg; LOG_TO(lg,LVL,MSG); }

enum logging_severity_level
{
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3,
};

typedef boost::log::sources::severity_logger<logging_severity_level> logger;

std::string logging_severity_level_name(logging_severity_level const level);
bool LOG_SETUP(std::string const& log_file_name = "E2.log");

#endif
