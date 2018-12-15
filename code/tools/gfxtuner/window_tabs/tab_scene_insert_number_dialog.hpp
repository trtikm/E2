#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_INSERT_NUMBER_DIALOG_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SCENE_INSERT_NUMBER_DIALOG_HPP_INCLUDED

#   include <gfxtuner/program_window.hpp>
#   include <utility/basic_numeric_types.hpp>
#   include <QDialog>
#   include <QLineEdit>
#   include <QPushButton>
#   include <string>

namespace window_tabs { namespace tab_scene {


struct  insert_number_dialog : public QDialog
{
    insert_number_dialog(
            program_window* const  wnd,
            std::string const&  title,
            float_32_bit const  initial_value,
            float_32_bit const  min_value,
            float_32_bit const  max_value,
            bool const  integral_only
            );

    float_32_bit const&  get_value() const { return m_value; }

    void  on_value_changed(QString const&  raw_value_text);

public slots:

    void  accept();
    void  reject();

private:
    program_window*  m_wnd;
    QLineEdit*  m_value_edit;
    QPushButton*  m_OK_button;
    std::string  m_title;
    float_32_bit  m_value;
    float_32_bit  m_initial_value;
    float_32_bit  m_min_value;
    float_32_bit  m_max_value;
    bool  m_integral_only;
};


}}

#endif
