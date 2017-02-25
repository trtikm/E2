#include <netviewer/window_tabs/tab_selected.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <QVBoxLayout>
#include <QPushButton>
#include <limits>

namespace {


constexpr natural_64_bit  invalid_network_update_id()
{
    return std::numeric_limits<natural_64_bit>::max();
}

inline netlab::compressed_layer_and_object_indices  invalid_network_object()
{
    return { std::numeric_limits<netlab::layer_index_type>::max(), std::numeric_limits<netlab::object_index_type>::max() };
}


}

namespace window_tabs { namespace tab_selected {


widgets::widgets(program_window* const  wnd)
    : m_wnd(wnd)
    , m_selected_text(new QTextEdit)
    , m_update_id(invalid_network_update_id())
    , m_selected_object(invalid_network_object())
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
    if (!wnd()->glwindow().call_now(&simulator::has_network))
    {
        selected_text()->setText(QString("No network is loaded."));
        m_update_id = invalid_network_update_id();
        m_selected_object = invalid_network_object();
        return;
    }
    auto const  stats_ptr = wnd()->glwindow().call_now(&simulator::get_selected_object_stats);
    if (stats_ptr == nullptr)
    {
        selected_text()->setText(QString("No network object is selected."));
        m_update_id = invalid_network_update_id();
        m_selected_object = invalid_network_object();
        return;
    }
    natural_64_bit const  update_id = wnd()->glwindow().call_now(&simulator::get_network_update_id);
    if (update_id != m_update_id || m_selected_object != stats_ptr->indices())
    {
        std::string const  text = wnd()->glwindow().call_now(&simulator::get_selected_info_text);
        selected_text()->setText(QString(text.c_str()));

        m_update_id = update_id;
        m_selected_object = stats_ptr->indices();
    }
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
