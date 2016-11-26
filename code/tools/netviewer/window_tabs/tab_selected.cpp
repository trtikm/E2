#include <netviewer/window_tabs/tab_selected.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <QVBoxLayout>

namespace window_tabs { namespace tab_selected {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_selected_text(new QTextEdit)
{}

program_window* widgets::wnd() const noexcept
{
    return m_wnd;
}

QTextEdit*  widgets::selected_text() const noexcept
{
    return m_selected_text;
}

void  widgets::on_selection_update()
{
    std::string const  text = wnd()->glwindow().call_now(&simulator::get_selected_info_text);
    selected_text()->setText(QString(text.c_str()));
}

void  widgets::save()
{
    // There is nothing to save.
}


QWidget*  make_selected_tab_content(widgets const&  w)
{
    QWidget* const  selected_tab = new QWidget;
    {
        QVBoxLayout* const layout = new QVBoxLayout;
        {
            w.selected_text()->setReadOnly(true);
            layout->addWidget(w.selected_text());
        }
        selected_tab->setLayout(layout);
    }
    return selected_tab;
}


}}
