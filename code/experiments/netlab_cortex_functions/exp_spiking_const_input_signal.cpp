#include <netlab_cortex_functions/exp_spiking_const_input_signal.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>


exp_spiking_const_input_signal::exp_spiking_const_input_signal()
    : simulator_base()
    , NUM_DENDRITES(50U)
    , EXPECTED_SPIKING_FREQUENCY(10.0f)
    , EXPECTED_SIMULATION_FREQUENCY(100.0f)
    , VALUE_SCALE(10.0f)
    , HISTORY_TIME_WINDOW(3.0f)
    , HISTORY_DT_TO_DX(-VALUE_SCALE)
{}


void  exp_spiking_const_input_signal::network_setup()
{
    layer_idx = cortex.add_layer(1U, 0U, NUM_DENDRITES, true);
    cortex.set_constant_neuron_ln_of_excitation_decay_coef(layer_idx, -3.0f);
    neuron_guid = { layer_idx, 0U };
    num_input_spikes_per_second = NUM_DENDRITES * EXPECTED_SPIKING_FREQUENCY;
    simulatied_time = 0.0f;
}


void  exp_spiking_const_input_signal::network_update()
{
    float_32_bit const  time_step = 1.0f / EXPECTED_SIMULATION_FREQUENCY; // round_seconds(); // 1.0f / cortex.get_constants().simulation_fequency;

    cortex.clear_input_signal_of_neurons();
    cortex.add_to_input_signal(neuron_guid, num_input_spikes_per_second * time_step);
    cortex.update_neurons(time_step);

    simulatied_time += time_step;

    erase_obsolete_records(excitation_history, HISTORY_TIME_WINDOW, simulatied_time);
    insert_to_history(excitation_history, { simulatied_time, cortex.neuron_excitation(neuron_guid) });

    erase_obsolete_records(input_signal_history, HISTORY_TIME_WINDOW, simulatied_time);
    insert_to_history(input_signal_history, { simulatied_time, cortex.get_neuron(neuron_guid).input_signal });

    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_Y()) != 0UL)
        NUM_DENDRITES = std::min(10000U, NUM_DENDRITES + 10U);
    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_U()) != 0UL)
        NUM_DENDRITES = std::max(10U, NUM_DENDRITES - 10U);
    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_G()) != 0UL)
        EXPECTED_SIMULATION_FREQUENCY = std::min(10000.0f, EXPECTED_SIMULATION_FREQUENCY + 10.0f);
    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_H()) != 0UL)
        EXPECTED_SIMULATION_FREQUENCY = std::max(10.0f, EXPECTED_SIMULATION_FREQUENCY - 10.0f);
    if (get_keyboard_props().keys_pressed().count(osi::KEY_O()) != 0UL)
        cortex.neuron_ref(neuron_guid).spiking_excitation += 0.1f * round_seconds();
    if (get_keyboard_props().keys_pressed().count(osi::KEY_P()) != 0UL)
        cortex.neuron_ref(neuron_guid).spiking_excitation -= 0.1f * round_seconds();
    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_K()) != 0UL)
        num_input_spikes_per_second += 1.0f;
    if (get_keyboard_props().keys_just_pressed().count(osi::KEY_L()) != 0UL)
        num_input_spikes_per_second = std::max(0.0f, num_input_spikes_per_second - 1.0f);
    if (get_keyboard_props().keys_pressed().count(osi::KEY_N()) != 0UL)
        cortex.set_constant_neuron_ln_of_excitation_decay_coef(
                layer_idx, 
                cortex.get_layer_constants(layer_idx).neuron.ln_of_excitation_decay_coef + 2.0f * round_seconds()
                );
    if (get_keyboard_props().keys_pressed().count(osi::KEY_M()) != 0UL)
        cortex.set_constant_neuron_ln_of_excitation_decay_coef(
                layer_idx, 
                cortex.get_layer_constants(layer_idx).neuron.ln_of_excitation_decay_coef - 2.0f * round_seconds()
                );
}


void  exp_spiking_const_input_signal::scene_setup()
{
    com::simulation_context&  ctx = *context();

    com::object_guid  folder_guid = ctx.insert_folder(ctx.root_folder(), "excitation_folder");
    excitation_frame_guid = ctx.insert_frame(folder_guid, com::invalid_object_guid(), vector3_unit_x(), quaternion_identity());
    ctx.insert_batch_solid_sphere(folder_guid, "excitation_batch", 0.1f, 5U, vector4{1.0f, 0.0f, 0.8f, 1.0f});

    folder_guid = ctx.insert_folder(ctx.root_folder(), "input_signal_folder");
    input_signal_frame_guid =
            ctx.insert_frame(folder_guid, com::invalid_object_guid(), vector3_zero(), quaternion_identity());
    ctx.insert_batch_solid_sphere(folder_guid, "input_signal_batch", 0.1f, 5U, vector4{0.0f, 1.0f, 0.8f, 1.0f});

    folder_guid = ctx.insert_folder(ctx.root_folder(), "spiking_excitation_folder");
    spiking_excitation_frame_guid =
            ctx.insert_frame(folder_guid, com::invalid_object_guid(), vector3_zero(), quaternion_identity());
    ctx.insert_batch_lines3d(
                folder_guid,
                "spiking_excitation_batch",
                {{
                    vector3_zero(),
                    vector3(HISTORY_TIME_WINDOW * HISTORY_DT_TO_DX, 0.0f, 0.0f)
                }},
                vector4{0.5f, 0.5f, 1.0f, 1.0f}
                );

    history_folder_guid = ctx.insert_folder(ctx.root_folder(), "history_folder");
    ctx.insert_frame(history_folder_guid, com::invalid_object_guid(), vector3_zero(), quaternion_identity());
}


void  exp_spiking_const_input_signal::scene_update()
{
    com::simulation_context&  ctx = *context();

    ctx.frame_set_origin(excitation_frame_guid, {0.0f, 10.0f * cortex.neuron_excitation(neuron_guid), 0.0f });
    ctx.frame_set_origin(input_signal_frame_guid, {0.0f, 10.0f * cortex.get_neuron(neuron_guid).input_signal, 0.0f });
    ctx.frame_set_origin(spiking_excitation_frame_guid, {0.0f, 10.0f * cortex.get_neuron(neuron_guid).spiking_excitation, 0.0f });

    rebuild_history_lines_batch(
            ctx,
            excitation_curve_batch_guid,
            history_folder_guid,
            "excitation_curve_batch",
            vector4{1.0f, 0.0f, 0.8f, 1.0f},
            excitation_history,
            0.0f,
            HISTORY_DT_TO_DX,
            VALUE_SCALE
            );
    rebuild_history_lines_batch(
            ctx,
            input_signal_batch_guid,
            history_folder_guid,
            "input_signal_batch",
            vector4{0.0f, 1.0f, 0.8f, 1.0f},
            input_signal_history,
            0.0f,
            HISTORY_DT_TO_DX,
            VALUE_SCALE
            );
}


void  exp_spiking_const_input_signal::on_restart()
{
    excitation_history.clear();
    input_signal_history.clear();
    excitation_curve_batch_guid = com::invalid_object_guid();
    input_signal_batch_guid = com::invalid_object_guid();
}


void  exp_spiking_const_input_signal::custom_render()
{
    SLOG(
        "NUM_DENDRITES=" << NUM_DENDRITES << " (requires restart Ctrl+R)\n"
        "EXPECTED_SIMULATION_FREQUENCY=" << EXPECTED_SIMULATION_FREQUENCY << "\n"
        "spiking_excitation=" << cortex.get_neuron(neuron_guid).spiking_excitation << "\n"
        "num_input_spikes_per_second=" << num_input_spikes_per_second << "\n"    
        "num_input_spikes_per_frame=" << num_input_spikes_per_second / EXPECTED_SIMULATION_FREQUENCY << "\n"    
        "ln_of_excitation_decay_coef=" << cortex.get_layer_constants(layer_idx).neuron.ln_of_excitation_decay_coef << "\n"
    );
}


void  exp_spiking_const_input_signal::help()
{
    SLOG(
        "\tEvaluation of spiking for a constant input signal.\n"
        "\tRed sphere & curve - neuron's excitation\n"
        "\tGreen sphere & curve - input signal\n"
        "\tBlue line - neuron's spiking excitation\n"
        "\tY/U - increase/decrease number of dendrites (needs restart)\n"
        "\tG/H - increase/decrease expected simulation frequency\n"
        "\tO/P - increase/decrease neuron's spiking excitation\n"
        "\tK/L - increase/decrease number of input spikes per second\n"
        "\tN/M - increase/decrease ln of neuron's excitartion decay coef\n"
        );
}
