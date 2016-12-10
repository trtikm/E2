#include <netviewer/window_tabs/tab_selected.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <QVBoxLayout>
#include <QPushButton>

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

void  widgets::on_select_owner_spiker()
{
    wnd()->glwindow().call_now(&simulator::on_select_owner_spiker);
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

            layout->addWidget(
                    [](program_window* wnd) {
                            struct look_at_selected : public QPushButton {
                                look_at_selected(program_window* wnd) : QPushButton("Select owner spiker")
                                {
                                    QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_select_owner_spiker()));
                                }
                            };
                            return new look_at_selected(wnd);
                    }(w.wnd())
                    );

        }
        selected_tab->setLayout(layout);
    }
    return selected_tab;
}


}}
