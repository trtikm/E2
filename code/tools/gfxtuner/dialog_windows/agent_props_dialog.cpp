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


agent_props_dialog::agent_props_dialog(program_window* const  wnd, scn::skeleton_props_const_ptr const  current_skeleton_props)
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

    , m_current_skeleton_props(current_skeleton_props)
    , m_new_skeleton_props()
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        dlg_layout->addWidget(new QLabel("TODO!"));

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

    m_widget_ok->setEnabled(false);
}


void  agent_props_dialog::accept()
{
    m_ok = true;
    QDialog::accept();
}


void  agent_props_dialog::reject()
{
    QDialog::reject();
}


}
