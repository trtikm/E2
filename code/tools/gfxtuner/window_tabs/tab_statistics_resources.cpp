#include <gfxtuner/window_tabs/tab_statistics_resources.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/program_options.hpp>
#include <QVBoxLayout>
#include <QString>

namespace window_tabs { namespace tab_statistics { namespace tab_resources {


widgets::widgets(program_window* const  wnd)
    : QTreeWidget()
    , m_wnd(wnd)
    , m_icon_data_type((boost::filesystem::path{ get_program_options()->dataRoot() } /
                       "shared/gfx/icons/data_type.png").string().c_str())
    , m_icon_wainting_for_load((boost::filesystem::path{ get_program_options()->dataRoot() } /
                               "shared/gfx/icons/wait.png").string().c_str())
    , m_icon_being_loaded((boost::filesystem::path{ get_program_options()->dataRoot() } /
                          "shared/gfx/icons/loading.png").string().c_str())
    , m_icon_in_use((boost::filesystem::path{ get_program_options()->dataRoot() } /
                    "shared/gfx/icons/in_use.png").string().c_str())
    , m_records()
    , m_just_being_loaded(async::get_invalid_key())
{
    setSelectionMode(QAbstractItemView::SelectionMode::NoSelection);

    {
        QTreeWidgetItem* const  headerItem = new QTreeWidgetItem();
        headerItem->setText(0, QString("Resource types"));
        headerItem->setText(1, QString("#Refs"));
        setHeaderItem(headerItem);
    }
}


void  widgets::on_time_event()
{
}


}}}
