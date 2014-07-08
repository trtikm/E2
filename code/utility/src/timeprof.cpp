#include <utility/timeprof.hpp>
#include <boost/chrono.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/noncopyable.hpp>
#include <list>
#include <ostream>
#include <string>
#include <map>
#include <tuple>
#include <sstream>
#include <iomanip>
#include <cmath>

namespace tmprof {

    struct Record
    {
        typedef boost::chrono::system_clock::duration Duration;

        Record(char const* const file, int const line, char const* const func)
            : mAntiRecursionSemaphore(0)
            , mCurrentExecutionStartTimePoint()
            , mTotalNumberOfAllExecutions(0)
            , mTotalDurationOfAllExecutions(0)
            , mMaximalDurationOfAllExecutions(0)
            , mFileName(file)
            , mLine(line)
            , mFunctionName(func)
        {}

        ~Record()
        {}

        unsigned int totalExecutions() const { return mTotalNumberOfAllExecutions; }
        Duration totalDuration() const { return mTotalDurationOfAllExecutions; }
        Duration maxDuration() const { return mMaximalDurationOfAllExecutions; }
        char const* fileName() const { return mFileName; }
        int  line() const { return mLine; }
        char const* functionName() const { return mFunctionName; }
        bool beeingExecuted() const { return mAntiRecursionSemaphore != 0; }

        void onExecutionBegin();
        void onExecutionEnd();

    private:
        typedef boost::chrono::system_clock::time_point TimePoint;

        unsigned int mAntiRecursionSemaphore;
        TimePoint mCurrentExecutionStartTimePoint;

        unsigned int mTotalNumberOfAllExecutions;
        Duration mTotalDurationOfAllExecutions;
        Duration mMaximalDurationOfAllExecutions;

        char const* mFileName;
        int mLine;
        char const* mFunctionName;
    };

    void Record::onExecutionBegin()
    {
        if (++mAntiRecursionSemaphore == 1)
        {
            mCurrentExecutionStartTimePoint = boost::chrono::system_clock::now();
        }
    }

    void Record::onExecutionEnd()
    {
        if (--mAntiRecursionSemaphore == 0)
        {
            Duration const D =
                boost::chrono::system_clock::now() - mCurrentExecutionStartTimePoint;

            ++mTotalNumberOfAllExecutions;
            mTotalDurationOfAllExecutions += D;
            if (mMaximalDurationOfAllExecutions < D)
                mMaximalDurationOfAllExecutions = D;
        }
    }

    MeasurementSignalsGenerator::MeasurementSignalsGenerator(Record* const r)
        : mRecord(r)
    {
        mRecord->onExecutionBegin();
    }

    MeasurementSignalsGenerator::~MeasurementSignalsGenerator()
    {
        mRecord->onExecutionEnd();
    }

}

namespace tmprof {

    struct Statistics : private boost::noncopyable
    {
        typedef std::list<Record> Records;
        typedef boost::chrono::system_clock::time_point TimePoint;

        static Statistics& instance();

        Statistics()
            : mRecords()
            , mStartTimePoint(boost::chrono::system_clock::now())
        {}

        Records& records() { return mRecords; }
        TimePoint startTimePoint() const { return mStartTimePoint; }

    private:
        Records mRecords;
        TimePoint mStartTimePoint;
    };

    Statistics& Statistics::instance()
    {
        static Statistics stats;
        return stats;
    }

    static Statistics::Records& records()
    {
        return Statistics::instance().records();
    }

    static Statistics::TimePoint startTimePoint()
    {
        return Statistics::instance().startTimePoint();
    }

    Record* getNewRecordPtr(char const* const file, int const line, char const* const func)
    {
        records().push_back(Record(file,line,func));
        return &records().back();
    }

}

namespace tmprof {

    struct WriteRec
    {
        static std::string normalizeDuration(double const d, unsigned int prec = 3)
        {
            auto const dur =
                std::floor((float)d * 1000.0f + 0.5f) / 1000.0f
                //d
                ;
            std::stringstream sstr;
            sstr << std::setprecision(prec) << std::fixed << dur;
            return sstr.str();
        }

        static std::string normalizeDuration(Record::Duration const& d,
                                             double const divider = 1.0,
                                             unsigned int prec = 3)
        {
            double const dur = boost::chrono::duration<double>(d).count() /
                                    (divider < 0.0001 ? 0.0001 : divider);
            return normalizeDuration(dur,prec);
        }

        WriteRec(Record const& r, double const totalDuration)
            : mFunction(std::string(r.beeingExecuted() ? "*": "") + r.functionName())
            , mFile(r.fileName())
            , mLine(boost::lexical_cast<std::string>(r.line()))
            , mDuration(normalizeDuration(r.totalDuration()))
            , mCount(boost::lexical_cast<std::string>(r.totalExecutions()))
            , mMaximal(normalizeDuration(r.maxDuration()))
            , mAverage(normalizeDuration(r.totalDuration(),r.totalExecutions()))
            , mPercentage(normalizeDuration(r.totalDuration(),totalDuration * 0.01))
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

    static boost::filesystem::path getCommonPrefix(boost::filesystem::path const& p,
                                                   boost::filesystem::path const& q)
    {
        boost::filesystem::path res;
        auto pit = p.begin();
        auto qit = q.begin();
        for ( ; pit != p.end() && qit != q.end() && *pit == *qit; ++pit, ++qit)
            res = res / *pit;
        return res;
    }

    static boost::filesystem::path getRelativePath(boost::filesystem::path const& dir,
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

    std::ostream& write(std::ostream& os)
    {
        double const totalDuration =
            boost::chrono::duration<double>(
                Record::Duration(boost::chrono::system_clock::now() - startTimePoint())
                ).count();
        std::string const totalDurationName = WriteRec::normalizeDuration(totalDuration);

        boost::filesystem::path commonPathPrefix(
            records().empty() ?
                boost::filesystem::path("") :
                boost::filesystem::path(records().front().fileName()).branch_path()
            );
        for (auto it = records().begin(); it != records().end(); ++it)
        {
            if (it->totalExecutions() == 0)
                continue;
            commonPathPrefix = getCommonPrefix(commonPathPrefix,it->fileName());
        }

        typedef std::multimap<std::tuple<double,double,unsigned int,double>,WriteRec>
                RecordsMap;
        RecordsMap recs;
        unsigned int capOrderLen = std::string("Order").size();
        unsigned int capFunctionLen = std::string("Function").size();
        unsigned int capDurationLen = std::string("Duration").size();
        unsigned int capAverageLen = std::string("Average").size();
        unsigned int capCountLen = std::string("Count").size();
        unsigned int capMaximalLen = std::string("Peak").size();
        unsigned int capPercentageLen = std::string("%").size();
        unsigned int capDelta = 4;
        unsigned int maxOrderLen = capOrderLen;
        unsigned int maxFunctionLen = capFunctionLen;
        unsigned int maxDurationLen = capDurationLen;
        unsigned int maxAverageLen = capAverageLen;
        unsigned int maxCountLen = capCountLen;
        unsigned int maxMaximalLen = capMaximalLen;
        unsigned int maxPercentageLen = capPercentageLen;
        for (auto it = records().begin(); it != records().end(); ++it)
        {
            if (it->totalExecutions() == 0)
                continue;

            WriteRec const r(*it,totalDuration);
            auto const key =
                std::make_tuple(
                    boost::chrono::duration<double>(it->totalDuration()).count(),
                    boost::chrono::duration<double>(it->totalDuration()).count()
                        / it->totalExecutions(),
                    it->totalExecutions(),
                    boost::chrono::duration<double>(it->maxDuration()).count()
                    );
            RecordsMap::value_type const value(key,r);
            recs.insert(value);
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
        if (maxOrderLen < boost::lexical_cast<std::string>(recs.size()).size())
            maxOrderLen = boost::lexical_cast<std::string>(recs.size()).size();
        if (maxDurationLen < totalDurationName.size())
            maxDurationLen = totalDurationName.size();

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
        unsigned int i = 1;
        for (auto it = recs.rbegin(); it != recs.rend(); ++it, ++i)
        {
            std::string const ord = boost::lexical_cast<std::string>(i);
            os << std::string(maxOrderLen - ord.size(),' ')
               << ord
               << std::string(capDelta,' ')
               ;

            WriteRec const& r = it->second;
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
            os << getRelativePath(commonPathPrefix,r.mFile).string()
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
                7 * capDelta    +
                std::string("File[line]").size()
                ,'-')
           << '\n'
           ;
        os << std::string(maxOrderLen + maxFunctionLen + 2 * capDelta +
                          maxDurationLen - totalDurationName.size(),' ')
           << totalDurationName
           << std::string(maxAverageLen + maxCountLen + maxMaximalLen +
                          maxPercentageLen + 5 * capDelta,' ')
           << commonPathPrefix.string() << "/*\n"
           ;

        return os;
    }

    void write(std::string const& filePathName)
    {
        boost::filesystem::ofstream file(filePathName);
        write(file);
    }

}
