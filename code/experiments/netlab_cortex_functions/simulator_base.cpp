#include <netlab_cortex_functions/simulator_base.hpp>
#include <netlab_cortex_functions/program_info.hpp>
#include <netlab_cortex_functions/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <gfx/image.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>


simulator_base::simulator_base()
    : com::simulator(get_program_options()->data_root())
{}


void  simulator_base::initialise()
{
    com::simulator::initialise();

    set_window_title(get_program_name() + ": " + get_program_options()->experiment());
    gfx::image_rgba_8888  img;
    gfx::load_png_image(get_program_options()->data_root() + "/icon/E2_icon.png", img);
    set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
    set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
    set_window_size(1024U, 768U);
    maximise_window();

    render_config().camera->coordinate_system()->set_origin(vector3(0.0f, 0.0f, 50.0f));
    render_config().camera->coordinate_system()->set_orientation(quaternion_identity());
    change_camera_speed(4.0f);
    render_config().batch_grid = gfx::create_default_grid_2d(100.0f);
    simulation_config().paused = false;
    simulation_config().num_rounds_to_pause = 0U;

    network_setup();
    scene_setup();
}


void  simulator_base::custom_module_round()
{
    network_update();
    scene_update();
}


void  simulator_base::on_begin_round()
{
    //if (get_window_props().focus_just_lost())
    //    simulation_config().paused = true;

    if (!get_window_props().has_focus())
        return;

    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_PAUSE()) != 0UL)
        simulation_config().paused = !simulation_config().paused;
    else if (get_keyboard_props().keys_just_pressed().count(osi::KEY_SPACE()) != 0UL)
    {
        simulation_config().paused = false;
        simulation_config().num_rounds_to_pause = 1U;
    }

    if (is_alt_down() && !is_ctrl_down())
    {
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_G()) != 0UL)
            render_config().render_grid = !render_config().render_grid;
    }
    else if (is_ctrl_down() && !is_alt_down())
    {
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_R()) != 0UL)
        {
            on_restart();
            clear(true);
            network_setup();
            scene_setup();
        }
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_K()) != 0UL)
            render_config().camera->coordinate_system()->set_orientation(quaternion_identity());
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_NUMERIC_PLUS()) != 0UL)
            change_camera_speed(2.0f);
        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_NUMERIC_MINUS()) != 0UL)
            change_camera_speed(0.5f);
    }
}

void  simulator_base::on_begin_render()
{
    if (get_keyboard_props().keys_pressed().count(osi::KEY_F1()) != 0UL)
    {
        SLOG("\n=== HELP BEGIN ===\n");

        SLOG("1. Camera - free fly\n");
        for (auto const&  action : render_config().free_fly_config)
            if (!action.help().empty())
                SLOG("\t" << action.help() << "\n");
        SLOG(
            "\tCTRL+'NUM+' - double speed\n"
            "\tCTRL+'NUM-' - half speed\n"
            "\tCTRL+K' - reset camera rotation.\n"
            );
        SLOG(
            "2. Simulation\n"
            "\tPAUSE - toggle pause simulation\n"
            "\tSPACE - single simulation round\n"
            "\tCtrl+R - restart the experiment\n"
            );
        SLOG(
            "3. Experiment\n"
            );
        help();
        SLOG("=== HELP END ===\n");
    }
    else
        custom_render();
}

bool  simulator_base::is_shift_down() const
{
    return get_keyboard_props().keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
           get_keyboard_props().keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
}

bool  simulator_base::is_ctrl_down() const
{
    return get_keyboard_props().keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
           get_keyboard_props().keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
}

bool  simulator_base::is_alt_down() const
{
    return get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
           get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL;
}

bool  simulator_base::is_key_just_pressed(osi::keyboard_key_name const&  key) const
{
    return get_keyboard_props().keys_just_pressed().count(key) != 0UL;
}
