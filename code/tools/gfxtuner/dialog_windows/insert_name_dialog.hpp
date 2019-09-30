#ifndef E2_TOOL_GFXTUNER_DIALOG_WINDOWS_INSERT_NAME_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_DIALOG_WINDOWS_INSERT_NAME_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <QDialog>
#   include <QLineEdit>
#   include <QLabel>
#   include <QPushButton>
#   include <functional>
#   include <string>

namespace dialog_windows {


struct  insert_name_dialog : public QDialog
{
    insert_name_dialog(
            program_window* const  wnd,
            std::string const&  initial_name,
            std::function<bool(std::string const&)> const&  is_name_valid
            );

    std::string const&  get_name() const { return m_name; }

    void  on_name_changed(QString const&  qname);

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    QLineEdit*  m_name_edit;
    QLabel*  m_name_taken_indicator;
    QPushButton*  m_OK_button;
    std::function<bool(std::string const&)>  m_is_name_valid_function;
    std::string  m_name;
};


bool  is_scene_forbidden_name(std::string const&  name);


}

#endif
