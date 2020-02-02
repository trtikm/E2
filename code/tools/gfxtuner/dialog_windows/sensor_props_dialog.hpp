#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_SENSOR_PROPS_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_SENSOR_PROPS_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <scene/scene_record_id.hpp>
#   include <scene/scene_node_record_id.hpp>
#   include <scene/records/sensor/sensor.hpp>
#   include <ai/sensor_kind.hpp>
#   include <ai/property_map.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <QDialog>
#   include <QPushButton>
#   include <QComboBox>
#   include <QTableWidget>
#   include <unordered_map>

namespace dialog_windows {


struct  sensor_props_dialog : public QDialog
{
    sensor_props_dialog(program_window* const  wnd, scn::sensor_props const&  current_props);

    bool  ok() const { return m_ok; }

    scn::sensor_props const&  get_new_props() const { return m_new_props; }

public slots:

    void  accept();
    void  reject();

private:

    ai::SENSOR_KIND  read_kind_combo() const;
    void  on_kind_combo_changed(int);
    void  load_property_map_table();
    void  save_property_map_table();

    program_window*  m_wnd;
    bool  m_ok;
    QPushButton* m_widget_ok;
    QComboBox*  m_sensor_kind_combobox;
    QTableWidget*  m_property_map_table;

    scn::sensor_props  m_current_props;
    scn::sensor_props  m_new_props;
    std::unordered_map<ai::SENSOR_KIND, ai::property_map>  m_all_props;
};


}

#endif
