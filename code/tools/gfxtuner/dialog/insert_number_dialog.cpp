#include <gfxtuner/dialog/insert_number_dialog.hpp>
#include <qtgl/gui_utils.hpp>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QString>
#include <locale>

namespace dialog {


insert_number_dialog::insert_number_dialog(
            program_window* const  wnd,
            std::string const&  title,
            float_32_bit const  initial_value,
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            bool const  integral_only
            )
    : QDialog(wnd)
    , m_wnd(wnd)
    , m_value_edit(
        [](insert_number_dialog* dlg, float_32_bit const  initial_value, bool const  integral_only) {
            struct s : public QLineEdit {
                s(insert_number_dialog* dlg, float_32_bit const  initial_value, bool const  integral_only) : QLineEdit()
                {
                    if (integral_only)
                        setText(QString(std::to_string((natural_32_bit)initial_value).c_str()));
                    else
                        setText(QString(std::to_string(initial_value).c_str()));
                    QObject::connect(this, &QLineEdit::textChanged, dlg, &insert_number_dialog::on_value_changed);
                }
            };
            return new s(dlg, initial_value, integral_only);
        }(this, initial_value, integral_only)
        )
    , m_OK_button(
        [](insert_number_dialog* wnd) {
            struct Open : public QPushButton {
                Open(insert_number_dialog* wnd) : QPushButton("OK")
                {
                    QObject::connect(this, SIGNAL(released()), wnd, SLOT(accept()));
                    setDefault(true);
                    autoDefault();
                }
            };
            return new Open(wnd);
        }(this)
        )
    , m_title(title)
    , m_value(initial_value)
    , m_initial_value(initial_value)
    , m_min_value(min_value)
    , m_max_value(max_value)
    , m_integral_only(integral_only)
{
    QVBoxLayout* const dlg_layout = new QVBoxLayout;
    {
        dlg_layout->addWidget(m_value_edit);

        QHBoxLayout* const buttons_layout = new QHBoxLayout;
        {
            buttons_layout->addWidget(m_OK_button);
            buttons_layout->addWidget(
                [](insert_number_dialog* wnd) {
                    struct Close : public QPushButton {
                        Close(insert_number_dialog* wnd) : QPushButton("Cancel")
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
    this->setWindowTitle(m_title.c_str());
    this->resize(300,100);

    if (m_integral_only)
        on_value_changed(std::to_string((natural_32_bit)m_initial_value).c_str());
    else
        on_value_changed(std::to_string(m_initial_value).c_str());
}


void  insert_number_dialog::on_value_changed(QString const&  raw_value_text)
{
    std::string const  value_text = qtgl::to_string(raw_value_text);
    {
        if (value_text.empty()) { m_OK_button->setDisabled(true); return; }
        int idx = 0;
        if (value_text.at(idx) != '+' && value_text.at(idx) != '-' && !std::isdigit(value_text.at(idx), std::locale::classic()))
        { m_OK_button->setDisabled(true); return; }
        bool has_dot = value_text.at(idx) == '.';
        for (++idx; idx != value_text.size(); ++idx)
            if (value_text.at(idx) == '.') { if (has_dot){ m_OK_button->setDisabled(true); return; } has_dot = true; }
            else if (!std::isdigit(value_text.at(idx), std::locale::classic())) { m_OK_button->setDisabled(true); return; }
        if (has_dot && m_integral_only) { m_OK_button->setDisabled(true); return; }
    }
    float_32_bit value = std::atof(value_text.c_str());
    m_OK_button->setDisabled(value < m_min_value || value > m_max_value || (m_integral_only && std::floorf(value) != value));
}


void  insert_number_dialog::accept()
{
    std::string const  value_text = qtgl::to_string(m_value_edit->text());
    m_value = std::atof(value_text.c_str());
    QDialog::accept();
}

void  insert_number_dialog::reject()
{
    QDialog::reject();
}


}
