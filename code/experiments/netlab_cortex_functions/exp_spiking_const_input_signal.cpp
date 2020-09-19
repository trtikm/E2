#include <netlab_cortex_functions/exp_spiking_const_input_signal.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>


exp_spiking_const_input_signal::exp_spiking_const_input_signal()
    : simulator_base()
    , SIMULATION_FREQUENCY(100.0f)
    , VALUE_SCALE(1.0f)
    , HISTORY_TIME_WINDOW(10.0f)
    , HISTORY_DT_TO_DX(-10.0f)
{}


void  exp_spiking_const_input_signal::network_setup()
{
    layer_idx = cortex.add_layer(1U, 0U, 1U, true);

    neuron_guid = { layer_idx, 0U };
    input_signal = 10.0f;
    simulatied_time = 0.0f;

    cortex.update_dependent_constants();
    cortex.set_constant_neuron_excitation_decay_coef(layer_idx, -1.0f);
    cortex.set_constant_neuron_excitation_spike(layer_idx, 9.0f);
    cortex.set_constant_neuron_input_signal(layer_idx, input_signal);
}


void  exp_spiking_const_input_signal::network_update()
{
    simulatied_time += 1.0f / SIMULATION_FREQUENCY;

    cortex.clear_input_signal_of_neurons();
    cortex.update_neurons();

    erase_obsolete_records(excitation_history, HISTORY_TIME_WINDOW, simulatied_time);
    insert_to_history(excitation_history, { simulatied_time, cortex.neuron_excitation(neuron_guid) });

    erase_obsolete_records(input_signal_history, HISTORY_TIME_WINDOW, simulatied_time);
    insert_to_history(input_signal_history, { simulatied_time, cortex.get_neuron(neuron_guid).input_signal });

    if (cortex.is_neuron_spiking(neuron_guid))
        insert_to_history(output_spikes_history, { simulatied_time, 1.0f });
    erase_obsolete_records(output_spikes_history, HISTORY_TIME_WINDOW, simulatied_time);

    if (is_key_just_pressed(osi::KEY_G()) != 0UL)
    {
        SIMULATION_FREQUENCY = std::min(10000.0f, SIMULATION_FREQUENCY + 10.0f);
        cortex.set_constant_simulation_frequency(SIMULATION_FREQUENCY);
        cortex.update_dependent_constants();
    }
    if (is_key_just_pressed(osi::KEY_H()) != 0UL)
    {
        SIMULATION_FREQUENCY = std::max(10.0f, SIMULATION_FREQUENCY - 10.0f);
        cortex.set_constant_simulation_frequency(SIMULATION_FREQUENCY);
        cortex.update_dependent_constants();
    }
    if (is_key_just_pressed(osi::KEY_O()) != 0UL)
        cortex.set_constant_neuron_excitation_spike(layer_idx,
                cortex.get_layer_constants(layer_idx).neuron.excitation_spike + 0.1f);
    if (is_key_just_pressed(osi::KEY_P()) != 0UL)
        cortex.set_constant_neuron_excitation_spike(layer_idx,
                cortex.get_layer_constants(layer_idx).neuron.excitation_spike - 0.1f);
    if (is_key_just_pressed(osi::KEY_K()) != 0UL)
    {
        input_signal += 1.0f;
        cortex.set_constant_neuron_input_signal(layer_idx, input_signal);
    }
    if (is_key_just_pressed(osi::KEY_L()) != 0UL)
    {
        input_signal -= 1.0f;
        cortex.set_constant_neuron_input_signal(layer_idx, input_signal);
    }
    if (is_key_just_pressed(osi::KEY_N()) != 0UL)
        cortex.set_constant_neuron_excitation_decay_coef(
                layer_idx, 
                std::min(
                    -0.1f,
                    cortex.get_layer_constants(layer_idx).neuron.excitation_decay_coef + 0.1f
                    )
                );
    if (is_key_just_pressed(osi::KEY_M()) != 0UL)
        cortex.set_constant_neuron_excitation_decay_coef(
                layer_idx, 
                cortex.get_layer_constants(layer_idx).neuron.excitation_decay_coef - 0.1f
                );
    if (is_key_just_pressed(osi::KEY_Z()) != 0UL)
        cortex.set_constant_neuron_excitation_spike(layer_idx,
                0.95f * -input_signal / cortex.get_layer_constants(layer_idx).neuron.excitation_decay_coef);

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

    ctx.frame_set_origin(excitation_frame_guid, {0.0f, VALUE_SCALE * cortex.neuron_excitation(neuron_guid), 0.0f });
    ctx.frame_set_origin(input_signal_frame_guid, {0.0f, VALUE_SCALE * cortex.get_neuron(neuron_guid).input_signal, 0.0f });
    ctx.frame_set_origin(spiking_excitation_frame_guid, {0.0f, VALUE_SCALE * cortex.get_layer_constants(layer_idx).neuron.excitation_spike, 0.0f });

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
    output_spikes_history.clear();
}


void  exp_spiking_const_input_signal::custom_render()
{
    SLOG(
        "SIMULATION_FREQUENCY=" << SIMULATION_FREQUENCY << "\n"
        "excitation_spike=" << cortex.get_layer_constants(layer_idx).neuron.excitation_spike << "\n"
        "input_signal=" << input_signal << "\n"    
        "excitation_decay_coef=" << cortex.get_layer_constants(layer_idx).neuron.excitation_decay_coef << "\n"
        "output_spiking_frequency=" << output_spikes_history.size() / HISTORY_TIME_WINDOW << "\n"
    );
}


void  exp_spiking_const_input_signal::help()
{
    SLOG(
        "\tEvaluation of spiking for a constant input signal.\n"
        "\tRed sphere & curve - neuron's excitation\n"
        "\tGreen sphere & curve - input signal\n"
        "\tBlue line - neuron's spiking excitation\n"
        "\tG/H - increase/decrease simulation frequency\n"
        "\tO/P - increase/decrease neuron's excitation_spike\n"
        "\tK/L - increase/decrease input signal\n"
        "\tN/M - increase/decrease ln of neuron's excitartion decay coef\n"
        "\tZ - set spiking excitation to 95% of max. neuron excitation\n"
        );
}
