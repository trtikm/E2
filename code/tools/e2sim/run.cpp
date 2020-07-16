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
                    false
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

        if (get_keyboard_props().keys_pressed().count(osi::KEY_LALT()) != 0UL ||
            get_keyboard_props().keys_pressed().count(osi::KEY_RALT()) != 0UL )
        {
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_B()) != 0UL)
                render_config().render_scene_batches = !render_config().render_scene_batches;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_R()) != 0UL)
                render_config().render_colliders_of_rigid_bodies = !render_config().render_colliders_of_rigid_bodies;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_S()) != 0UL)
                render_config().render_colliders_of_sensors = !render_config().render_colliders_of_sensors;
            if (get_keyboard_props().keys_just_pressed().count(osi::KEY_C()) != 0UL)
                render_config().render_colliders_of_activators = !render_config().render_colliders_of_activators;
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
    }
};


void run(int argc, char* argv[])
{
    osi::run(std::make_unique<simulator>());
}
