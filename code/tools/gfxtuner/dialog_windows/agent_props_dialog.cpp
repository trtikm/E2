#include <gfxtuner/dialog_windows/agent_props_dialog.hpp>
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

namespace dialog_windows {


agent_props_dialog::agent_props_dialog(
        program_window* const  wnd,
        scn::agent_props const&  current_props,
        std::vector<std::pair<scn::scene_record_id, ai::SENSOR_KIND> > const&  sensor_nodes_and_kinds
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_ok(false)
    , m_widget_ok(
            [](agent_props_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(agent_props_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )
    , m_agent_kind_combobox(new QComboBox)

    , m_current_props(current_props)
    , m_new_props(current_props)
    , m_sensor_nodes_and_kinds(sensor_nodes_and_kinds)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const kind_layout = new QHBoxLayout;
        {
            QLabel* const  kind_label = new QLabel("Agent/cortex kind");
            std::string  descriptions_of_kinds = "Defines a kind of the cortex, i.e. agent's behaviour. These kinds are available:\n";
            for (natural_8_bit i = 0U; i != ai::num_agent_kinds(); ++i)
            {
                ai::AGENT_KIND const  kind = ai::as_agent_kind(i);
                descriptions_of_kinds += "    " + ai::as_string(kind) + ": " + ai::description(kind) + "\n";
            }
            kind_label->setToolTip(descriptions_of_kinds.c_str());
            kind_layout->addWidget(kind_label);

            for (natural_8_bit i = 0U; i != ai::num_agent_kinds(); ++i)
            {
                std::string const  kind_name = ai::as_string(ai::as_agent_kind(i));
                m_agent_kind_combobox->addItem(kind_name.c_str());
            }
            std::string const  current_kind_name = ai::as_string(m_current_props.m_agent_kind);
            m_agent_kind_combobox->setCurrentText(current_kind_name.c_str());
            kind_layout->addWidget(m_agent_kind_combobox);

            kind_layout->addStretch(1);
        }
        dlg_layout->addLayout(kind_layout);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](agent_props_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(agent_props_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("Agent");
}


void  agent_props_dialog::accept()
{
    m_new_props.m_agent_kind = ai::as_agent_kind(qtgl::to_string(m_agent_kind_combobox->currentText()));
    m_new_props.m_skeleton_props = m_current_props.m_skeleton_props;

    m_ok = true;
    QDialog::accept();
}


void  agent_props_dialog::reject()
{
    QDialog::reject();
}


}
