#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <boost/filesystem/fstream.hpp>
#include <vector>
#include <iostream>

static bool LOG_SETUP(std::string const& log_file_name)
{
    static bool first_call = true;
    if (!first_call)
    {
        LOG(info,"LOG_SETUP was already called ==> ignoring this call.");
        return false;
    }
    first_call = false;

    boost::log::core::get()->add_global_attribute("TimeStamp",boost::log::attributes::local_clock());
    boost::log::core::get()->add_global_attribute("ThreadID",boost::log::attributes::current_thread_id());

    namespace expr = boost::log::expressions;
    boost::log::add_file_log(
        boost::log::keywords::file_name = log_file_name,
        boost::log::keywords::auto_flush = true,
        boost::log::keywords::open_mode = (std::ios::out | std::ios::app),
        boost::log::keywords::format = "[%TimeStamp%][%ThreadID%][%File%][%Line%][%Level%]: %Message%"
    );

    return true;
}

std::string const& logging_severity_level_name(logging_severity_level const level)
{
    static std::vector<std::string> level_names{ "debug", "info", "warning", "error" };
    ASSUMPTION(static_cast<unsigned int>(level) < level_names.size());
    return level_names.at(static_cast<unsigned int>(level));
}

logging_setup_caller::logging_setup_caller(std::string const& log_file_name)
    : m_log_file_name(log_file_name)
{
    boost::filesystem::ofstream f(m_log_file_name);
    f << "HELLO!\n\n";
    LOG_SETUP(m_log_file_name);
}

logging_setup_caller::~logging_setup_caller()
{
    boost::log::core::get()->remove_all_sinks();
    boost::filesystem::ofstream f(m_log_file_name,std::ios::out | std::ios::app);
    f << "\n\nBYE!\n";
}
