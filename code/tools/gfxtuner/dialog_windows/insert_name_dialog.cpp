#include <gfxtuner/dialog_windows/insert_name_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>

namespace dialog_windows {


insert_name_dialog::insert_name_dialog(program_window* const  wnd,
                    std::string const&  initial_name,
                    std::function<bool(std::string const&)> const&  is_name_valid)
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_name_edit(
        [](insert_name_dialog* dlg, std::string const&  initial_name) {
            struct s : public QLineEdit {
                s(insert_name_dialog* dlg, std::string const&  initial_name) : QLineEdit()
                {
                    setText(QString(initial_name.c_str()));
                    QObject::connect(this, &QLineEdit::textChanged, dlg, &insert_name_dialog::on_name_changed);
                }
            };
            return new s(dlg, initial_name);
        }(this, initial_name)
        )
    , m_name_taken_indicator(new QLabel("WARNING: The name has wrong format or it is already in use."))
    , m_OK_button(
        [](insert_name_dialog* wnd) {
            struct Open : public QPushButton {
                Open(insert_name_dialog* wnd) : QPushButton("OK")
                {
                    QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                    setDefault(true);
                    autoDefault();
                }
            };
            return new Open(wnd);
        }(this)
        )
    , m_is_name_valid_function(is_name_valid)
    , m_name()
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        dlg_layout->addWidget(m_name_edit);
        dlg_layout->addWidget(m_name_taken_indicator);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_OK_button);
            buttons_layout->addWidget(
                [](insert_name_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(insert_name_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle("Insert name");
    this->resize(300,100);

    on_name_changed(QString(initial_name.c_str()));
}


void  insert_name_dialog::on_name_changed(QString const&  qname)
{
    if (qname.isEmpty() || !m_is_name_valid_function(qtgl::to_string(qname)))
    {
        m_name_taken_indicator->setText(QString("WARNING: the name is either forbidden or already in use."));
        m_OK_button->setEnabled(false);
    }
    else
    {
        m_name_taken_indicator->setText(QString("OK: The name can be used."));
        m_OK_button->setEnabled(true);
    }
}


void  insert_name_dialog::accept()
{
    m_name = qtgl::to_string(m_name_edit->text());
    QDialog::accept();
}

void  insert_name_dialog::reject()
{
    QDialog::reject();
}


bool  is_scene_forbidden_name(std::string const&  name)
{
    return  name.empty()
            || name == "."
            || name == ".."
            || name == "*"
            || name.front() == '@'
            || name.find('/') != std::string::npos
            || name.find('\\') != std::string::npos
            ;
}


}
