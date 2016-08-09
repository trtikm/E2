#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./program_window.hpp"
#include <utility/timeprof.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/info_parser.hpp>
#include <boost/filesystem.hpp>
#include <QApplication>
#include <string>

void run()
{
    TMPROF_BLOCK();

    std::string const  property_file_pathname =
            std::string("./") + get_program_name() + ".info";
    boost::property_tree::ptree  ptree;
    if (boost::filesystem::exists(property_file_pathname))
        boost::property_tree::read_info(property_file_pathname,ptree);
    else
    {
        ptree.put("program_window.pos.x", 0);
        ptree.put("program_window.pos.y", 0);
        ptree.put("program_window.width", 1024);
        ptree.put("program_window.height", 768);
        ptree.put("program_window.splitter_ratio", 0.641453803f);
        ptree.put("program_window.show_maximised", false);

        ptree.put("tabs.window.clear_colour.red", 64);
        ptree.put("tabs.window.clear_colour.green", 64);
        ptree.put("tabs.window.clear_colour.blue", 64);

        ptree.put("tabs.camera.pos.x", 6.56402016f);
        ptree.put("tabs.camera.pos.y", 6.53800011f);
        ptree.put("tabs.camera.pos.z", 3.27975011f);
        ptree.put("tabs.camera.rot.w", 0.298171997f);
        ptree.put("tabs.camera.rot.x", 0.228851005f);
        ptree.put("tabs.camera.rot.y", 0.564207971f);
        ptree.put("tabs.camera.rot.z", 0.735112011f);

        ptree.put("tabs.camera.save_pos_rot", false);

        ptree.put("tabs.batch.root_dir", boost::filesystem::current_path().string());
        ptree.put("tabs.batch.last_dir", boost::filesystem::current_path().string());

        ptree.put("last_dirs.buffer", boost::filesystem::current_path().string());
    }

    {
        int argc = 0;
        char** argv = 0;
        QApplication app(argc,argv);
        program_window window(&ptree);
        window.show();
        app.exec();
    }

    boost::property_tree::write_info(property_file_pathname,ptree);
}
