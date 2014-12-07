#include <utility/timeprof.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/noncopyable.hpp>
#include <list>
#include <ostream>
#include <map>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <mutex>
#include <thread>


namespace tmprof_internal_private_implementation_details {


struct Record
{
    Record(char const* const file, int const line, char const* const func)
        : m_number_of_executions(0ULL)
        , m_summary_duration(0U)
        , m_duration_of_longest_execution(0U)
        , m_file_name(file)
        , m_line(line)
        , m_function_name(func)
        , m_mutex()
    {}

    natural_64_bit  number_of_executions() const;
    float_64_bit  summary_duration() const;
    float_64_bit  duration_of_longest_execution() const;

    std::string  file_name() const { return std::string(m_file_name); }
    natural_32_bit  line() const { return m_line; }
    std::string  function_name() const { return std::string(m_function_name); }

    void  on_end_of_execution(boost::chrono::system_clock::duration const&  duration_of_execution);

private:
    natural_64_bit  m_number_of_executions;
    boost::chrono::system_clock::duration  m_summary_duration;
    boost::chrono::system_clock::duration  m_duration_of_longest_execution;
    char const*  m_file_name;
    int  m_line;
    char const*  m_function_name;
    mutable std::recursive_mutex  m_mutex;
};


natural_64_bit  Record::number_of_executions() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    natural_64_bit const  result = m_number_of_executions;
    return result;
}

float_64_bit  Record::summary_duration() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    float_64_bit const  result = boost::chrono::duration<float_64_bit>(m_summary_duration).count();
    return result;
}

float_64_bit  Record::duration_of_longest_execution() const
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    float_64_bit const  result = boost::chrono::duration<float_64_bit>(m_duration_of_longest_execution).count();
    return result;
}

void  Record::on_end_of_execution(boost::chrono::system_clock::duration const& duration_of_execution)
{
    std::lock_guard<std::recursive_mutex> lock(m_mutex);
    ++m_number_of_executions;
    m_summary_duration += duration_of_execution;
    if (m_duration_of_longest_execution < duration_of_execution)
        m_duration_of_longest_execution = duration_of_execution;
}



block_stop_watches::block_stop_watches(Record* const  storage_for_results)
    : m_storage_for_results(storage_for_results)
    , m_start_time(boost::chrono::system_clock::now())
{}

block_stop_watches::~block_stop_watches()
{
    m_storage_for_results->on_end_of_execution(boost::chrono::system_clock::now() - m_start_time);
}



struct time_profile_statistics : private boost::noncopyable
{
    time_profile_statistics()
        : m_records()
        , m_mutex_to_list_of_records()
        , m_start_time()
    {}

    Record*  add_record(char const* const file, int const line, char const* const func);
    void copy_time_profile_data(std::vector<time_profile_data_of_block>& storage);
    boost::chrono::system_clock::time_point  start_time() const { return m_start_time; }

private:
    std::list<Record>  m_records;
    std::mutex  m_mutex_to_list_of_records;
    boost::chrono::system_clock::time_point  m_start_time;
};


Record*  time_profile_statistics::add_record(char const* const file, int const line, char const* const func)
{
    std::lock_guard<std::mutex> lock(m_mutex_to_list_of_records);
    if (m_records.empty())
        m_start_time = boost::chrono::system_clock::now();
    m_records.emplace_back(file,line,func);
    Record* const  result = &m_records.back();
    return result;
}

void time_profile_statistics::copy_time_profile_data(std::vector<time_profile_data_of_block>& storage)
{
    std::size_t  num_records;
    std::list<Record>::const_iterator  it;
    {
        std::lock_guard<std::mutex> lock(m_mutex_to_list_of_records);
        num_records = m_records.size();
        it = m_records.begin();
    }
    for (std::size_t  index = 0U; index < num_records; ++index, ++it)
        storage.push_back(
                    time_profile_data_of_block(
                            it->number_of_executions(),
                            it->summary_duration(),
                            it->duration_of_longest_execution(),
                            it->file_name(),
                            it->line(),
                            it->function_name()
                            )
                    );
}


static time_profile_statistics  statistics;

Record* create_new_record_for_block(char const* const file, int const line, char const* const func)
{
    return statistics.add_record(file,line,func);
}


}



time_profile_data_of_block::time_profile_data_of_block(
        natural_64_bit  num_executions,
        float_64_bit  summary_duration,
        float_64_bit  longest_duration,
        std::string  file_name,
        natural_32_bit  line,
        std::string  function_name
        )
    : m_num_executions(num_executions)
    , m_summary_duration(summary_duration)
    , m_longest_duration(longest_duration)
    , m_file_name(file_name)
    , m_line(line)
    , m_function_name(function_name)
{}

natural_64_bit  time_profile_data_of_block::number_of_executions() const
{
    return m_num_executions;
}

float_64_bit  time_profile_data_of_block::summary_duration_of_all_executions_in_seconds() const
{
    return m_summary_duration;
}

float_64_bit  time_profile_data_of_block::duration_of_longest_execution_in_seconds() const
{
    return m_longest_duration;
}

std::string const&  time_profile_data_of_block::file_name() const
{
    return m_file_name;
}

natural_32_bit  time_profile_data_of_block::line() const
{
    return m_line;
}

std::string const&  time_profile_data_of_block::function_name() const
{
    return m_function_name;
}



using ::tmprof_internal_private_implementation_details::statistics;



void copy_time_profile_data_of_all_measured_blocks_into_vector(
        std::vector<time_profile_data_of_block>& storage_for_the_copy_of_data
        )
{
    statistics.copy_time_profile_data(storage_for_the_copy_of_data);
}

float_64_bit  compute_summary_duration_of_all_executions_of_all_blocks_in_seconds(
        std::vector<time_profile_data_of_block> const& collected_profile_data)
{
    float_64_bit  result = 0.0;
    for (auto it = collected_profile_data.begin(); it != collected_profile_data.end(); ++it)
        result += it->summary_duration_of_all_executions_in_seconds();
    return result;
}

boost::chrono::system_clock::time_point  get_time_profiling_start_time_point()
{
    return statistics.start_time();
}

float_64_bit  get_total_time_profiling_time_in_seconds()
{
    return boost::chrono::duration<float_64_bit>(
                boost::chrono::system_clock::now() - get_time_profiling_start_time_point()
                ).count();
}


namespace tmprof_internal_private_implementation_details {


static std::string normalise_duration(float_64_bit const d, natural_32_bit const prec = 3)
{
    auto const dur =
        std::floor((float_32_bit)d * 1000.0f + 0.5f) / 1000.0f
        //d
        ;
    std::stringstream sstr;
    sstr << std::setprecision(prec) << std::fixed << dur;
    return sstr.str();
}

static std::string normalise_divided_duration(float_64_bit const d,
                                              float_64_bit const divider = 1.0,
                                              natural_32_bit const prec = 3)
{
    float_64_bit const dur = d / (divider < 0.0001 ? 0.0001 : divider);
    return normalise_duration(dur,prec);
}

static boost::filesystem::path get_common_prefix(boost::filesystem::path const& p,
                                                 boost::filesystem::path const& q)
{
    boost::filesystem::path res;
    auto pit = p.begin();
    auto qit = q.begin();
    for ( ; pit != p.end() && qit != q.end() && *pit == *qit; ++pit, ++qit)
        res = res / *pit;
    return res;
}

static boost::filesystem::path get_relative_path(boost::filesystem::path const& dir,
                                                 boost::filesystem::path const& file)
{
    auto dit = dir.begin();
    auto fit = file.begin();
    for ( ; dit != dir.end() && fit != file.end() && *dit == *fit; ++dit, ++fit)
        ;
    boost::filesystem::path res;
    for ( ; fit != file.end(); ++fit)
        res = res / *fit;
    return res;
}


}


std::ostream& print_time_profile_data_to_stream(
        std::ostream& os,
        std::vector<time_profile_data_of_block> const& data
        )
{
    using namespace tmprof_internal_private_implementation_details;

    struct print_record
    {
        print_record(time_profile_data_of_block const& r, float_64_bit const total_duration)
            : mFunction(r.function_name())
            , mFile(r.file_name())
            , mLine(boost::lexical_cast<std::string>(r.line()))
            , mDuration(normalise_duration(r.summary_duration_of_all_executions_in_seconds()))
            , mCount(boost::lexical_cast<std::string>(r.number_of_executions()))
            , mMaximal(normalise_duration(r.duration_of_longest_execution_in_seconds()))
            , mAverage(normalise_divided_duration(r.summary_duration_of_all_executions_in_seconds(),
                                                  r.number_of_executions()))
            , mPercentage(normalise_divided_duration(r.summary_duration_of_all_executions_in_seconds(),
                                                     total_duration * 0.01))
        {}
        std::string mFunction;
        std::string mFile;
        std::string mLine;
        std::string mDuration;
        std::string mCount;
        std::string mMaximal;
        std::string mAverage;
        std::string mPercentage;
    };

    float_64_bit const  total_duration =
            //compute_summary_duration_of_all_executions_of_all_blocks_in_seconds(data);
            get_total_time_profiling_time_in_seconds();
    std::string const total_duration_name =
            normalise_duration(total_duration);

    boost::filesystem::path common_path_prefix(
        data.empty() ?
            boost::filesystem::path("") :
            boost::filesystem::path(data.front().file_name()).branch_path()
        );
    for (auto it = data.begin(); it != data.end(); ++it)
        if (it->number_of_executions() > 0ULL)
            common_path_prefix = get_common_prefix(common_path_prefix,it->file_name());

    typedef std::multimap<std::tuple<float_64_bit,float_64_bit,natural_64_bit,float_64_bit>,print_record>
            map_of_print_record;
    map_of_print_record print_records;
    natural_32_bit  capOrderLen = std::string("Order").size();
    natural_32_bit  capFunctionLen = std::string("Function").size();
    natural_32_bit  capDurationLen = std::string("Duration").size();
    natural_32_bit  capAverageLen = std::string("Average").size();
    natural_32_bit  capCountLen = std::string("Count").size();
    natural_32_bit  capMaximalLen = std::string("Peak").size();
    natural_32_bit  capPercentageLen = std::string("%").size();
    natural_32_bit  capDelta = 4U;
    natural_32_bit  maxOrderLen = capOrderLen;
    natural_32_bit  maxFunctionLen = capFunctionLen;
    natural_32_bit  maxDurationLen = capDurationLen;
    natural_32_bit  maxAverageLen = capAverageLen;
    natural_32_bit  maxCountLen = capCountLen;
    natural_32_bit  maxMaximalLen = capMaximalLen;
    natural_32_bit  maxPercentageLen = capPercentageLen;
    for (auto it = data.begin(); it != data.end(); ++it)
    {
        if (it->number_of_executions() == 0ULL)
            continue;

        print_record const r(*it,total_duration);
        auto const key =
            std::make_tuple(
                boost::chrono::duration<float_64_bit>(
                        it->summary_duration_of_all_executions_in_seconds()
                        ).count(),
                boost::chrono::duration<float_64_bit>(
                        it->summary_duration_of_all_executions_in_seconds()
                        ).count() / it->number_of_executions(),
                it->number_of_executions(),
                boost::chrono::duration<float_64_bit>(
                        it->duration_of_longest_execution_in_seconds()
                        ).count()
                );
        map_of_print_record::value_type const value(key,r);
        print_records.insert(value);
        if (maxFunctionLen < r.mFunction.size())
            maxFunctionLen = r.mFunction.size();
        if (maxDurationLen < r.mDuration.size())
            maxDurationLen = r.mDuration.size();
        if (maxAverageLen < r.mAverage.size())
            maxAverageLen = r.mAverage.size();
        if (maxCountLen < r.mCount.size())
            maxCountLen = r.mCount.size();
        if (maxMaximalLen < r.mMaximal.size())
            maxMaximalLen = r.mMaximal.size();
        if (maxPercentageLen < r.mPercentage.size())
            maxPercentageLen = r.mPercentage.size();
    }
    if (maxOrderLen < boost::lexical_cast<std::string>(print_records.size()).size())
        maxOrderLen = boost::lexical_cast<std::string>(print_records.size()).size();
    if (maxDurationLen < total_duration_name.size())
        maxDurationLen = total_duration_name.size();

    os << std::string(maxOrderLen - capOrderLen,' ')
       << "Order"
       << std::string(maxFunctionLen - capFunctionLen + capDelta,' ')
       << "Function"
       << std::string(maxDurationLen - capDurationLen + capDelta,' ')
       << "Duration"
       << std::string(maxAverageLen - capAverageLen + capDelta,' ')
       << "Average"
       << std::string(maxCountLen - capCountLen + capDelta,' ')
       << "Count"
       << std::string(maxMaximalLen - capMaximalLen + capDelta,' ')
       << "Peak"
       << std::string(maxPercentageLen - capPercentageLen + capDelta,' ')
       << "%"
       << std::string(capDelta,' ')
       << "File[line]\n"
       << std::string(
            maxOrderLen     +
            maxFunctionLen  +
            maxDurationLen  +
            maxAverageLen   +
            maxCountLen     +
            maxMaximalLen   +
            maxPercentageLen+
            7 * capDelta    +
            std::string("File[line]").size()
            ,'-')
       << '\n'
       ;
    natural_32_bit i = 1U;
    for (auto it = print_records.rbegin(); it != print_records.rend(); ++it, ++i)
    {
        std::string const ord = boost::lexical_cast<std::string>(i);
        os << std::string(maxOrderLen - ord.size(),' ')
           << ord
           << std::string(capDelta,' ')
           ;

        print_record const& r = it->second;
        os << std::string(maxFunctionLen - r.mFunction.size(),' ')
           << r.mFunction
           << std::string(capDelta,' ')
           ;
        os << std::string(maxDurationLen - r.mDuration.size(),' ')
           << r.mDuration
           << std::string(capDelta,' ')
           ;
        os << std::string(maxAverageLen - r.mAverage.size(),' ')
           << r.mAverage
           << std::string(capDelta,' ')
           ;
        os << std::string(maxCountLen - r.mCount.size(),' ')
           << r.mCount
           << std::string(capDelta,' ')
           ;
        os << std::string(maxMaximalLen - r.mMaximal.size(),' ')
           << r.mMaximal
           << std::string(capDelta,' ')
           ;
        os << std::string(maxPercentageLen - r.mPercentage.size(),' ')
           << r.mPercentage
           << std::string(capDelta,' ')
           ;
        os << get_relative_path(common_path_prefix,r.mFile).string()
           << '[' << r.mLine << "]\n"
           ;
    }
    os << std::string(
            maxOrderLen     +
            maxFunctionLen  +
            maxDurationLen  +
            maxAverageLen   +
            maxCountLen     +
            maxMaximalLen   +
            maxPercentageLen+
            7U * capDelta   +
            std::string("File[line]").size()
            ,'-')
       << '\n'
       ;
    os << std::string(maxOrderLen + maxFunctionLen + 2 * capDelta +
                      maxDurationLen - total_duration_name.size(),' ')
       << total_duration_name
       << std::string(maxAverageLen + maxCountLen + maxMaximalLen +
                      maxPercentageLen + 5 * capDelta,' ')
       << common_path_prefix.string() << "/*\n"
       ;

    return os;
}

std::ostream& print_time_profile_to_stream(std::ostream& os)
{
    std::vector<time_profile_data_of_block> data;
    copy_time_profile_data_of_all_measured_blocks_into_vector(data);
    print_time_profile_data_to_stream(os,data);
    return os;
}

void print_time_profile_to_file(std::string const& file_path_name)
{
    boost::filesystem::ofstream file(file_path_name);
    print_time_profile_to_stream(file);
}
