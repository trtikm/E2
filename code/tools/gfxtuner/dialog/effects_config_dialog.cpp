#include <gfxtuner/dialog/effects_config_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
//#include <QCheckBox>
//#include <QComboBox>
//#include <QLineEdit>
//#include <QGroupBox>
#include <QString>

namespace dialog {


effects_config_dialog::effects_config_dialog(
        program_window* const  wnd,
        qtgl::effects_config_data const&  old_effects_data,
        std::string const&  old_skin_name,
        std::vector<std::string> const&  available_skin_names
        )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_ok(false)
    , m_widget_ok(
            [](effects_config_dialog* wnd) {
                    struct OK : public QPushButton {
                        OK(effects_config_dialog* wnd) : QPushButton("OK")
                        {
                            QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                        }
                    };
                    return new OK(wnd);
                }(this)
            )

    , m_old_effects_data(old_effects_data)
    , m_old_skin_name(old_skin_name)
    , m_new_effects_data(old_effects_data)
    , m_new_skin_name(old_skin_name)
    , m_available_skin_names(available_skin_names)
{
    ASSUMPTION(std::find(m_available_skin_names.cbegin(), m_available_skin_names.cend(), m_old_skin_name) != m_available_skin_names.cend());

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        dlg_layout->addWidget(new QLabel("TODO!"));

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_widget_ok);
            buttons_layout->addWidget(
                [](effects_config_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(effects_config_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("Batch");

    m_widget_ok->setEnabled(false);
}


void  effects_config_dialog::accept()
{
    m_ok = true;
    QDialog::accept();
}


void  effects_config_dialog::reject()
{
    QDialog::reject();
}


}
