#ifndef E2_TOOL_GFXTUNER_PROPERTY_MAP_WIDGET_UTILS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_PROPERTY_MAP_WIDGET_UTILS_HPP_INCLUDED

#   include <aiold/property_map.hpp>
#   include <aiold/sensor_kind.hpp>
#   include <aiold/device_kind.hpp>
#   include <aiold/sensor_action.hpp>
#   include <QComboBox>
#   include <QPushButton>
#   include <QListWidget>
#   include <QTableWidget>
#   include <vector>

namespace dialog_windows {


void  on_property_map_table_changed(QTableWidget* const  table, aiold::property_map&  props);


void  rebuild_property_map_table(
        QTableWidget* const  table,
        aiold::property_map&  props,
        aiold::property_map::default_config_records_map const&  default_content,
        bool const  reset_to_defaults = false
        );


void  rebuild_sensor_property_map_table(
        QTableWidget* const  table,
        aiold::property_map&  props,
        aiold::SENSOR_KIND const  kind,
        bool const  reset_to_defaults = false
        );


struct  sensor_action_editor : public QWidget
{
    explicit sensor_action_editor(
            aiold::from_sensor_record_to_sensor_action_map* const  output_sensor_action_map_,
            std::vector<std::pair<scn::scene_record_id, aiold::SENSOR_KIND> > const&  sensor_nodes_and_kinds_
            );

public slots:

    void  on_sensor_record_id_list_selection_changed(int = 0);
    void  on_sensor_action_kind_list_selection_changed(int = 0);
    void  on_sensor_action_kind_combo_changed(int = 0);
    void  on_sensor_action_kind_insert_button_pressed();
    void  on_sensor_action_kind_delete_button_pressed();
    void  on_sensor_action_props_table_changed(QTableWidgetItem* = nullptr);

public:

    aiold::from_sensor_record_to_sensor_action_map*  output_sensor_action_map;

    QListWidget*  m_sensor_record_id_list;
    QListWidget*  m_sensor_action_kind_list;
    QComboBox*  m_sensor_action_kind_combobox;
    QPushButton*  m_sensor_action_kind_insert_button;
    QPushButton*  m_sensor_action_kind_delete_button;
    QTableWidget*  m_sensor_action_props_table;

    std::vector<std::pair<scn::scene_record_id, aiold::SENSOR_KIND> >  m_sensor_nodes_and_kinds;

private:
    Q_OBJECT

    bool  m_locked;
};


}

#endif
