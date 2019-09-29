#include <gfxtuner/dialog/effects_config_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
//#include <QCheckBox>
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

    , m_skins_combo_box(new QComboBox)
{
    ASSUMPTION(std::find(m_available_skin_names.cbegin(), m_available_skin_names.cend(), m_old_skin_name) != m_available_skin_names.cend());

    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        QHBoxLayout* const skin_layout = new QHBoxLayout;
        {
            QLabel* const  skin_label = new QLabel("Skin");
            skin_label->setToolTip(
                "A 'skin' is a pre-defined collection of textures and\n"
                "alpha-blending/testing setup. Each 'batch' contains\n"
                "at least one skin, each uniquely identified by a name.\n"
                "Exactly one of the skin names must be 'default'.");
            skin_layout->addWidget(skin_label);

            for (auto const&  skin_name : m_available_skin_names)
                m_skins_combo_box->addItem(skin_name.c_str());
            m_skins_combo_box->setCurrentIndex(
                    std::distance(
                            m_available_skin_names.cbegin(),
                            std::find(m_available_skin_names.cbegin(), m_available_skin_names.cend(), m_old_skin_name)
                            )
                    );
            skin_layout->addWidget(m_skins_combo_box);

            skin_layout->addStretch(1);
        }
        dlg_layout->addLayout(skin_layout);


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
    this->setWindowTitle("Effects config");
}


void  effects_config_dialog::accept()
{
    m_new_skin_name = m_available_skin_names.at(m_skins_combo_box->currentIndex());

    m_ok = true;
    QDialog::accept();
}


void  effects_config_dialog::reject()
{
    QDialog::reject();
}


}
