#ifndef E2_TOOL_NETVIEWER_STATUS_BAR_HPP_INCLUDED
#   define E2_TOOL_NETVIEWER_STATUS_BAR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QLabel>


struct  program_window;


struct  status_bar
{
    status_bar(program_window* const  wnd);

    program_window*  wnd() const noexcept;

    QLabel* spent_real_time() const noexcept;
    QLabel* spent_simulation_time() const noexcept;
    QLabel* spent_times_ratio() const noexcept;
    QLabel* num_passed_simulation_steps() const noexcept;
    QLabel* mode() const noexcept;
    QLabel* FPS() const noexcept;

    void  update();

private:
    program_window*  m_wnd;

    QLabel*  m_spent_real_time;
    QLabel*  m_spent_simulation_time;
    QLabel*  m_spent_times_ratio;
    QLabel*  m_num_passed_simulation_steps;
    QLabel*  m_mode;
    QLabel*  m_FPS;
};


void  make_status_bar_content(status_bar const&  w);


#endif
