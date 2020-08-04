#include <gfxtuner/dialog_windows/device_props_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>

namespace dialog_windows {


device_props_dialog::device_props_dialog(
        program_window* const  wnd,
        scn::device_props const&  current_props,
        std::vector<std::pair<scn::scene_record_id, aiold::SENSOR_KIND> > const&  sensor_nodes_and_kinds
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

    , m_current_props(current_props)
    , m_new_props(current_props)

    , m_sensor_action_editor(&m_new_props.m_sensor_action_map, sensor_nodes_and_kinds)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const kind_layout = new QHBoxLayout;
        {
            QLabel* const  kind_label = new QLabel("Device kind");
            std::string  descriptions_of_kinds = "Defines a kind of the cortex, i.e. device's behaviour. These kinds are available:\n";
            for (natural_8_bit i = 0U; i != aiold::num_device_kinds(); ++i)
            {
                aiold::DEVICE_KIND const  kind = aiold::as_device_kind(i);
                descriptions_of_kinds += "    " + aiold::as_string(kind) + ": " + aiold::description(kind) + "\n";
            }
            kind_label->setToolTip(descriptions_of_kinds.c_str());
            kind_layout->addWidget(kind_label);

            for (natural_8_bit i = 0U; i != aiold::num_device_kinds(); ++i)
            {
                std::string const  kind_name = aiold::as_string(aiold::as_device_kind(i));
                m_device_kind_combobox->addItem(kind_name.c_str());
            }
            std::string const  current_kind_name = aiold::as_string(m_current_props.m_device_kind);
            m_device_kind_combobox->setCurrentText(current_kind_name.c_str());
            kind_layout->addWidget(m_device_kind_combobox);

            kind_layout->addStretch(1);
        }
        dlg_layout->addLayout(kind_layout);

        QHBoxLayout* const sensor_action_layout = new QHBoxLayout;
        {
            sensor_action_layout->addWidget(m_sensor_action_editor.m_sensor_record_id_list);

            QVBoxLayout* const  sensor_action_kind_layout = new QVBoxLayout;
            {
                sensor_action_kind_layout->addWidget(m_sensor_action_editor.m_sensor_action_kind_list);
                QHBoxLayout* const  sensor_action_kind_combo_and_buttons_layout = new QHBoxLayout;
                {
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_editor.m_sensor_action_kind_combobox);
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_editor.m_sensor_action_kind_insert_button);
                    sensor_action_kind_combo_and_buttons_layout->addWidget(m_sensor_action_editor.m_sensor_action_kind_delete_button);

                    sensor_action_kind_combo_and_buttons_layout->addStretch(1);
                }
                sensor_action_kind_layout->addLayout(sensor_action_kind_combo_and_buttons_layout);

                //sensor_action_kind_layout->addStretch(1);
            }
            sensor_action_layout->addLayout(sensor_action_kind_layout);

            //sensor_action_layout->addStretch(1);
        }
        dlg_layout->addLayout(sensor_action_layout);

        dlg_layout->addWidget(m_sensor_action_editor.m_sensor_action_props_table);

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
    m_new_props.m_device_kind = aiold::as_device_kind(qtgl::to_string(m_device_kind_combobox->currentText()));
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


}
