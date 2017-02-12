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

namespace {


std::string  seconds_to_pretty_time_string(float_64_bit  seconds)
{
    float_64_bit constexpr  minute_seconds = 60.0;
    float_64_bit constexpr  hour_seconds = 60.0 * minute_seconds;
    float_64_bit constexpr  day_seconds = 24.0 * hour_seconds;

    msgstream  mstr;
    if (seconds > day_seconds)
    {
        float_64_bit const  num_days = std::floor(seconds / day_seconds);
        mstr << std::fixed << std::setprecision(0) << num_days << "d ";
        seconds -= num_days * day_seconds;
    }
    if (seconds > hour_seconds)
    {
        float_64_bit const  num_hours = std::floor(seconds / hour_seconds);
        mstr << std::fixed << std::setprecision(0) << num_hours << "h ";
        seconds -= num_hours * hour_seconds;
    }
    if (seconds > minute_seconds)
    {
        float_64_bit const  num_minutes = std::floor(seconds / minute_seconds);
        mstr << std::fixed << std::setprecision(0) << num_minutes << "m ";
        seconds -= num_minutes * minute_seconds;
    }
    mstr << std::fixed << std::setprecision(3) << seconds << 's';
    return mstr.get();
}


}


status_bar::status_bar(program_window* const  wnd)
    : m_wnd(wnd)

    , m_spent_real_time(new QLabel(" RT: N/A "))
    , m_spent_simulation_time(new QLabel(" NT: N/A "))
    , m_spent_times_ratio(new QLabel(" NT/RT: N/A "))
    , m_num_passed_simulation_steps(new QLabel(" #0 "))
    , m_experiment_name(new QLabel(" NONE "))
    , m_state(new QLabel(" IDLE "))
    , m_mode(new QLabel(" PAUSED "))
    , m_FPS(new QLabel(" FPS: 0 "))
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

QLabel* status_bar::experiment_name() const noexcept
{
    return m_experiment_name;
}

QLabel* status_bar::state() const noexcept
{
    return m_state;
}

QLabel* status_bar::mode() const noexcept
{
    return m_mode;
}

QLabel* status_bar::FPS() const noexcept
{
    return m_FPS;
}

void status_bar::update()
{
    if (wnd()->glwindow().call_now(&simulator::is_network_being_constructed))
    {
        INVARIANT(!wnd()->glwindow().call_now(&simulator::has_network));
        m_spent_real_time->setText(" RT: N/A ");
        m_spent_simulation_time->setText(" NT: N/A ");
        m_spent_times_ratio->setText(" NT/RT: N/A ");
        m_num_passed_simulation_steps->setText(QString(
            (msgstream() << " #" << wnd()->glwindow().call_now(&simulator::get_num_network_construction_steps)).get().c_str()
            ));
        m_experiment_name->setText(QString(wnd()->glwindow().call_now(&simulator::get_experiment_name).c_str()));
        m_state->setText(QString(
            (msgstream() << " CONSTRUCTION["
                         << wnd()->glwindow().call_now(&simulator::get_constructed_network_progress_text)
                         << "] ").get().c_str()
            ));
    }
    else if (wnd()->glwindow().call_now(&simulator::has_network))
    {
        float_64_bit const  real_time = wnd()->glwindow().call_now(&simulator::spent_real_time);
        std::string  msg = msgstream() << " RT: " << seconds_to_pretty_time_string(real_time) << " ";
        m_spent_real_time->setText(msg.c_str());

        float_64_bit const  network_time = wnd()->glwindow().call_now(&simulator::spent_network_time);
        msg = msgstream() << " NT: " << seconds_to_pretty_time_string(network_time) << " ";
        m_spent_simulation_time->setText(msg.c_str());

        float_64_bit const  network_time_to_real_time = real_time > 1e-5f ? network_time / real_time : 1.0;
        msg = msgstream() << " NT/RT: " << std::fixed << std::setprecision(3) << network_time_to_real_time << " ";
        m_spent_times_ratio->setText(msg.c_str());

        natural_64_bit const  num_steps = wnd()->glwindow().call_now(&simulator::num_network_updates);
        msg = msgstream() << " #" << num_steps << " ";
        m_num_passed_simulation_steps->setText(msg.c_str());

        m_experiment_name->setText(QString(wnd()->glwindow().call_now(&simulator::get_experiment_name).c_str()));
        m_state->setText(" SIMULATION ");
    }
    else
    {
        m_spent_real_time->setText(" RT: N/A ");
        m_spent_simulation_time->setText(" NT: N/A ");
        m_spent_times_ratio->setText(" NT/RT: N/A ");
        m_num_passed_simulation_steps->setText(" #0 ");
        m_experiment_name->setText(" NONE ");
        m_state->setText(" IDLE ");
    }

    m_mode->setText(wnd()->glwindow().call_now(&simulator::paused) ? " PAUSED " : " RUNNING ");

    {
        std::stringstream  sstr;
        sstr << " FPS: " << wnd()->glwindow().call_now(&qtgl::real_time_simulator::FPS) << " ";
        m_FPS->setText(sstr.str().c_str());
    }
}

void  make_status_bar_content(status_bar const&  w)
{
    w.wnd()->statusBar()->addPermanentWidget(w.spent_real_time());
    w.wnd()->statusBar()->addPermanentWidget(w.spent_simulation_time());
    w.wnd()->statusBar()->addPermanentWidget(w.spent_times_ratio());
    w.wnd()->statusBar()->addPermanentWidget(w.num_passed_simulation_steps());
    w.wnd()->statusBar()->addPermanentWidget(w.experiment_name());
    w.wnd()->statusBar()->addPermanentWidget(w.state());
    w.wnd()->statusBar()->addPermanentWidget(w.mode());
    w.wnd()->statusBar()->addPermanentWidget(w.FPS());

    w.wnd()->statusBar()->showMessage("Ready", 2000);
}
