#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <com/simulator.hpp>
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
        set_window_title(get_program_name());
        gfx::image_rgba_8888  img;
        gfx::load_png_image(get_program_options()->data_root() + "/shared/gfx/icons/E2_icon.png", img);
        set_window_icon((natural_8_bit)img.width, (natural_8_bit)img.height, img.data);
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        maximise_window();

        com::object_guid const  grid_folder_guid = context()->insert_folder(context()->root_folder(), "grid");
        context()->insert_frame(grid_folder_guid);
        context()->insert_batch_default_grid(grid_folder_guid, "BATCH.grid", com::BATCH_CLASS::HELPER);
        context()->load_batch(grid_folder_guid, "BATCH.sphere_10cm", com::BATCH_CLASS::COMMON_OBJECT,
                              get_program_options()->data_root() + "/shared/gfx/batches/sphere_10cm.txt",
                              render_config().effects_config);

        if (get_program_options()->has_scene_dir())
            context()->request_import_scene_from_directory(
                    get_program_options()->data_root() + '/' + get_program_options()->scene_dir()
                    );
    }

    void  on_begin_simulation() override
    {
        if (get_window_props().focus_just_lost())
            set_paused(true);
    }

    void  on_begin_render() override
    {
        if (get_window_props().focus_just_received())
            render_config().clear_colour = { 0.25f, 0.25f, 0.25f };
        else if (get_window_props().focus_just_lost())
            render_config().clear_colour = { 0.75f, 0.25f, 0.25f };
    }

};


void run(int argc, char* argv[])
{
    osi::run(std::make_unique<simulator>());
}
