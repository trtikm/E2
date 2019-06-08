#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_COLLISIONS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_COLLISIONS_HPP_INCLUDED

#   include <QWidget>
#   include <QLabel>
#   include <string>


struct  program_window;


namespace window_tabs { namespace tab_statistics { namespace tab_collisions {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    void  on_time_event();

    QLabel* num_capsules() const { return m_num_capsules; }
    QLabel* num_lines() const { return m_num_lines; }
    QLabel* num_points() const { return m_num_points; }
    QLabel* num_spheres() const { return m_num_spheres; }
    QLabel* num_triangles() const { return m_num_triangles; }
    QLabel* num_compute_contacts_calls_in_last_frame() const { return m_num_compute_contacts_calls_in_last_frame; }
    QLabel* max_num_compute_contacts_calls_till_last_frame() const { return m_max_num_compute_contacts_calls_till_last_frame; }

private:

    program_window*  m_wnd;

    QLabel* m_num_capsules;
    QLabel* m_num_lines;
    QLabel* m_num_points;
    QLabel* m_num_spheres;
    QLabel* m_num_triangles;
    QLabel* m_num_compute_contacts_calls_in_last_frame;
    QLabel* m_max_num_compute_contacts_calls_till_last_frame;
};


std::string  get_tooltip();
QWidget*  make_tab_content(widgets const&  w);


}}}

#endif
