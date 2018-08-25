#include <gfxtuner/window_tabs/tab_statistics_resources.hpp>
#include <gfxtuner/program_window.hpp>
#include <QVBoxLayout>
#include <QString>

namespace window_tabs { namespace tab_statistics { namespace tab_resources {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
{
}


QWidget*  make_tab_content(widgets const&  w)
{
    QWidget* const  tab = new QWidget;
    {
        QVBoxLayout* const tab_layout = new QVBoxLayout;
        {
            tab_layout->addStretch(1);
        }
        tab->setLayout(tab_layout);
    }
    return tab;
}


}}}
