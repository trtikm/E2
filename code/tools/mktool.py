import argparse
import os
import re


_template_CMakeLists_txt = \
"""set(THIS_TARGET_NAME <%TARGET_NAME%>)

<%QT_AUTOMOC%>

add_executable(${THIS_TARGET_NAME}
    program_info.hpp
    program_info.cpp

    program_options.hpp
    program_options.cpp

    main.cpp
    run.cpp
    )

target_link_libraries(${THIS_TARGET_NAME}
    <%LINK_LIBS_LIST%>
    ${BOOST_LIST_OF_LIBRARIES_TO_LINK_WITH}
    )

set_target_properties(${THIS_TARGET_NAME} PROPERTIES
    DEBUG_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Debug"
    RELEASE_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_Release"
    RELWITHDEBINFO_OUTPUT_NAME "${THIS_TARGET_NAME}_${CMAKE_SYSTEM_NAME}_RelWithDebInfo"
    )

install(TARGETS ${THIS_TARGET_NAME} DESTINATION "tools")

"""


_template_main_cpp = \
"""#include <<%TARGET_NAME%>/program_info.hpp>
#include <<%TARGET_NAME%>/program_options.hpp>
#include <utility/config.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
<%GFX_ON_WINDOWS%>


LOG_INITIALISE(get_program_name() + "_LOG",true,true,warning)

extern void run(int argc, char* argv[]);

static void save_crash_report(std::string const& crash_message)
{
    std::cout << "ERROR: " << crash_message << "\\n";
    std::ofstream  ofile( get_program_name() + "_CRASH.txt", std::ios_base::app );
    ofile << crash_message << "\\n";
}

int main(int argc, char* argv[])
{
#if BUILD_RELEASE() == 1
    try
#endif
    {
        initialise_program_options(argc,argv);
        if (get_program_options()->helpMode())
            std::cout << get_program_options();
        else if (get_program_options()->versionMode())
            std::cout << get_program_version() << "\\n";
        else
        {
            run(argc,argv);
            TMPROF_PRINT_TO_FILE(get_program_name() + "_TMPROF.html",true);
        }

    }
#if BUILD_RELEASE() == 1
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
#endif
    return 0;
}
"""


_template_gfx_on_windows = \
"""#if PLATFORM() == PLATFORM_WINDOWS()
#   pragma comment(linker, "/SUBSYSTEM:windows /ENTRY:mainCRTStartup") 
#endif
"""


_template_program_info_hpp = \
"""#ifndef E2_TOOL_<%TARGET_NAME_UPPER_CASE%>_PROGRAM_INFO_HPP_INCLUDED
#   define E2_TOOL_<%TARGET_NAME_UPPER_CASE%>_PROGRAM_INFO_HPP_INCLUDED

#   include <string>

std::string  get_program_name();
std::string  get_program_version();
std::string  get_program_description();

#endif
"""


_template_program_info_cpp = \
"""#include <<%TARGET_NAME%>/program_info.hpp>

std::string  get_program_name()
{
    return "<%TARGET_NAME%>";
}

std::string  get_program_version()
{
    return "<%VERSION%>";
}

std::string  get_program_description()
{
    return "TODO: Describe here the purpose of the tool, what functionality\\n"
           "it provides, and briefly explain basic idea(s)/principle(s) of its\\n"
           "functionality. (Note: end each line by '\\\\n'.)"
           ;
}
"""


_template_program_options_hpp = \
"""#ifndef E2_TOOL_<%TARGET_NAME_UPPER_CASE%>_PROGRAM_OPTIONS_HPP_INCLUDED
#   define E2_TOOL_<%TARGET_NAME_UPPER_CASE%>_PROGRAM_OPTIONS_HPP_INCLUDED

#   include <boost/program_options.hpp>
#   include <boost/noncopyable.hpp>
#   include <string>
#   include <ostream>
#   include <memory>

class program_options : private boost::noncopyable
{
public:
    program_options(int argc, char* argv[]);

    bool helpMode() const { return vm.count("help") > 0; }
    bool versionMode() const { return vm.count("version") > 0; }
    std::string  data_root() const { return vm["data"].as<std::string>(); }
    
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
"""


_template_program_options_cpp = \
"""#include <<%TARGET_NAME%>/program_options.hpp>
#include <<%TARGET_NAME%>/program_info.hpp>
#include <utility/assumptions.hpp>
#include <stdexcept>
#include <iostream>

program_options::program_options(int argc, char* argv[])
    : vm()
    , desc(get_program_description() + "\\nUsage")
{
    namespace bpo = boost::program_options;

    desc.add_options()
        ("help,h","Produces this help message.")
        ("version,v", "Prints the version string.")
        ("data,D",
            bpo::value<std::string>()->default_value("../data"),
            "A root directory under which program's data are stored.")
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
"""


_template_run_cpp = \
"""#include <<%TARGET_NAME%>/program_info.hpp>
#include <<%TARGET_NAME%>/program_options.hpp>
#include <angeo/tensor_math.hpp><%COM_SIMULATOR_INCLUDES%>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>

<%COM_SIMULATOR_CODE%>

void run(int argc, char* argv[])
{
    TMPROF_BLOCK();
<%RUN_IMPLEMENTATION%>
}
"""


_template_com_simulator_includes = \
"""
#include <com/simulator.hpp>
#include <gfx/image.hpp>"""


_template_com_simulator_code = \
"""
struct  simulator : public com::simulator
{
    simulator() : com::simulator(get_program_options()->data_root()) {}

    void  initialise() override
    {
        com::simulator::initialise();

        set_window_title(get_program_name());
        gfx::image_rgba_8888  img;
        gfx::load_png_image(context()->get_icon_root_dir() + "E2_icon.png", img);
        set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        //maximise_window();
    }

    void  on_begin_round() override
    {
        if (get_window_props().focus_just_lost())
            simulation_config().paused = true;

        if (!get_window_props().has_focus())
            return;

        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_PAUSE()) != 0UL)
            simulation_config().paused = !simulation_config().paused;
        else if (get_keyboard_props().keys_just_pressed().count(osi::KEY_SPACE()) != 0UL)
        {
            simulation_config().paused = false;
            simulation_config().num_rounds_to_pause = 1U;
        }

        //bool const  shift = get_keyboard_props().keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
        //                    get_keyboard_props().keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
        //bool const  ctrl = get_keyboard_props().keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
        //                   get_keyboard_props().keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
        //bool const  alt = get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
        //                  get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL;

        // TODO: Implement tool's functionality here.
    }

    void  on_begin_render() override
    {
        if (get_keyboard_props().keys_pressed().count(osi::KEY_F1()) != 0UL)
        {
            SLOG("\\n=== HELP ===\\n");
        }

        // TODO: Implement tool's custom render here.
    }
};
"""


_template_com_simulator_call = \
"""    osi::run(std::make_unique<simulator>());"""


_template_run_todo_message = \
"""
    // TODO: Implement tool's functionality here.
    //       Program options are already parsed. You
    //       thus do not have to access argc or argv.
    //       Simply call 'get_program_options()' to
    //       access the options.
"""


def get_list_of_E2_libraries():
    return sorted([
        "ai",
        "angeo",
        "com",
        "gfx",
        "netlab",
        "osi",
        "utility",
        
        # Obsolete libraries:
        "cellab",
        "cellconnect",
        "efloop",
        "envlab",
        "ode",
        "netexp",
        "netview",
        "paralab",
        "plot",
        "qtgl",
        "scene"
        ])

def get_list_of_E2_libraries_dependent_on_OS_or_GL():
    return sorted([
        "com",
        "gfx",
        "osi"
        ])
        

def parse_cmd_line():
    parser = argparse.ArgumentParser(
        description="This script allows you to easily create an initial version of source and build "
                    "files of a new tool based on libraries of E2 project. The script contains a "
                    "'template tool project' and it instantiates the template according to information "
                    "provided to this script via command line arguments. The script creates a new "
                    "sub-directory of the current directory and all generated files are stored into "
                    "that sub-directory. The name of the sub-directory is equal to a passed project name "
                    "(see the option '--target_name'). The project is automatically included into "
                    "the build of E2.")
    parser.add_argument("-T", "--target_name", type=str,
                        help="A project name of the tool. All generated project files will be "
                             "generated into a newly created sub-directory of the current one. The "
                             "name of the sub-directory is equal as the passed target name.")
    parser.add_argument("-V","--version", type=str, default="0.01",
                        help="A string representing an initial version of the tool. The version can be later "
                             "updated in a function 'get_program_version()' in a generated file 'program_info.cpp'")
    parser.add_argument("-L","--link_libs", nargs='+', type=str, default=["utility"],
                        help="A space-separated list of names of E2 libraries the tool should be "
                             "linked with. Use quotes if library's name contains spaces. The library 'utility' "
                             "is always included automatically. Here is a list of poosible libraries: "
                             "ai, angeo, com, gfx, netlab, osi.\nNOTE: 3rd libraries must be added to the "
                             "CMakeLists.txt file of the tool manually. Only libraries Boost, GLAD, GLFW, LODPNG, "
                             "and OpenGL are added automatically with E2 libraries dependent on them. "
                             "Header only libraries, like Eigen, should not be specified.")
    parser.add_argument("--console", type=bool, default=False,
                        help="When specified a console window will also appear when starting gfx application.")
    args = parser.parse_args()
    return args


def scriptMain():
    args = parse_cmd_line()

    if args.target_name is None:
        print("ERROR: The option --target_name is not specified. Use the option --help for more info.")
        return
    if re.match(r"^[^\d\W]\w*\Z",args.target_name, re.UNICODE) is None:
        print("ERROR: The target name (i.e. the value of --target_name) is not an proper C identifier.")
        return
    dir_name = os.path.abspath("./" + args.target_name)
    if os.path.exists(dir_name):
        print("ERROR: The disk path '" + dir_name + "' already exists.")
        return

    add_gl = False
    libs_list = ""
    automoc_text = ""
    for libname in args.link_libs:
        if not libname in get_list_of_E2_libraries():
            print("ERROR: '" + libname + "' is not recognised as E2 library.")
            return
        if libname in get_list_of_E2_libraries_dependent_on_OS_or_GL():
            add_gl = True
        libs_list += libname + "\n    "
    if "angeo" not in args.link_libs and any(x in args.link_libs for x in ["com", "gfx", "ai"]):
        libs_list += "angeo\n    "
    if "utility" not in args.link_libs:
        libs_list += "utility\n    "
    if add_gl:
        libs_list += "${OPENGL_LIST_OF_LIBRARIES_TO_LINK_WITH}" + "\n    "
        libs_list += "${GLAD_LIST_OF_LIBRARIES_TO_LINK_WITH}" + "\n    "
        libs_list += "${GLFW_LIST_OF_LIBRARIES_TO_LINK_WITH}" + "\n    "
        libs_list += "${LODEPNG_LIST_OF_LIBRARIES_TO_LINK_WITH}" + "\n    "
    libs_list = libs_list.strip()

    os.mkdir(dir_name)

    out_file = open(dir_name + "/CMakeLists.txt","w")
    out_text = _template_CMakeLists_txt.replace("<%TARGET_NAME%>",args.target_name)\
                                       .replace("<%LINK_LIBS_LIST%>",libs_list)\
                                       .replace("<%QT_AUTOMOC%>",automoc_text)
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/main.cpp","w")
    out_text = _template_main_cpp.replace("<%TARGET_NAME%>",args.target_name)
    out_text = out_text.replace("<%GFX_ON_WINDOWS%>", "" if args.console is True else _template_gfx_on_windows)
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/program_info.hpp","w")
    out_text = _template_program_info_hpp.replace("<%TARGET_NAME_UPPER_CASE%>",args.target_name.upper())
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/program_info.cpp","w")
    out_text = _template_program_info_cpp.replace("<%TARGET_NAME%>",args.target_name)\
                                         .replace("<%VERSION%>",args.version)
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/program_options.hpp","w")
    out_text = _template_program_options_hpp.replace("<%TARGET_NAME_UPPER_CASE%>",args.target_name.upper())
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/program_options.cpp","w")
    out_text = _template_program_options_cpp.replace("<%TARGET_NAME%>",args.target_name)
    out_file.write(out_text)
    out_file.close()

    out_file = open(dir_name + "/run.cpp","w")
    out_text = _template_run_cpp.replace("<%TARGET_NAME%>",args.target_name)
    if "com" in args.link_libs:
        out_text = out_text.replace("<%COM_SIMULATOR_INCLUDES%>", _template_com_simulator_includes)
        out_text = out_text.replace("<%COM_SIMULATOR_CODE%>", _template_com_simulator_code)
        out_text = out_text.replace("<%RUN_IMPLEMENTATION%>", _template_com_simulator_call)
    else:
        out_text = out_text.replace("<%COM_SIMULATOR_INCLUDES%>", "")
        out_text = out_text.replace("<%COM_SIMULATOR_CODE%>", "")
        out_text = out_text.replace("<%RUN_IMPLEMENTATION%>", _template_run_todo_message)
    
    out_file.write(out_text)
    out_file.close()

    out_file = open("./CMakeLists.txt","a")
    out_file.write("\nadd_subdirectory(./" + args.target_name + ")\n    message(\"-- " + args.target_name + "\")\n")
    out_file.close()

    
if __name__ == "__main__":
    scriptMain()

