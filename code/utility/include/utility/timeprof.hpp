#ifndef UTILITY_TIMEPROF_HPP_INCLUDED
#   define UTILITY_TIMEPROF_HPP_INCLUDED

#   include <utility/config.hpp>
#   include <iosfwd>
#   include <string>
#   include <vector>

#   if !((BUILD_DEBUG() == 1 && defined(DEBUG_DISABLE_TIME_PROFILING)) ||      \
         (BUILD_RELEASE() == 1 && defined(RELEASE_DISABLE_TIME_PROFILING)))
#       define TMPROF_BLOCK()                                                  \
            static tmprof::Record* const ___tmprof__Record__pointer__ =        \
                tmprof::getNewRecordPtr(__FILE__,__LINE__,__FUNCTION__);       \
            tmprof::MeasurementSignalsGenerator const                          \
                ___tmprof__MeasurementSignalsGenerator__(                      \
                    ___tmprof__Record__pointer__);
#       define TMPROF_WRITE_TO_FILE(fname) tmprof::write(fname);
#       define TMPROF_WRITE_TO_STREAM(stream) tmprof::write(stream);
#   else
#       define TMPROF_BLOCK()
#       define TMPROF_WRITE_TO_FILE(stream)
#       define TMPROF_WRITE_TO_STREAM(stream)
#   endif

namespace tmprof {

    struct Record;

    struct MeasurementSignalsGenerator
    {
        explicit MeasurementSignalsGenerator(Record* const r);
        ~MeasurementSignalsGenerator();
    private:
        Record* mRecord;
    };

    Record* getNewRecordPtr(char const* const file, int const line,
                            char const* const func);

    void write(std::string const& filePathName);
    std::ostream& write(std::ostream& os);

    struct RecordReader
    {
        explicit RecordReader(Record const* const rec);
        unsigned int totalExecutions() const;
        double totalDuration() const;
        double maxDuration() const;
        std::string fileName() const;
        int line() const;
        std::string functionName() const;
    private:
        Record const* mRecord;
    };

    typedef std::vector<RecordReader> RecordReaders;

    void write(RecordReaders& v);

    double getTotalProfilingTime();

}

#endif
