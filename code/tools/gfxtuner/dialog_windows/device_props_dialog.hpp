#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_DEVICE_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_DEVICE_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <gfxtuner/dialog_windows/property_map_widget_utils.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/records/device/device.hpp>
#   include <ai/device_kind.hpp>
#   include <ai/sensor_kind.hpp>
#   include <ai/sensor_action.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QComboBox>

namespace dialog_windows {


struct  device_props_dialog : public QDialog
{
    device_props_dialog(
            program_window* const  wnd,
            scn::device_props const&  current_props,
            std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> > const&  sensor_nodes_and_kinds
            );

    bool  ok() const { return m_ok; }

    scn::device_props const&  get_new_props() const { return m_new_props; }

public slots:

    void  accept();
    void  reject();

    void  on_device_kind_combo_changed(int = 0);

private:

    Q_OBJECT

    program_window*  m_wnd;
    bool  m_ok;
    QPushButton* m_widget_ok;
    QComboBox*  m_device_kind_combobox;

    scn::device_props  m_current_props;
    scn::device_props  m_new_props;

    sensor_action_editor  m_sensor_action_editor;
};


}

#endif
