#ifndef E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_COLLISIONS_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_WINDOW_TABS_TAB_STATISTICS_TAB_COLLISIONS_HPP_INCLUDED

#   include <QWidget>
#   include <QLineEdit>
#   include <string>


struct  program_window;


namespace window_tabs { namespace tab_statistics { namespace tab_collisions {


struct  widgets
{
    widgets(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    void  on_time_event();

    QLineEdit* num_capsules() const { return m_num_capsules; }
    QLineEdit* num_lines() const { return m_num_lines; }
    QLineEdit* num_points() const { return m_num_points; }
    QLineEdit* num_spheres() const { return m_num_spheres; }
    QLineEdit* num_triangles() const { return m_num_triangles; }

    QLineEdit* num_compute_contacts_calls_in_last_frame() const { return m_num_compute_contacts_calls_in_last_frame; }
    QLineEdit* max_num_compute_contacts_calls_till_last_frame() const { return m_max_num_compute_contacts_calls_till_last_frame; }

    QLineEdit* num_contacts_in_last_frame() const { return m_num_contacts_in_last_frame; }
    QLineEdit* max_num_contacts_till_last_frame() const { return m_max_num_contacts_till_last_frame; }

    QLineEdit* proximity_static_num_objects() const { return m_proximity_static_num_objects; }
    QLineEdit* proximity_static_num_split_nodes() const { return m_proximity_static_num_split_nodes; }
    QLineEdit* proximity_static_num_searches_by_bbox_in_last_frame() const { return m_proximity_static_num_searches_by_bbox_in_last_frame; }
    QLineEdit* proximity_static_max_num_searches_by_bbox_till_last_frame() const { return m_proximity_static_max_num_searches_by_bbox_till_last_frame; }
    QLineEdit* proximity_static_num_searches_by_line_in_last_frame() const { return m_proximity_static_num_searches_by_line_in_last_frame; }
    QLineEdit* proximity_static_max_num_searches_by_line_till_last_frame() const { return m_proximity_static_max_num_searches_by_line_till_last_frame; }
    QLineEdit* proximity_static_num_enumerate_calls_in_last_frame() const { return m_proximity_static_num_enumerate_calls_in_last_frame; }
    QLineEdit* proximity_static_max_num_enumerate_calls_till_last_frame() const { return m_proximity_static_max_num_enumerate_calls_till_last_frame; }

    QLineEdit* proximity_dynamic_num_objects() const { return m_proximity_dynamic_num_objects; }
    QLineEdit* proximity_dynamic_num_split_nodes() const { return m_proximity_dynamic_num_split_nodes; }
    QLineEdit* proximity_dynamic_num_searches_by_bbox_in_last_frame() const { return m_proximity_dynamic_num_searches_by_bbox_in_last_frame; }
    QLineEdit* proximity_dynamic_max_num_searches_by_bbox_till_last_frame() const { return m_proximity_dynamic_max_num_searches_by_bbox_till_last_frame; }
    QLineEdit* proximity_dynamic_num_searches_by_line_in_last_frame() const { return m_proximity_dynamic_num_searches_by_line_in_last_frame; }
    QLineEdit* proximity_dynamic_max_num_searches_by_line_till_last_frame() const { return m_proximity_dynamic_max_num_searches_by_line_till_last_frame; }
    QLineEdit* proximity_dynamic_num_enumerate_calls_in_last_frame() const { return m_proximity_dynamic_num_enumerate_calls_in_last_frame; }
    QLineEdit* proximity_dynamic_max_num_enumerate_calls_till_last_frame() const { return m_proximity_dynamic_max_num_enumerate_calls_till_last_frame; }

private:

    program_window*  m_wnd;

    QLineEdit* m_num_capsules;
    QLineEdit* m_num_lines;
    QLineEdit* m_num_points;
    QLineEdit* m_num_spheres;
    QLineEdit* m_num_triangles;

    QLineEdit* m_num_compute_contacts_calls_in_last_frame;
    QLineEdit* m_max_num_compute_contacts_calls_till_last_frame;

    QLineEdit* m_num_contacts_in_last_frame;
    QLineEdit* m_max_num_contacts_till_last_frame;

    QLineEdit* m_proximity_static_num_objects;
    QLineEdit* m_proximity_static_num_split_nodes;
    QLineEdit* m_proximity_static_num_searches_by_bbox_in_last_frame;
    QLineEdit* m_proximity_static_max_num_searches_by_bbox_till_last_frame;
    QLineEdit* m_proximity_static_num_searches_by_line_in_last_frame;
    QLineEdit* m_proximity_static_max_num_searches_by_line_till_last_frame;
    QLineEdit* m_proximity_static_num_enumerate_calls_in_last_frame;
    QLineEdit* m_proximity_static_max_num_enumerate_calls_till_last_frame;

    QLineEdit* m_proximity_dynamic_num_objects;
    QLineEdit* m_proximity_dynamic_num_split_nodes;
    QLineEdit* m_proximity_dynamic_num_searches_by_bbox_in_last_frame;
    QLineEdit* m_proximity_dynamic_max_num_searches_by_bbox_till_last_frame;
    QLineEdit* m_proximity_dynamic_num_searches_by_line_in_last_frame;
    QLineEdit* m_proximity_dynamic_max_num_searches_by_line_till_last_frame;
    QLineEdit* m_proximity_dynamic_num_enumerate_calls_in_last_frame;
    QLineEdit* m_proximity_dynamic_max_num_enumerate_calls_till_last_frame;
};


std::string  get_tooltip();
QWidget*  make_tab_content(widgets const&  w);


}}}

#endif
