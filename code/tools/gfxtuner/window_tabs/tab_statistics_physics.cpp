#include <gfxtuner/window_tabs/tab_statistics_physics.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <boost/algorithm/string.hpp>
#include <QVBoxLayout>
#include <QString>
#include <vector>
#include <memory>

namespace window_tabs { namespace tab_statistics { namespace tab_physics {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
{
}


void  widgets::on_time_event()
{
    TMPROF_BLOCK();

}


std::string  get_tooltip()
{
    return "Statistics of rigid body simulator,\n"
           "including the constraints solver.";
}


QWidget*  make_tab_content(widgets const&  w)
{
    QWidget* const  tab = new QWidget;
    {
    }
    return tab;
}


}}}
