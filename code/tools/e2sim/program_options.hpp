#ifndef E2_TOOL_E2SIM_PROGRAM_OPTIONS_HPP_INCLUDED
#   define E2_TOOL_E2SIM_PROGRAM_OPTIONS_HPP_INCLUDED

#   include <utility/program_options_base.hpp>
#   include <memory>

class program_options : public program_options_default
{
public:
    program_options(int argc, char* argv[]);

    bool  has_scene_dir() const { return has("scene"); }
    std::string  scene_dir() const { return value("scene"); }
};

typedef std::shared_ptr<program_options const> program_options_ptr;

void initialise_program_options(int argc, char* argv[]);
program_options_ptr get_program_options();

std::ostream& operator<<(std::ostream& ostr, program_options_ptr options);

#endif
