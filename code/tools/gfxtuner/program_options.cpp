#include <gfxtuner/program_options.hpp>
#include <gfxtuner/program_info.hpp>
#include <utility/assumptions.hpp>
#include <stdexcept>
#include <iostream>

program_options::program_options(int argc, char* argv[])
    : vm()
    , desc(get_program_description() + "\nUsage")
{
    namespace bpo = boost::program_options;

    desc.add_options()
        ("help,h","Produces this help message.")
        ("version,v", "Prints the version string.")
        ("data,D",
            bpo::value<std::string>()->default_value("../data"),
            "A root directory under which program's data are stored.")
        ("scene,s",
            bpo::value<std::string>(),
            "A directory of a scene to be loaded. A scene directory "
            "always contains a file 'hierarchy.info'. The scene can either "
            "be absolute disk path, or relative path to either the "
            "current working directory or to the data root directory "
            "(see --data option).")
        // Specify more options here, if needed.
        ;

    bpo::positional_options_description pos_desc;

    bpo::store(bpo::command_line_parser(argc,argv).allow_unregistered().
               options(desc).positional(pos_desc).run(),vm);
    bpo::notify(vm);
}

std::ostream& program_options::operator<<(std::ostream& ostr) const
{
    return ostr << desc;
}

static program_options_ptr  global_program_options;

void initialise_program_options(int argc, char* argv[])
{
    ASSUMPTION(!global_program_options.operator bool());
    global_program_options = program_options_ptr(new program_options(argc,argv));
}

program_options_ptr get_program_options()
{
    ASSUMPTION(global_program_options.operator bool());
    return global_program_options;
}

std::ostream& operator<<(std::ostream& ostr, program_options_ptr options)
{
    ASSUMPTION(options.operator bool());
    options->operator<<(ostr);
    return ostr;
}
