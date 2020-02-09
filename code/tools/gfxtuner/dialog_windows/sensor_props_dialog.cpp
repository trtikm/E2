#include <gfxtuner/dialog_windows/sensor_props_dialog.hpp>
#include <gfxtuner/dialog_windows/property_map_widget_utils.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/lock_bool.hpp>
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
                            QObject::connect(this, SIGNAL(currentIndexChanged(int)), wnd, SLOT(on_kind_combo_changed(int)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_property_map_table(
            [](sensor_props_dialog* wnd) {
                    struct s : public QTableWidget {
                        s(sensor_props_dialog* wnd) : QTableWidget(1, 2)
                        {
                            QObject::connect(this, SIGNAL(itemChanged(QTableWidgetItem*)),
                                             wnd, SLOT(on_props_table_changed(QTableWidgetItem*)));
                        }
                    };
                    return new s(wnd);
                }(this)
            )
    , m_locked(false)

    , m_current_props(current_props)
    , m_new_props(current_props)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const kind_layout = new QHBoxLayout;
        {
            QLabel* const  kind_label = new QLabel("Sensor kind");
            std::string  descriptions_of_kinds =
                "Defines what the sensor senses and/or what events it sends to the owner. These kinds are available:\n";
            for (natural_8_bit i = 0U; i != ai::num_sensor_kinds(); ++i)
            {
                ai::SENSOR_KIND const  kind = ai::as_sensor_kind(i);
                descriptions_of_kinds += "    " + as_string(kind) + ": " + ai::description(kind) + "\n";
            }
            kind_label->setToolTip(descriptions_of_kinds.c_str());
            kind_layout->addWidget(kind_label);

            for (natural_8_bit i = 0U; i != ai::num_sensor_kinds(); ++i)
            {
                std::string const  kind_name = as_string(ai::as_sensor_kind(i));
                m_sensor_kind_combobox->addItem(kind_name.c_str());

            }
            std::string const  current_kind_name = as_string(m_current_props.m_sensor_kind);
            m_sensor_kind_combobox->setCurrentText(current_kind_name.c_str());
            kind_layout->addWidget(m_sensor_kind_combobox);

            kind_layout->addStretch(1);
        }
        dlg_layout->addLayout(kind_layout);

        rebuild_sensor_property_map_table(m_property_map_table, m_new_props.m_sensor_props, m_new_props.m_sensor_kind, false);
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
    m_ok = true;
    QDialog::accept();
}


void  sensor_props_dialog::reject()
{
    QDialog::reject();
}


void  sensor_props_dialog::on_kind_combo_changed(int)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        ai::SENSOR_KIND const  new_kind = read_kind_combo();
        if (new_kind != m_new_props.m_sensor_kind)
        {
            m_new_props.m_sensor_kind = new_kind;
            rebuild_sensor_property_map_table(m_property_map_table, m_new_props.m_sensor_props, new_kind, true);
        }
    LOCK_BOOL_BLOCK_END();
}


void  sensor_props_dialog::on_props_table_changed(QTableWidgetItem*)
{
    LOCK_BOOL_BLOCK_BEGIN(m_locked);
        on_property_map_table_changed(m_property_map_table, m_new_props.m_sensor_props);
    LOCK_BOOL_BLOCK_END();
}


ai::SENSOR_KIND  sensor_props_dialog::read_kind_combo() const
{
    return as_sensor_kind(qtgl::to_string(m_sensor_kind_combobox->currentText()));
}


}
