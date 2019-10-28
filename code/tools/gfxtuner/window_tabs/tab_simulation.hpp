#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SIMULATION_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_SIMULATION_HPP_INCLUDED

#   include <QWidget>
#   include <QCheckBox>
#   include <QLineEdit>
#   include <QRadioButton>


struct  program_window;


namespace window_tabs { namespace tab_simulation {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QCheckBox*  get_start_paused() const { return m_start_paused; }
    QLineEdit*  get_fixed_frequency() const { return m_fixed_frequency; }
    QLineEdit*  get_min_frequency() const { return  m_min_frequency; }
    QRadioButton* get_fixed_frequency_never() const { return m_fixed_frequency_never; }

    void  on_simulator_started();
    void  on_fixed_frequency_changed();
    void  on_min_frequency_changed();

    void  save();

private:

    program_window*  m_wnd;

    QCheckBox*  m_start_paused;
    QLineEdit*  m_fixed_frequency;
    QLineEdit*  m_min_frequency;
    QRadioButton*  m_fixed_frequency_never;
};


QWidget*  make_simulation_tab_content(widgets const&  w);


}}

#endif
