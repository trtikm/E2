#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./program_window.hpp"
#include <utility/timeprof.hpp>
#include <QApplication>

void run()
{
    TMPROF_BLOCK();

    int argc = 0;
    char** argv = 0;
    QApplication app(argc,argv);
    program_window window;
    window.show();
    app.exec();
}
