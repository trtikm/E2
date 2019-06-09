#ifndef E2_TOOL_GFXTUNER_STATUS_BAR_HPP_INCLUDED
#   define E2_TOOL_GFXTUNER_STATUS_BAR_HPP_INCLUDED

#   include <angeo/tensor_math.hpp>
#   include <utility/async_resource_load.hpp>
#   include <boost/property_tree/ptree.hpp>
#   include <QWidget>
#   include <QLabel>


struct  program_window;


struct  status_bar
{
    status_bar(program_window* const  wnd);

    program_window*  wnd() const { return m_wnd; }

    QLabel* spent_real_time() const { return m_spent_real_time; }
    QLabel* num_passed_simulation_steps() const { return m_num_passed_simulation_steps; }
    QLabel* camera_controller_type() const { return m_camera_controller_type; }
    QLabel* state() const { return m_state; }
    QLabel* mode() const { return m_mode; }
    QLabel* FPS() const { return m_FPS; }

    void  edit_mode_listener();
    void  camera_controller_changed_listener();

    void  update();

    void print_status_message(std::string const&  msg, natural_32_bit const  num_miliseconds_to_show = 2000);

private:
    program_window*  m_wnd;

    QLabel*  m_spent_real_time;
    QLabel*  m_num_passed_simulation_steps;
    QLabel*  m_camera_controller_type;
    QLabel*  m_state;
    QLabel*  m_mode;
    QLabel*  m_FPS;

    async::key_type  m_currently_loaded_resource;
};


void  make_status_bar_content(status_bar const&  w);


#endif
