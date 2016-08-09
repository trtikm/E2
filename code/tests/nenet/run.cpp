#include "./program_info.hpp"
#include "./program_options.hpp"
#include "./simulator.hpp"
#include <qtgl/window.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <QApplication>
#include <memory>


void run()
{
    TMPROF_BLOCK();

    int argc = 1;
    char* argv[] = { "" };
    QGuiApplication app(argc, argv);

    //QScreen *screen = QGuiApplication::primaryScreen();
    //QRect screenGeometry = screen->availableGeometry();
    //QPoint center = QPoint(screenGeometry.center().x(), screenGeometry.top() + 80);
    //QSize windowSize(400, 320);

    std::unique_ptr< qtgl::window<simulator> > pwindow(new qtgl::window<simulator>());
    pwindow->qtbase().setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    pwindow->qtbase().setTitle(QStringLiteral("nenet"));
    pwindow->qtbase().setIcon(QIcon("../data/shared/gfx/icons/E2_icon.png"));
    pwindow->qtbase().setGeometry(QRect(QPoint(0,0), QSize(800,600)));
    pwindow->qtbase().setVisible(true);

    app.exec();
}
