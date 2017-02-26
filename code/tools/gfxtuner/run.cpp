#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_info.hpp>
#include <gfxtuner/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <QApplication>


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();

    QApplication app(argc, argv);
    program_window  window(std::string("./") + get_program_name() + ".info");
    window.show();
    app.exec();
}
