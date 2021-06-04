#include <fontcfg/program_info.hpp>
#include <fontcfg/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <com/simulator.hpp>
#include <gfx/image.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/basic_numeric_types.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


struct  simulator : public com::simulator
{
    simulator() : com::simulator(get_program_options()->data_root()) {}

    void  initialise() override
    {
        com::simulator::initialise();

        set_window_title(get_program_name());
        gfx::image_rgba_8888  img;
        gfx::load_png_image(context()->get_icon_root_dir() + "E2_icon.png", img);
        set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        //maximise_window();
        render_config().camera->coordinate_system()->set_origin({ 0.0f, 0.0f, 20.0f });
        render_config().camera->coordinate_system()->set_orientation(quaternion_identity());
    }

    void  on_begin_round() override
    {
        if (get_window_props().focus_just_lost())
            simulation_config().paused = true;

        if (!get_window_props().has_focus())
            return;

        if (get_keyboard_props().keys_just_pressed().count(osi::KEY_PAUSE()) != 0UL)
            simulation_config().paused = !simulation_config().paused;
        else if (get_keyboard_props().keys_just_pressed().count(osi::KEY_SPACE()) != 0UL)
        {
            simulation_config().paused = false;
            simulation_config().num_rounds_to_pause = 1U;
        }

        //bool const  shift = get_keyboard_props().keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
        //                    get_keyboard_props().keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
        //bool const  ctrl = get_keyboard_props().keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
        //                   get_keyboard_props().keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
        //bool const  alt = get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
        //                  get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL;

        // TODO: Implement tool's functionality here.
    }

    void  on_begin_render() override
    {
        if (get_keyboard_props().keys_pressed().count(osi::KEY_F1()) != 0UL)
        {
            SLOG("\n=== HELP ===\n");
        }

        // TODO: Implement tool's custom render here.
    }
};


void run(int argc, char* argv[])
{
    TMPROF_BLOCK();
    osi::run(std::make_unique<simulator>());
}
