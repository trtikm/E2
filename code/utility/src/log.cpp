#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/attributes/clock.hpp>
#include <boost/log/attributes/current_thread_id.hpp>
#include <vector>

std::string logging_severity_level_name(logging_severity_level const level)
{
    static std::vector<std::string> level_names{ "DEBUG", "INFO", "WARNING", "ERROR" };
    ASSUMPTION(static_cast<unsigned int>(level) < level_names.size());
    return level_names.at(static_cast<unsigned int>(level));
}

bool LOG_SETUP(std::string const& log_file_name)
{
    static bool first_call = true;
    if (!first_call)
    {
        //LOG(INFO,"LOG_SETUP was already called ==> ignoring this call.");
        return false;
    }
    first_call = false;

    boost::log::core::get()->add_global_attribute("TimeStamp",boost::log::attributes::local_clock());
    boost::log::core::get()->add_global_attribute("ThreadID",boost::log::attributes::current_thread_id());

    namespace expr = boost::log::expressions;
    boost::log::add_file_log(
        boost::log::keywords::file_name = log_file_name,
        boost::log::keywords::format = "[%TimeStamp%][%ThreadID%][%File%][%Line%][%Level%]: %Message%"
    );

//    boost::log::core::get()->set_filter(
//        boost::log::severity >= DEBUG
//        );

//    LOG(DEBUG,"debug hlaska");
//    LOG(INFO,"info hlaska");
//    LOG(WARNING,"warning hlaska");
//    LOG(ERROR,"error hlaska");

    return true;
}
