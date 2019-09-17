#include <setimagealphachannel/program_options.hpp>
#include <setimagealphachannel/program_info.hpp>
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
        ("colour_image,c",
            bpo::value<std::string>(),
            "Pathname to the first input PNG image with RGBA colour channels, "
            "which will directly represent resulting RGB channels.")
        ("alpha_image,a",
            bpo::value<std::string>(),
            "Pathname to the second input PNG image with RGBA colour channels, "
            "where mean value of RGB channels will represent the resulting\n"
            "A-channel.")
        ("resulting_image,r",
            bpo::value<std::string>(),
            "Pathname to the resulting PNG image with RGBA channels will be "
            "computed from RGB channels of the input images.")
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
