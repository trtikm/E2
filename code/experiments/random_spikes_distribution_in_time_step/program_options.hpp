#ifndef E2_EXPERIMENTS_RANDOM_SPIKES_DISTRIBUTION_IN_TIME_STEP_PROGRAM_OPTIONS_HPP_INCLUDED
#   define E2_EXPERIMENTS_RANDOM_SPIKES_DISTRIBUTION_IN_TIME_STEP_PROGRAM_OPTIONS_HPP_INCLUDED

#   include <boost/program_options.hpp>
#   include <boost/noncopyable.hpp>
#   include <ostream>
#   include <memory>

class program_options : private boost::noncopyable
{
public:
    program_options(int argc, char* argv[]);

    bool helpMode() const { return vm.count("help") > 0; }
    bool versionMode() const { return vm.count("version") > 0; }
    bool genJson() const { return vm.count("genjson") > 0; }
    std::string const& inputJson() const { return vm["inputjson"].as<std::string>(); }
    std::string const& outputDir() const { return vm["outputdir"].as<std::string>(); }

    std::ostream& operator<<(std::ostream& ostr) const;

private:
    boost::program_options::variables_map vm;
    boost::program_options::options_description desc;
};

typedef std::shared_ptr<program_options const> program_options_ptr;

void initialise_program_options(int argc, char* argv[]);
program_options_ptr get_program_options();

std::ostream& operator<<(std::ostream& ostr, program_options_ptr options);

#endif
