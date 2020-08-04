#include <gfxtuner/dialog_windows/property_map_widget_utils.hpp>
#include <aiold/sensor.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/lock_bool.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <string>
#include <map>
#include <algorithm>

namespace dialog_windows {


void  on_property_map_table_changed(QTableWidget* const  table, aiold::property_map&  props)
{
    int const  row = table->currentRow();
    int const  column = table->currentColumn();
    QTableWidgetItem* const  name_item = table->item(row, 0);
    QTableWidgetItem* const  value_item = table->item(row, 1);
    if (name_item == nullptr || value_item == nullptr)
        return;
    std::string const  name = qtgl::to_string(name_item->text());
    std::string const  value = qtgl::to_string(value_item->text());
    if (name == "TODO" || value == "TODO")
        return;
    if (props.has(name))
        props.set(name, as_property_map_value(props.get_type(name), value));
    else if (value.empty())
        props.set(name, as_property_map_value(aiold::property_map::PROPERTY_TYPE::STRING, value));
    else if (value.front() == '+' || value.front() == '-' || std::isdigit(value.front()))
    {
        if (value.find('.') == std::string::npos)
            props.set(name, as_property_map_value(aiold::property_map::PROPERTY_TYPE::INT, value));
        else
            props.set(name, as_property_map_value(aiold::property_map::PROPERTY_TYPE::FLOAT, value));
    }
    else
        props.set(name, as_property_map_value(aiold::property_map::PROPERTY_TYPE::STRING, value));
}


void  rebuild_property_map_table(
        QTableWidget* const  table,
        aiold::property_map&  props,
        aiold::property_map::default_config_records_map const&  default_content,
        bool const  reset_to_defaults
        )
{
    if (reset_to_defaults)
        props.reset(default_content);

    std::map<natural_32_bit, std::string>  map;
    for (auto const&  name_and_record : default_content)
        if (reset_to_defaults || name_and_record.second.is_mandatory || props.has(name_and_record.first))
            map.insert({name_and_record.second.edit_order, name_and_record.first});
    natural_32_bit  order = map.empty() ? 0U : map.rbegin()->first;
    for (auto const&  name_and_value : props)
        if (default_content.count(name_and_value.first) == 0UL)
            map.insert({ ++order, name_and_value.first });

    table->clear();
    table->setRowCount((int)map.size());
    int  row = 0;
    for (auto const&  order_and_name : map)
    {
        QTableWidgetItem*  name_item = table->item(row, 0);
        if (name_item == nullptr)
        {
            table->setItem(row, 0, new QTableWidgetItem);
            name_item = table->item(row, 0);
        }
        QTableWidgetItem*  value_item = table->item(row, 1);
        if (value_item == nullptr)
        {
            table->setItem(row, 1, new QTableWidgetItem);
            value_item = table->item(row, 1);
        }

        name_item->setText(order_and_name.second.c_str());

        auto const  record_it = default_content.find(order_and_name.second);

        if (record_it != default_content.end())
        {
            std::string const  tool_tip = record_it->second.description + (record_it->second.is_mandatory ? "\nNOTE: This is mandatory property." : "");
            name_item->setToolTip(tool_tip.c_str());
        }

        std::string const  value_text = as_string(
                props.has(order_and_name.second) ? ((aiold::property_map const&)props).at(order_and_name.second) : *record_it->second.value
                );
        value_item->setText(value_text.c_str());

        ++row;
    }
}


void  rebuild_sensor_property_map_table(
        QTableWidget* const  table,
        aiold::property_map&  props,
        aiold::SENSOR_KIND const  kind,
        bool const  reset_to_defaults
        )
{
    rebuild_property_map_table(table, props, aiold::default_sensor_configs().at(kind), reset_to_defaults);
}



sensor_action_editor::sensor_action_editor(
        aiold::from_sensor_record_to_sensor_action_map* const  output_sensor_action_map_,
        std::vector<std::pair<scn::scene_record_id, aiold::SENSOR_KIND> > const&  sensor_nodes_and_kinds_
        )
    : QWidget()
    , output_sensor_action_map(output_sensor_action_map_)
    , m_sensor_record_id_list(
            [](sensor_action_editor* wnd) {
                    struct s : public QListWidget {
                        s(sensor_action_editor* wnd) : QListWidget()
                        {
                            QObject::connect(this, SIGNAL(currentRowChanged(int)),
                                             wnd, SLOT(on_sensor_record_id_list_selection_changed(int)));
                            setSortingEnabled(false);
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_list(
            [](sensor_action_editor* wnd) {
                    struct s : public QListWidget {
                        s(sensor_action_editor* wnd) : QListWidget()
                        {
                            QObject::connect(this, SIGNAL(currentRowChanged(int)),
                                             wnd, SLOT(on_sensor_action_kind_list_selection_changed(int)));
                            setSortingEnabled(false);
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_combobox(
            [](sensor_action_editor* wnd) {
                    struct s : public QComboBox {
                        s(sensor_action_editor* wnd) : QComboBox()
                        {
                            QObject::connect(this, SIGNAL(currentIndexChanged(int)), wnd, SLOT(on_sensor_action_kind_combo_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_insert_button(
            [](sensor_action_editor* wnd) {
                    struct s : public QPushButton {
                        s(sensor_action_editor* wnd) : QPushButton("Insert")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_sensor_action_kind_insert_button_pressed()));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_delete_button(
            [](sensor_action_editor* wnd) {
                    struct s : public QPushButton {
                        s(sensor_action_editor* wnd) : QPushButton("Delete")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_sensor_action_kind_delete_button_pressed()));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_props_table(
            [](sensor_action_editor* wnd) {
                    struct s : public QTableWidget {
                        s(sensor_action_editor* wnd) : QTableWidget(1,2)
                        {
                            QObject::connect(this, SIGNAL(itemChanged(QTableWidgetItem*)),
                                             wnd, SLOT(on_sensor_action_props_table_changed(QTableWidgetItem*)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_locked(false)

    , m_sensor_nodes_and_kinds(sensor_nodes_and_kinds_)
{
    m_sensor_action_kind_insert_button->setAutoDefault(false);
    m_sensor_action_kind_insert_button->setDefault(false);
    m_sensor_action_kind_delete_button->setAutoDefault(false);
    m_sensor_action_kind_delete_button->setDefault(false);

    std::sort(m_sensor_nodes_and_kinds.begin(), m_sensor_nodes_and_kinds.end(),
        [](std::pair<scn::scene_record_id, aiold::SENSOR_KIND> const&  left,
           std::pair<scn::scene_record_id, aiold::SENSOR_KIND> const&  right) -> bool {
            return left.first < right.first;
        });

    for (auto const& node_and_kind : m_sensor_nodes_and_kinds)
    {
        std::string const  text = msgstream() << ::as_string(node_and_kind.first)
            << " [" << as_string(node_and_kind.second) << "]";
        m_sensor_record_id_list->addItem(new QListWidgetItem(text.c_str()));
    }
    m_sensor_record_id_list->setCurrentRow(0);

    std::vector<std::string>  sensor_kind_names;
    for (auto const& elem : aiold::default_sensor_action_configs())
        sensor_kind_names.push_back(as_string(elem.first));
    std::sort(sensor_kind_names.begin(), sensor_kind_names.end());
    for (auto const& sensor_kind_name : sensor_kind_names)
        m_sensor_action_kind_combobox->addItem(sensor_kind_name.c_str());

    on_sensor_record_id_list_selection_changed();
}


void  sensor_action_editor::on_sensor_record_id_list_selection_changed(int)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        m_sensor_action_kind_list->clear();
        m_sensor_action_props_table->clear();
        if (m_sensor_record_id_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = output_sensor_action_map->find(id);
        if (it == output_sensor_action_map->end())
            return;
        for (aiold::sensor_action  action : it->second)
        {
            std::string const  text = as_string(action.kind);
            std::string const  tool_tip = aiold::description(action.kind);
            QListWidgetItem* const  item(new QListWidgetItem(text.c_str()));
            item->setToolTip(tool_tip.c_str());
            m_sensor_action_kind_list->addItem(item);
        }
        if (m_sensor_action_kind_list->count() > 0)
            m_sensor_action_kind_list->setCurrentRow(0);
    LOCK_BOOL_BLOCK_END();
    on_sensor_action_kind_list_selection_changed();
}


void  sensor_action_editor::on_sensor_action_kind_list_selection_changed(int)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        m_sensor_action_props_table->clear();
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        aiold::sensor_action&  action = output_sensor_action_map->at(id).at(m_sensor_action_kind_list->currentRow());
        rebuild_property_map_table(
            m_sensor_action_props_table,
            action.props,
            aiold::default_sensor_action_configs().at(action.kind),
            false
            );
    LOCK_BOOL_BLOCK_END();
}


void  sensor_action_editor::on_sensor_action_kind_combo_changed(int)
{
    // Nothing to do actually.
}


void  sensor_action_editor::on_sensor_action_kind_insert_button_pressed()
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        if (m_sensor_record_id_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto  it = output_sensor_action_map->find(id);
        if (it == output_sensor_action_map->end())
            it = output_sensor_action_map->insert({ id, {} }).first;
        std::string const  action_kind_name = qtgl::to_string(m_sensor_action_kind_combobox->currentText());
        aiold::SENSOR_ACTION_KIND  action_kind = as_sensor_action_kind(action_kind_name);
        it->second.push_back({ action_kind, aiold::property_map(aiold::default_sensor_action_configs().at(action_kind)) });
    LOCK_BOOL_BLOCK_END();
    on_sensor_record_id_list_selection_changed();
}


void  sensor_action_editor::on_sensor_action_kind_delete_button_pressed()
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const& id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = output_sensor_action_map->find(id);
        if (it == output_sensor_action_map->end())
            return;
        it->second.erase(std::next(it->second.begin(), m_sensor_action_kind_list->currentRow()));
    LOCK_BOOL_BLOCK_END();
    on_sensor_record_id_list_selection_changed();
}



void  sensor_action_editor::on_sensor_action_props_table_changed(QTableWidgetItem*)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = output_sensor_action_map->find(id);
        if (it == output_sensor_action_map->end())
            return;
        on_property_map_table_changed(m_sensor_action_props_table, it->second.at(m_sensor_action_kind_list->currentRow()).props);
    LOCK_BOOL_BLOCK_END();
}




}
