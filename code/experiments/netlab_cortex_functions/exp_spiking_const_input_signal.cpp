#include <netlab_cortex_functions/exp_spiking_const_input_signal.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>


static std::vector<std::pair<vector3, vector3> >&  history_to_lines(
        exp_spiking_const_input_signal::history_records const&  records,
        float_32_bit const  x0,
        float_32_bit const  dt_to_dx,
        float_32_bit const  value_scale,
        std::vector<std::pair<vector3, vector3> >&  lines
        )
{
    lines.clear();
    float_32_bit  x = x0;
    if (records.size() > 5UL)
    {
        int iii=0;
    }

    for (natural_32_bit  i = 0U, n = (natural_32_bit)records.size(); i + 1U < n; ++i)
    {
        float_32_bit const  dx = dt_to_dx * (records.at(i).time_point - records.at(i+1U).time_point);
        lines.push_back({
            vector3(x, value_scale * records.at(i).value, 0.0f),
            vector3(x + dx, value_scale * records.at(i+1U).value, 0.0f)
            });
        x += dx;
    }
    return lines;
}


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

    while (!excitation_history.empty() && excitation_history.back().time_point + HISTORY_TIME_WINDOW < simulatied_time)
        excitation_history.pop_back();
    excitation_history.push_front({ simulatied_time, cortex.neuron_excitation(neuron_guid) });

    while (!input_signal_history.empty() && input_signal_history.back().time_point + HISTORY_TIME_WINDOW < simulatied_time)
        input_signal_history.pop_back();
    input_signal_history.push_front({ simulatied_time, cortex.get_neuron(neuron_guid).input_signal });

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

    if (ctx.is_valid_batch_guid(excitation_curve_batch_guid))
        ctx.erase_batch(excitation_curve_batch_guid);
    std::vector<std::pair<vector3, vector3> >  lines;
    history_to_lines(excitation_history, 0.0f, HISTORY_DT_TO_DX, VALUE_SCALE, lines);
    if (!lines.empty())
        excitation_curve_batch_guid = ctx.insert_batch_lines3d(
                history_folder_guid,
                "excitation_curve_batch",
                lines,
                vector4{1.0f, 0.0f, 0.8f, 1.0f}
                );
    if (ctx.is_valid_batch_guid(input_signal_batch_guid))
        ctx.erase_batch(input_signal_batch_guid);
    history_to_lines(input_signal_history, 0.0f, HISTORY_DT_TO_DX, VALUE_SCALE, lines);
    if (!lines.empty())
        input_signal_batch_guid = ctx.insert_batch_lines3d(
                history_folder_guid,
                "input_signal_batch",
                lines,
                vector4{0.0f, 1.0f, 0.8f, 1.0f}
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
