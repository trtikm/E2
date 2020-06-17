#include <random_spikes_distribution_in_time_step/program_options.hpp>
#include <random_spikes_distribution_in_time_step/program_info.hpp>
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
        ("genjson,g", "Generates a default JSON experiment setup file into the current directory.")
        ("inputjson,i",
            bpo::value<std::string>()->default_value("./" + get_program_name() + ".json"),
            "A pathname of a JSON file defining the experiment setup.")
        ("outputdir,o",
            bpo::value<std::string>()->default_value("."),
            "Output directory under which results will be saved.")
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
