#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./program_window.hpp"
#include <utility/timeprof.hpp>
#include <QApplication>


void run()
{
    TMPROF_BLOCK();

    int argc = 1;
    char* argv[] = { "" };
    QApplication app(argc, argv);
    program_window  window(std::string("./") + get_program_name() + ".info");
    window.show();
    app.exec();
}
