#include <gfxtuner/dialog_windows/sensor_props_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/canonical_path.hpp>
#include <utility/msgstream.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <map>

namespace dialog_windows {


sensor_props_dialog::sensor_props_dialog(program_window* const  wnd, scn::sensor_props const&  current_props)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_ok(false)
    , m_widget_ok(
            [](sensor_props_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(sensor_props_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )
    , m_sensor_kind_combobox(
            [](sensor_props_dialog* wnd) {
                    struct s : public QComboBox {
                        s(sensor_props_dialog* wnd) : QComboBox()
                        {
                            QObject::connect(this, SIGNAL(currentIndexChanged()), wnd, SLOT(on_kind_combo_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_property_map_table(new QTableWidget(1, 2))

    , m_current_props(current_props)
    , m_new_props(current_props)
    , m_all_props(ai::sensor::default_configs())
{
    {
        auto&  props = m_all_props.at(m_current_props.m_sensor_kind);
        for (auto const&  elem : m_current_props.m_sensor_props)
            props.at(elem.first) = elem.second;
    }

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const kind_layout = new QHBoxLayout;
        {
            QLabel* const  kind_label = new QLabel("Sensor kind");
            std::string  descriptions_of_kinds = "Defines a kind of the cortex, i.e. sensor's behaviour. These kinds are available:\n";
            for (natural_8_bit i = 0U; i != ai::num_sensor_kinds(); ++i)
            {
                ai::SENSOR_KIND const  kind = ai::as_sensor_kind(i);
                descriptions_of_kinds += "    " + ai::as_string(kind) + ": " + ai::description(kind) + "\n";
            }
            kind_label->setToolTip(descriptions_of_kinds.c_str());
            kind_layout->addWidget(kind_label);

            for (natural_8_bit i = 0U; i != ai::num_sensor_kinds(); ++i)
            {
                std::string const  kind_name = ai::as_string(ai::as_sensor_kind(i));
                m_sensor_kind_combobox->addItem(kind_name.c_str());
            }
            std::string const  current_kind_name = ai::as_string(m_current_props.m_sensor_kind);
            m_sensor_kind_combobox->setCurrentText(current_kind_name.c_str());
            kind_layout->addWidget(m_sensor_kind_combobox);

            kind_layout->addStretch(1);
        }
        dlg_layout->addLayout(kind_layout);

        load_property_map_table();
        dlg_layout->addWidget(m_property_map_table);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](sensor_props_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(sensor_props_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("sensor");
}


void  sensor_props_dialog::accept()
{
    m_new_props.m_sensor_kind = read_kind_combo();
    save_property_map_table();
    m_new_props.m_sensor_props = m_all_props.at(m_new_props.m_sensor_kind);

    m_ok = true;
    QDialog::accept();
}


void  sensor_props_dialog::reject()
{
    QDialog::reject();
}


ai::SENSOR_KIND  sensor_props_dialog::read_kind_combo() const
{
    return ai::as_sensor_kind(qtgl::to_string(m_sensor_kind_combobox->currentText()));
}


void  sensor_props_dialog::on_kind_combo_changed(int)
{
    save_property_map_table();
    m_new_props.m_sensor_kind = read_kind_combo();
    load_property_map_table();
}


void  sensor_props_dialog::load_property_map_table()
{
    auto const&  _ = m_all_props.at(m_new_props.m_sensor_kind);
    std::map<ai::property_map::property_name, ai::property_map::property_type_and_value> const  props(_.begin(), _.end());
    m_property_map_table->clear();
    m_property_map_table->setRowCount((int)props.size());
    int  row = 0;
    for (auto const&  elem : props)
    {
        QTableWidgetItem*  name_item = m_property_map_table->item(row, 0);
        if (name_item == nullptr)
        {
            m_property_map_table->setItem(row, 0, new QTableWidgetItem);
            name_item = m_property_map_table->item(row, 0);
        }
        QTableWidgetItem*  value_item = m_property_map_table->item(row, 1);
        if (value_item == nullptr)
        {
            m_property_map_table->setItem(row, 1, new QTableWidgetItem);
            value_item = m_property_map_table->item(row, 1);
        }

        name_item->setText(elem.first.c_str());
        std::string const  value_text = ai::as_string(elem.second);
        value_item->setText(value_text.c_str());

        ++row;
    }
}


void  sensor_props_dialog::save_property_map_table()
{
    auto&  props = m_all_props.at(m_new_props.m_sensor_kind);
    INVARIANT(m_property_map_table->rowCount() == props.size());
    for (int  row = 0; row != m_property_map_table->rowCount(); ++row)
    {
        QTableWidgetItem* const  name_item = m_property_map_table->item(row, 0);
        QTableWidgetItem* const  value_item = m_property_map_table->item(row, 1);
        auto&  type_and_value = props.at(qtgl::to_string(name_item->text()));
        type_and_value = ai::as_property_type_and_value(type_and_value.get_type(), qtgl::to_string(value_item->text()));
    }
}


}
