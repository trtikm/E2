#include <com/simulator.hpp>
#include <e2sim/program_info.hpp>
#include <e2sim/program_options.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>


struct  simulator : public com::simulator
{
    void  initialise() override
    {
        set_window_title(get_program_name());
        set_window_pos(get_window_props().window_frame_size_left(), get_window_props().window_frame_size_top());
        set_window_size(1024U, 768U);
        maximise_window();

        context()->insert_frame(context()->root_folder());
        context()->insert_batch_default_grid(context()->root_folder(), "BATCH.grid", com::BATCH_CLASS::HELPER);
        context()->load_batch(context()->root_folder(), "BATCH.sphere_10cm", com::BATCH_CLASS::COMMON_OBJECT,
                              "../data/shared/gfx/batches/sphere_10cm.txt", render_config().effects_config);
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
