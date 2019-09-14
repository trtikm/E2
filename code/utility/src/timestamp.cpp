#include <utility/timestamp.hpp>
#include <utility/config.hpp>
#include <boost/chrono.hpp>
#include <boost/filesystem/fstream.hpp>
#include <ctime>
#include <iomanip>


std::string  compute_timestamp()
{
    std::time_t t = boost::chrono::system_clock::to_time_t(boost::chrono::system_clock::now());
#   if COMPILER() == COMPILER_VC()
    struct tm timeinfo;
    localtime_s(&timeinfo, &t);
    std::tm* const ptm = &timeinfo;
#else
    std::tm* const ptm = std::localtime(&t);
#endif
    std::stringstream sstr;
    sstr << "--"
         << ptm->tm_year + 1900 << "-"
         << std::setfill('0') << std::setw(2)
         << ptm->tm_mon << "-"
         << std::setfill('0') << std::setw(2)
         << ptm->tm_mday << "--"
         << std::setfill('0') << std::setw(2)
         << ptm->tm_hour << "-"
         << std::setfill('0') << std::setw(2)
         << ptm->tm_min << "-"
         << std::setfill('0') << std::setw(2)
         << ptm->tm_sec
         ;
    return sstr.str();
}

std::string  extend_file_path_name_by_timestamp(std::string const& file_path_name)
{
    boost::filesystem::path const file(file_path_name);
    boost::filesystem::path path = file.branch_path();
    boost::filesystem::path name = file.filename().replace_extension("");
    boost::filesystem::path ext = file.extension();
    return (path / (name.string() + compute_timestamp() + ext.string())).string();
}
