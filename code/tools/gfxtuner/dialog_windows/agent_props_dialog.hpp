#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_AGENT_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_AGENT_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/records/agent/agent.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QComboBox>

namespace dialog_windows {


struct  agent_props_dialog : public QDialog
{
    agent_props_dialog(program_window* const  wnd, scn::skeleton_props_const_ptr const  current_skeleton_props);

    bool  ok() const { return m_ok; }

    scn::skeleton_props_const_ptr  get_new_skeleton_props() const { return m_new_skeleton_props; }

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    bool  m_ok;
    QPushButton* m_widget_ok;

    scn::skeleton_props_const_ptr  m_current_skeleton_props;
    scn::skeleton_props_const_ptr  m_new_skeleton_props;
};


}

#endif
