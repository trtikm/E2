#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <com/simulator.hpp>
#include <gfx/batch_generators.hpp>
#include <gfx/image.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
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
        gfx::load_png_image(get_program_options()->data_root() + "/shared/gfx/icons/E2_icon.png", img);
        set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        //maximise_window();

        render_config().batch_grid = gfx::create_default_grid();
        render_config().batch_frame = gfx::create_basis_vectors();

        render_config().render_grid = true;

        if (get_program_options()->has_scene_dir())
            context()->request_import_scene_from_directory(
                    get_program_options()->data_root() + '/' + get_program_options()->scene_dir(),
                    context()->root_folder(),
                    com::invalid_object_guid(),
                    true
                    );
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

        bool const  shift = get_keyboard_props().keys_pressed().count(osi::KEY_LSHIFT()) != 0UL ||
                            get_keyboard_props().keys_pressed().count(osi::KEY_RSHIFT()) != 0UL;
        bool const  ctrl = get_keyboard_props().keys_pressed().count(osi::KEY_LCTRL()) != 0UL ||
                           get_keyboard_props().keys_pressed().count(osi::KEY_RCTRL()) != 0UL;
        bool const  alt = get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
                          get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL;

        if (alt && !ctrl)
        {
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_B()) != 0UL)
                render_config().render_scene_batches = !render_config().render_scene_batches;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_R()) != 0UL)
                render_config().render_colliders_of_rigid_bodies = !render_config().render_colliders_of_rigid_bodies;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_L()) != 0UL)
                render_config().render_colliders_of_fields = !render_config().render_colliders_of_fields;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_S()) != 0UL)
                render_config().render_colliders_of_sensors = !render_config().render_colliders_of_sensors;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_A()) != 0UL)
                render_config().render_colliders_of_agents = !render_config().render_colliders_of_agents;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_Y()) != 0UL)
                render_config().render_colliders_of_ray_casts = !render_config().render_colliders_of_ray_casts;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_K()) != 0UL)
                render_config().render_collision_contacts = !render_config().render_collision_contacts;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_W()) != 0UL)
                render_config().render_in_wireframe = !render_config().render_in_wireframe;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_G()) != 0UL)
                render_config().render_grid = !render_config().render_grid;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_F()) != 0UL)
                render_config().render_frames = !render_config().render_frames;
        }
        else if (ctrl && !alt)
        {
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_R()) != 0UL)
            {
                clear(shift);
                if (get_program_options()->has_scene_dir())
                    context()->request_import_scene_from_directory(
                            get_program_options()->data_root() + '/' + get_program_options()->scene_dir(),
                            context()->root_folder(),
                            com::invalid_object_guid(),
                            true
                            );
                simulation_config().paused = true;
            }
        }
    }

    void  on_begin_render() override
    {
        if (get_keyboard_props().keys_pressed().count(osi::KEY_F1()) != 0UL)
        {
            SLOG("\n=== HELP ===\n");

            SLOG("1. Camera - free fly\n\t");
            for (natural_32_bit  i = 0, n = (natural_32_bit)render_config().free_fly_config.size(); i != n; ++i)
            {
                if (i != 0U && (i % 2U) == 0U)
                    SLOG("\n\t");
                std::string const&  txt = render_config().free_fly_config.at(i).help();
                if (!txt.empty())
                    SLOG(txt << ",\t");
            }
            SLOG("\n");

            SLOG(
                "2. Simulation\n"
                "\tPAUSE - toggle pause simulation\n"
                "\tSPACE - single simulation round\n"
                );
            SLOG(
                "3. Render GFX\n"
                "\tALT+B - batches\n"
                "\tALT+G - grid\n"
                "\tALT+F - frames\n"
                "\tALT+W - wireframe\n"
            );
            SLOG(
                "4. Render colliders\n"
                "\tALT+R - rigid bodies, ALT+K - collisions\n"
                "\tALT+S - sensors\n"
                "\tALT+A - agents\n"
                "\tALT+L - fields, ALT+Y - ray cast targets\n"
            );
            SLOG(
                "5. Scene\n"
                "\tCTRL - fast reload (keep cached gfx)\n"
                "\tCTRL+SHIFT - full reload (clear caches)\n"
            );

            SLOG("\n");
        }
    }
};


void run(int argc, char* argv[])
{
    osi::run(std::make_unique<simulator>());
}
