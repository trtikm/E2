#ifndef E2_TOOL_GFXTUNER_PROGRAM_OPTIONS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_PROGRAM_OPTIONS_HPP_INCLUDED

#   include <boost/program_options.hpp>
#   include <boost/noncopyable.hpp>
#   include <ostream>
#   include <memory>
#   include <string>

class program_options : private boost::noncopyable
{
public:
    program_options(int argc, char* argv[]);

    bool helpMode() const { return vm.count("help") > 0; }
    bool versionMode() const { return vm.count("version") > 0; }
    
    std::string  dataRoot() const { return vm["data"].as<std::string>(); }

    bool  has_scene_dir() const { return vm.count("scene") != 0UL; }
    std::string  scene_dir() const { return vm["scene"].as<std::string>(); }

    // Add more option access/query functions here, if needed.

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
