#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <stdexcept>
#include <iostream>
#if PLATFORM() == PLATFORM_WINDOWS()
#   pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") 
#endif

LOG_INITIALISE(get_program_name() + "_LOG",true,true,warning)

extern void run(int argc, char* argv[]);

static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\n";
    boost::filesystem::ofstream  ofile( get_program_name() + "_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\n";
}

int main(int argc, char* argv[])
{
    try
    {
        initialise_program_options(argc,argv);
        if (get_program_options()->helpMode())
            std::cout << get_program_options();
        else if (get_program_options()->versionMode())
            std::cout << get_program_version() << "\n";
        else
        {
            run(argc,argv);
            TMPROF_PRINT_TO_FILE(get_program_name() + "_TMPROF.html",true);
        }

    }
    catch(std::exception const& e)
    {
        try { save_crash_report(e.what()); } catch (...) {}
        return -1;
    }
    catch(...)
    {
        try { save_crash_report("Unknown exception was thrown."); } catch (...) {}
        return -2;
    }
    return 0;
}
