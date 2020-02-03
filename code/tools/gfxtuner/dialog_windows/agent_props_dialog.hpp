#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_AGENT_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_AGENT_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/records/agent/agent.hpp>
#   include <ai/agent_kind.hpp>
#   include <ai/sensor_kind.hpp>
#   include <ai/sensor_action.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QComboBox>
#   include <QListWidget>
#   include <QTableWidget>
#   include <unordered_map>
#   include <vector>

namespace dialog_windows {


struct  agent_props_dialog : public QDialog
{
    agent_props_dialog(
            program_window* const  wnd,
            scn::agent_props const&  current_props,
            std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> > const&  sensor_nodes_and_kinds
            );

    bool  ok() const { return m_ok; }

    scn::agent_props const&  get_new_props() const { return m_new_props; }

public slots:

    void  accept();
    void  reject();

private:

    Q_OBJECT

    program_window*  m_wnd;
    bool  m_ok;
    QPushButton* m_widget_ok;
    QComboBox*  m_agent_kind_combobox;

    scn::agent_props  m_current_props;
    scn::agent_props  m_new_props;
    std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> >  m_sensor_nodes_and_kinds;
};


}

#endif
