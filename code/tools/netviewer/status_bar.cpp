#include <netviewer/status_bar.hpp>
#include <netviewer/program_window.hpp>
#include <netviewer/simulator.hpp>
#include <netviewer/simulator_notifications.hpp>
#include <qtgl/window.hpp>
#include <qtgl/gui_utils.hpp>
#include <qtgl/widget_base.hpp>
#include <utility/msgstream.hpp>
#include <QStatusBar>
#include <iomanip>


status_bar::status_bar(program_window* const  wnd)
    : m_wnd(wnd)

    , m_spent_real_time(new QLabel("0.0s"))
    , m_spent_simulation_time(new QLabel("0.0s"))
    , m_spent_times_ratio(new QLabel("1.0"))
    , m_num_passed_simulation_steps(new QLabel("#Steps: 0"))
{}

program_window*  status_bar::wnd() const noexcept
{
    return m_wnd;
}

QLabel* status_bar::spent_real_time() const noexcept
{
    return m_spent_real_time;
}

QLabel* status_bar::spent_simulation_time() const noexcept
{
    return m_spent_simulation_time;
}

QLabel* status_bar::spent_times_ratio() const noexcept
{
    return m_spent_times_ratio;
}

QLabel* status_bar::num_passed_simulation_steps() const noexcept
{
    return m_num_passed_simulation_steps;
}

void status_bar::update()
{
    float_64_bit const  real_time = wnd()->glwindow().call_now(&simulator::spent_real_time);
    std::string  msg = msgstream() << "RT: " << std::fixed << std::setprecision(3) << real_time << "s";
    m_spent_real_time->setText(msg.c_str());

//        float_64_bit const  simulation_time = m_glwindow.call_now(&simulator::spent_simulation_time);
//        msg = msgstream() << "ST: " << std::fixed << std::setprecision(3) << simulation_time  << "s";
//        m_spent_simulation_time->setText(msg.c_str());

//        float_64_bit const  simulation_time_to_real_time = real_time > 1e-5f ? simulation_time / real_time : 1.0;
//        msg = msgstream() << "ST/RT: " << std::fixed << std::setprecision(3) << simulation_time_to_real_time;
//        m_spent_times_ratio->setText(msg.c_str());

//        natural_64_bit const  num_steps = m_glwindow.call_now(&simulator::nenet_num_updates);
//        msg = msgstream() << "#Steps: " << num_steps;
//        m_num_passed_simulation_steps->setText(msg.c_str());
}

void  make_status_bar_content(status_bar const&  w)
{
    w.wnd()->statusBar()->addPermanentWidget(w.spent_real_time());
    w.wnd()->statusBar()->addPermanentWidget(w.spent_simulation_time());
    w.wnd()->statusBar()->addPermanentWidget(w.spent_times_ratio());
    w.wnd()->statusBar()->addPermanentWidget(w.num_passed_simulation_steps());
    w.wnd()->statusBar()->addPermanentWidget(
            [](qtgl::window<simulator>* const glwindow, bool const  paused) {
                struct s : public qtgl::widget_base<s, qtgl::window<simulator> >, public QLabel {
                    s(qtgl::window<simulator>* const  glwindow, bool const  paused)
                        : qtgl::widget_base<s, qtgl::window<simulator> >(glwindow), QLabel()
                    {
                        setText(paused ? "PAUSED" : "RUNNING");
                        register_listener(simulator_notifications::paused(),&s::on_paused_changed);
                    }
                    void  on_paused_changed()
                    {
                        setText(call_now(&simulator::paused) ? "PAUSED" : "RUNNING");
                    }
                };
                return new s(glwindow, paused);
            }(&w.wnd()->glwindow(), w.wnd()->ptree().get("simulation.paused", false))
            );
    w.wnd()->statusBar()->addPermanentWidget(
            [](qtgl::window<simulator>* const glwindow) {
                struct s : public qtgl::widget_base<s, qtgl::window<simulator> >, public QLabel {
                    s(qtgl::window<simulator>* const  glwindow)
                        : qtgl::widget_base<s, qtgl::window<simulator> >(glwindow), QLabel()
                    {
                        setText("FPS: 0");
                        register_listener(qtgl::notifications::fps_updated(),
                            &s::on_fps_changed);
                    }
                    void  on_fps_changed()
                    {
                        std::stringstream  sstr;
                        sstr << "FPS: " << call_now(&qtgl::real_time_simulator::FPS);
                        setText(sstr.str().c_str());
                    }
                };
                return new s(glwindow);
            }(&w.wnd()->glwindow())
            );
    w.wnd()->statusBar()->showMessage("Ready", 2000);
}
