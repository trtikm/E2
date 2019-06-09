#include <gfxtuner/status_bar.hpp>
#include <gfxtuner/program_window.hpp>
#include <gfxtuner/simulation/simulator.hpp>
#include <gfxtuner/simulation/simulator_notifications.hpp>
#include <scene/scene_editing.hpp>
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
    , m_num_passed_simulation_steps(new QLabel(" #0 "))
    , m_camera_controller_type(new QLabel(" N/A "))
    , m_state(new QLabel(" STARTING "))
    , m_mode(new QLabel(" PAUSED "))
    , m_FPS(new QLabel(" FPS: 0 "))
    , m_currently_loaded_resource(async::get_invalid_key())
{}


void  status_bar::edit_mode_listener()
{
    auto const  edit_mode = m_wnd->glwindow().call_now(&simulator::get_scene_edit_mode);
    switch (edit_mode)
    {
    case scn::SCENE_EDIT_MODE::SELECT_SCENE_OBJECT: m_state->setText(" SELECT "); break;
    case scn::SCENE_EDIT_MODE::TRANSLATE_SELECTED_NODES: m_state->setText(" TRANSLATE "); break;
    case scn::SCENE_EDIT_MODE::ROTATE_SELECTED_NODES: m_state->setText(" ROTATE "); break;
    }
}


void  status_bar::camera_controller_changed_listener()
{
    auto const  camera_controller_type = m_wnd->glwindow().call_now(&simulator::get_camera_controller_type);
    switch (camera_controller_type)
    {
    case CAMERA_CONTROLLER_FREE_FLY: m_camera_controller_type->setText(" FREE FLY "); break;
    case CAMERA_CONTROLLER_ORBIT: m_camera_controller_type->setText(" ORBIT "); break;
    case CAMERA_CONTROLLER_FOLLOW: m_camera_controller_type->setText(" FOLLOW "); break;
    case CAMERA_CONTROLLER_LOOK_AT: m_camera_controller_type->setText(" LOOK AT "); break;
    case CAMERA_CONTROLLER_FOLLOW_AND_LOOK_AT: m_camera_controller_type->setText(" FOLLOW & LOOK AT "); break;
    default: UNREACHABLE(); break;
    }
}


void status_bar::update()
{
    float_64_bit const  real_time = wnd()->glwindow().call_now(&qtgl::real_time_simulator::total_simulation_time);
    std::string  msg = msgstream() << " " << seconds_to_pretty_time_string(real_time) << " ";
    m_spent_real_time->setText(msg.c_str());

    natural_64_bit const  num_steps = wnd()->glwindow().call_now(&qtgl::real_time_simulator::round_id);
    msg = msgstream() << " #" << num_steps << " ";
    m_num_passed_simulation_steps->setText(msg.c_str());

    if (m_camera_controller_type->text() == " N/A ")
        camera_controller_changed_listener();

    m_mode->setText(wnd()->glwindow().call_now(&simulator::paused) ? " PAUSED " : " RUNNING ");

    {
        std::stringstream  sstr;
        sstr << " FPS: " << wnd()->glwindow().call_now(&qtgl::real_time_simulator::FPS) << " ";
        m_FPS->setText(sstr.str().c_str());
    }

    async::key_type const  currently_loaded_resource = async::get_key_of_resource_just_being_loaded();
    if (currently_loaded_resource != m_currently_loaded_resource)
    {
        m_currently_loaded_resource = currently_loaded_resource;
        std::stringstream  sstr;
        if (m_currently_loaded_resource == async::get_invalid_key())
            sstr << "Asynchronous load: All requests have been processed.";
        else
            sstr << "Asynchronous load: Loading '" << m_currently_loaded_resource << "'.";
        print_status_message(sstr.str(), 10000U);
    }
}


void status_bar::print_status_message(std::string const&  msg, natural_32_bit const  num_miliseconds_to_show)
{
    m_wnd->statusBar()->showMessage(QString(msg.c_str()), num_miliseconds_to_show);
}


void  make_status_bar_content(status_bar const&  w)
{
    w.wnd()->statusBar()->addPermanentWidget(w.spent_real_time());
    w.wnd()->statusBar()->addPermanentWidget(w.num_passed_simulation_steps());
    w.wnd()->statusBar()->addPermanentWidget(w.camera_controller_type());
    w.wnd()->statusBar()->addPermanentWidget(w.state());
    w.wnd()->statusBar()->addPermanentWidget(w.mode());
    w.wnd()->statusBar()->addPermanentWidget(w.FPS());

    w.wnd()->statusBar()->showMessage("Ready", 2000);

    w.wnd()->glwindow().register_listener(
        simulator_notifications::scene_edit_mode_changed(),
        { &program_window::status_bar_edit_mode_listener, w.wnd() }
    );

    w.wnd()->glwindow().register_listener(
        simulator_notifications::camera_controller_changed(),
        { &program_window::status_bar_camera_controller_changed_listener, w.wnd() }
    );
}
