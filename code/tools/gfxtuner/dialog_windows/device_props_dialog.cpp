#include <gfxtuner/dialog_windows/device_props_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <utility/lock_bool.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <map>
#include <algorithm>

namespace dialog_windows {


device_props_dialog::device_props_dialog(
        program_window* const  wnd,
        scn::device_props const&  current_props,
        std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> > const&  sensor_nodes_and_kinds
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_ok(false)
    , m_widget_ok(
            [](device_props_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(device_props_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )
    , m_device_kind_combobox(
            [](device_props_dialog* wnd) {
                    struct s : public QComboBox {
                        s(device_props_dialog* wnd) : QComboBox()
                        {
                            QObject::connect(this, SIGNAL(currentIndexChanged(int)), wnd, SLOT(on_device_kind_combo_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_record_id_list(
            [](device_props_dialog* wnd) {
                    struct s : public QListWidget {
                        s(device_props_dialog* wnd) : QListWidget()
                        {
                            QObject::connect(this, SIGNAL(currentRowChanged(int)),
                                             wnd, SLOT(on_sensor_record_id_list_selection_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_list(
            [](device_props_dialog* wnd) {
                    struct s : public QListWidget {
                        s(device_props_dialog* wnd) : QListWidget()
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
            [](device_props_dialog* wnd) {
                    struct s : public QComboBox {
                        s(device_props_dialog* wnd) : QComboBox()
                        {
                            QObject::connect(this, SIGNAL(currentIndexChanged(int)), wnd, SLOT(on_sensor_action_kind_combo_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_insert_button(
            [](device_props_dialog* wnd) {
                    struct s : public QPushButton {
                        s(device_props_dialog* wnd) : QPushButton("Insert")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_sensor_action_kind_insert_button_pressed()));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_kind_delete_button(
            [](device_props_dialog* wnd) {
                    struct s : public QPushButton {
                        s(device_props_dialog* wnd) : QPushButton("Delete")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(on_sensor_action_kind_delete_button_pressed()));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_sensor_action_props_table(
            [](device_props_dialog* wnd) {
                    struct s : public QTableWidget {
                        s(device_props_dialog* wnd) : QTableWidget(1,2)
                        {
                            QObject::connect(this, SIGNAL(itemChanged(QTableWidgetItem*)),
                                             wnd, SLOT(on_sensor_action_props_table_changed(QTableWidgetItem*)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_locked(false)

    , m_current_props(current_props)
    , m_new_props(current_props)
    , m_sensor_nodes_and_kinds(sensor_nodes_and_kinds)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const kind_layout = new QHBoxLayout;
        {
            QLabel* const  kind_label = new QLabel("Device kind");
            std::string  descriptions_of_kinds = "Defines a kind of the cortex, i.e. device's behaviour. These kinds are available:\n";
            for (natural_8_bit i = 0U; i != ai::num_device_kinds(); ++i)
            {
                ai::DEVICE_KIND const  kind = ai::as_device_kind(i);
                descriptions_of_kinds += "    " + ai::as_string(kind) + ": " + ai::description(kind) + "\n";
            }
            kind_label->setToolTip(descriptions_of_kinds.c_str());
            kind_layout->addWidget(kind_label);

            for (natural_8_bit i = 0U; i != ai::num_device_kinds(); ++i)
            {
                std::string const  kind_name = ai::as_string(ai::as_device_kind(i));
                m_device_kind_combobox->addItem(kind_name.c_str());
            }
            std::string const  current_kind_name = ai::as_string(m_current_props.m_device_kind);
            m_device_kind_combobox->setCurrentText(current_kind_name.c_str());
            kind_layout->addWidget(m_device_kind_combobox);

            kind_layout->addStretch(1);
        }
        dlg_layout->addLayout(kind_layout);

        QHBoxLayout* const sensor_action_layout = new QHBoxLayout;
        {
            m_sensor_record_id_list->setSortingEnabled(false);
            for (auto const&  node_and_kind : sensor_nodes_and_kinds)
            {
                std::string const  text = msgstream() << ::as_string(node_and_kind.first)
                                                      << " [" << ai::as_string(node_and_kind.second) << "]";
                m_sensor_record_id_list->addItem(new QListWidgetItem(text.c_str()));
            }
            m_sensor_record_id_list->setCurrentRow(0);
            sensor_action_layout->addWidget(m_sensor_record_id_list);

            QVBoxLayout* const  sensor_action_kind_layout = new QVBoxLayout;
            {
                sensor_action_kind_layout->addWidget(m_sensor_action_kind_list);
                QHBoxLayout* const  sensor_action_kind_combo_and_buttons_layout = new QHBoxLayout;
                {
                    std::vector<std::string>  sensor_kind_names;
                    for (auto const&  elem : ai::default_sensor_action_props())
                        sensor_kind_names.push_back(ai::as_string(elem.first));
                    std::sort(sensor_kind_names.begin(), sensor_kind_names.end());
                    for (auto const&  sensor_kind_name : sensor_kind_names)
                        m_sensor_action_kind_combobox->addItem(sensor_kind_name.c_str());
                    m_device_kind_combobox->setCurrentText(sensor_kind_names.begin()->c_str());
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_kind_combobox);
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_kind_insert_button);
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_kind_delete_button);

                    sensor_action_kind_combo_and_buttons_layout->addStretch(1);
                }
                sensor_action_kind_layout->addLayout(sensor_action_kind_combo_and_buttons_layout);

                //sensor_action_kind_layout->addStretch(1);
            }
            sensor_action_layout->addLayout(sensor_action_kind_layout);

            sensor_action_layout->addWidget(m_sensor_action_props_table);
            on_sensor_record_id_list_selection_changed();

            //sensor_action_layout->addStretch(1);
        }
        dlg_layout->addLayout(sensor_action_layout);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](device_props_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(device_props_dialog* wnd) : QPushButton("Cancel")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(reject()));
                        }
                    };
                    return new Close(wnd);
                }(this)
                );
            buttons_layout->addStretch(1);
        }
        dlg_layout->addLayout(buttons_layout);
    }
    this->setLayout(dlg_layout);
    this->setWindowTitle("device");
}


void  device_props_dialog::accept()
{
    m_new_props.m_device_kind = ai::as_device_kind(qtgl::to_string(m_device_kind_combobox->currentText()));
    m_new_props.m_skeleton_props = m_current_props.m_skeleton_props;

    m_ok = true;
    QDialog::accept();
}


void  device_props_dialog::reject()
{
    QDialog::reject();
}


void  device_props_dialog::on_device_kind_combo_changed(int)
{
    // Nothing to do, for now.
}


void  device_props_dialog::on_sensor_record_id_list_selection_changed(int)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        m_sensor_action_kind_list->clear();
        m_sensor_action_props_table->clear();
        if (m_sensor_record_id_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = m_new_props.m_sensor_action_map.find(id);
        if (it == m_new_props.m_sensor_action_map.end())
            return;
        for (ai::sensor_action  action : it->second)
        {
            std::string const  text = ai::as_string(action.kind);
            m_sensor_action_kind_list->addItem(new QListWidgetItem(text.c_str()));
        }
        if (m_sensor_action_kind_list->count() > 0)
            m_sensor_action_kind_list->setCurrentRow(0);
    LOCK_BOOL_BLOCK_END();
    on_sensor_action_kind_list_selection_changed();
}


void  device_props_dialog::on_sensor_action_kind_list_selection_changed(int)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        m_sensor_action_props_table->clear();
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        ai::sensor_action const&  action = m_new_props.m_sensor_action_map.at(id).at(m_sensor_action_kind_list->currentRow());
        auto const& _ = action.props;
        std::map<ai::property_map::property_name, ai::property_map::property_type_and_value> const  props(_.begin(), _.end());
        m_sensor_action_props_table->setRowCount((int)props.size());
        int  row = 0;
        for (auto const& elem : props)
        {
            QTableWidgetItem* name_item = m_sensor_action_props_table->item(row, 0);
            if (name_item == nullptr)
            {
                m_sensor_action_props_table->setItem(row, 0, new QTableWidgetItem);
                name_item = m_sensor_action_props_table->item(row, 0);
            }
            QTableWidgetItem* value_item = m_sensor_action_props_table->item(row, 1);
            if (value_item == nullptr)
            {
                m_sensor_action_props_table->setItem(row, 1, new QTableWidgetItem);
                value_item = m_sensor_action_props_table->item(row, 1);
            }

            name_item->setText(elem.first.c_str());
            std::string const  value_text = ai::as_string(elem.second);
            value_item->setText(value_text.c_str());

            ++row;
        }
    LOCK_BOOL_BLOCK_END();
}


void  device_props_dialog::on_sensor_action_kind_combo_changed(int)
{
    // Nothing to do actually.
}


void  device_props_dialog::on_sensor_action_kind_insert_button_pressed()
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto  it = m_new_props.m_sensor_action_map.find(id);
        if (it == m_new_props.m_sensor_action_map.end())
            it = m_new_props.m_sensor_action_map.insert({ id, {} }).first;
        std::string const  action_kind_name = qtgl::to_string(m_sensor_action_kind_combobox->currentText());
        ai::SENSOR_ACTION_KIND  action_kind = ai::as_sensor_action_kind(action_kind_name);
        it->second.push_back({ action_kind, ai::default_sensor_action_props().at(action_kind) });
    LOCK_BOOL_BLOCK_END();
    on_sensor_record_id_list_selection_changed();
}


void  device_props_dialog::on_sensor_action_kind_delete_button_pressed()
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const& id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = m_new_props.m_sensor_action_map.find(id);
        if (it == m_new_props.m_sensor_action_map.end())
            return;
        it->second.erase(std::next(it->second.begin(), m_sensor_action_kind_list->currentRow()));
    LOCK_BOOL_BLOCK_END();
    on_sensor_record_id_list_selection_changed();
}



void  device_props_dialog::on_sensor_action_props_table_changed(QTableWidgetItem*)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        if (m_sensor_record_id_list->count() == 0 || m_sensor_action_kind_list->count() == 0)
            return;
        scn::scene_record_id const&  id = m_sensor_nodes_and_kinds.at(m_sensor_record_id_list->currentRow()).first;
        auto const  it = m_new_props.m_sensor_action_map.find(id);
        if (it == m_new_props.m_sensor_action_map.end())
            return;
        int const  row = m_sensor_action_props_table->currentRow();
        int const  column = m_sensor_action_props_table->currentColumn();
        if (column != 1)
            return;
        QTableWidgetItem* const  name_item = m_sensor_action_props_table->item(row, 0);
        QTableWidgetItem* const  value_item = m_sensor_action_props_table->item(row, 1);
        std::string const  name = qtgl::to_string(name_item->text());
        int const kind_row = m_sensor_action_kind_list->currentRow();
        auto&  type_and_value = it->second.at(kind_row).props.at(name);
        type_and_value = ai::as_property_type_and_value(type_and_value.get_type(), qtgl::to_string(value_item->text()));
    LOCK_BOOL_BLOCK_END();
}


}
