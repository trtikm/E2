#include <netlab_cortex_functions/exp_spiking_trains.hpp>
#include <angeo/tensor_math.hpp>
#include <boost/filesystem.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>
#include <map>
#include <fstream>
#include <iomanip>


exp_spiking_trains::exp_spiking_trains()
    : simulator_base()
    , NUM_DENDRITES_EXCITATORY(100U)
    , NUM_DENDRITES_INHIBITORY(100U)
    , EXPECTED_SPIKING_FREQUENCY_EXCITATORY(10.0f)
    , EXPECTED_SPIKING_FREQUENCY_INHIBITORY(10.0f)
    , VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY(0.0f)
    , VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY(0.0f)
    , SIMULATION_FREQUENCY(100.0f)
    , VALUE_SCALE(1.0f)
    , HISTORY_TIME_WINDOW(10.0f)
    , HISTORY_DT_TO_DX(-10.0f)
    , SPIKES_TIME_WINDOW(-1.0f)
    , spike_trains_excitatory(HISTORY_DT_TO_DX, -1.0f, SPIKES_TIME_WINDOW, "spikes_excitatory", vector4{1.0f, 0.75f, 0.75f, 1.0f})
    , spike_trains_inhibitory(HISTORY_DT_TO_DX, -1.0f, SPIKES_TIME_WINDOW, "spikes_inhibitory", vector4{0.75f, 0.75f, 1.0f, 1.0f})
    , output_spikes_history()
    , input_sum_history()
    , input_sum_excitatory_history()
    , input_sum_inhibitory_history()
    , input_sum_folder_guid(com::invalid_object_guid())
    , input_sum_batch_guid(com::invalid_object_guid())
    , phaser()
{}


exp_spiking_trains::phase_manager::phase_manager()
    : phases()
    , phase_index(0U)
    , is_enabled(false)
{
    std::vector<natural_32_bit> const  Ds = {25U, 50U, 100U, 150U, 200U, 300U, 500U, 1000U, 5000U, 10000U };
    std::vector<float_32_bit> const  Fs = {100.0f, 150.0f, 200.0f, 250.0f, 500.0f, 1000.0f };

    for (natural_32_bit  D : Ds)
        for (float_32_bit  f : Fs)
            phases.push_back({ D, f });
}

exp_spiking_trains::phase_manager::phase_info::phase_info(
        natural_32_bit  NUM_DENDRITES_EXCITATORY_,
        float_32_bit  SIMULATION_FREQUENCY_
        )
    // constsants:
    : NUM_DENDRITES_EXCITATORY(NUM_DENDRITES_EXCITATORY_)
    , NUM_DENDRITES_INHIBITORY(NUM_DENDRITES_EXCITATORY)
    , SIMULATION_FREQUENCY(SIMULATION_FREQUENCY_)
    , RECORD_TIME_WINDOW(1.0f)
    , MAX_RECORDS(30U)
    // data:
    , phase_time(0.0f)
    // results:
    , r95s()
    , r95_average(0.0f)
{}


bool  exp_spiking_trains::move_to_next_phase()
{
    auto const  setup_phase = [this]() -> bool {
        if (phaser.phase_index >= (natural_32_bit)phaser.phases.size())
            return false;
        phase_manager::phase_info&  info = phaser.phases.at(phaser.phase_index);
        NUM_DENDRITES_EXCITATORY = info.NUM_DENDRITES_EXCITATORY;
        NUM_DENDRITES_INHIBITORY = info.NUM_DENDRITES_INHIBITORY;
        SIMULATION_FREQUENCY = info.SIMULATION_FREQUENCY;
        EXPECTED_SPIKING_FREQUENCY_EXCITATORY = 10.0f;
        EXPECTED_SPIKING_FREQUENCY_INHIBITORY = 10.0f;
        VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY = 0.0f;
        VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY = 0.0f;
        SPIKES_TIME_WINDOW = -1.0f;
        return true;
    };

    auto const  finish_phase = [this]() -> void {
        phase_manager::phase_info&  info = phaser.phases.at(phaser.phase_index);
        for (float_32_bit  r95 : info.r95s)
            info.r95_average += r95;
        info.r95_average /= (float_32_bit)info.r95s.size();
    };

    auto const  save_results = [this]() -> void {
        std::map<natural_32_bit, // NUM_DENDRITES_EXCITATORY
                 std::map<float_32_bit, // SIMULATION_FREQUENCY
                          float_32_bit // average r95
                          > >  table, model, errors;
        for (phase_manager::phase_info const&  info : phaser.phases)
        {
            table[info.NUM_DENDRITES_EXCITATORY][info.SIMULATION_FREQUENCY] = info.r95_average;

            float_32_bit const  model_r95 = 
                    std::powf(info.NUM_DENDRITES_EXCITATORY / (info.SIMULATION_FREQUENCY * 0.0028f + 0.3f), 1.0f / 2.21f);
            model[info.NUM_DENDRITES_EXCITATORY][info.SIMULATION_FREQUENCY] = model_r95;
            errors[info.NUM_DENDRITES_EXCITATORY][info.SIMULATION_FREQUENCY] =
                    100.0f * (model_r95 - info.r95_average) / info.r95_average;
        }

        std::string const  out_dir = get_experiment_dir();
        boost::filesystem::create_directories(out_dir);

        {
            std::string const  out_file = out_dir + "/r95.dat";
            std::ofstream  ofile(out_file.c_str(), std::ios_base::out);
            for (auto const&  dendrites_and_data : table)
            {
                ofile << dendrites_and_data.first << "\t";
                for (auto const&  frequency_and_r95 : dendrites_and_data.second)
                    ofile << std::fixed << std::setprecision(1) << frequency_and_r95.second << "\t";
                ofile << "\n";
            }
            ofile.close();
        }
        {
            std::string const  out_file = out_dir + "/r95_model.dat";
            std::ofstream  ofile(out_file.c_str(), std::ios_base::out);
            for (auto const&  dendrites_and_data : model)
            {
                ofile << dendrites_and_data.first << "\t";
                for (auto const&  frequency_and_r95 : dendrites_and_data.second)
                    ofile << std::fixed << std::setprecision(1)<< frequency_and_r95.second << "\t";
                ofile << "\n";
            }
            ofile.close();
        }
        {
            std::string const  out_file = out_dir + "/r95_model_err.dat";
            std::ofstream  ofile(out_file.c_str(), std::ios_base::out);
            for (auto const&  dendrites_and_data : errors)
            {
                ofile << dendrites_and_data.first << "\t";
                for (auto const&  frequency_and_r95 : dendrites_and_data.second)
                    ofile << std::fixed << std::setprecision(1) << frequency_and_r95.second << "\t";
                ofile << "\n";
            }
            ofile.close();
        }
    };

    if (!phaser.is_enabled)
    {
        if (is_ctrl_down() && is_alt_down() && is_key_just_pressed(osi::KEY_P()))
            phaser.is_enabled = setup_phase();
        return phaser.is_enabled;
    }

    if (phaser.phase_index >= (natural_32_bit)phaser.phases.size())
    {
        save_results();
        send_close_request();
        return false;
    }

    phase_manager::phase_info&  info = phaser.phases.at(phaser.phase_index);
    info.phase_time += 1.0f / info.SIMULATION_FREQUENCY;
    if (info.phase_time >= info.RECORD_TIME_WINDOW)
    {
        info.phase_time = 0.0f;
        info.r95s.push_back(histogram_radius_95(input_sum_history));
        if (info.r95s.size() >= info.MAX_RECORDS)
        {
            finish_phase();
            ++phaser.phase_index;
            return setup_phase();
        }
    }

    return false;
}


void  exp_spiking_trains::network_setup()
{
    input_layer_excitatory_idx = cortex.add_layer(NUM_DENDRITES_EXCITATORY, 1U, 0U, true);
    input_layer_inhibitory_idx = cortex.add_layer(NUM_DENDRITES_INHIBITORY, 1U, 0U, false);
    spiking_layer_idx = cortex.add_layer(1U, 0U, NUM_DENDRITES_EXCITATORY + NUM_DENDRITES_INHIBITORY, true);

    cortex.set_constant_synapse_weight_delta_per_second(input_layer_excitatory_idx, spiking_layer_idx, 0.0f);
    cortex.set_constant_synapse_weight_decay_delta_per_second(input_layer_excitatory_idx, spiking_layer_idx, 0.0f);
    cortex.set_constant_synapse_weight_delta_per_second(input_layer_inhibitory_idx, spiking_layer_idx, 0.0f);
    cortex.set_constant_synapse_weight_decay_delta_per_second(input_layer_inhibitory_idx, spiking_layer_idx, 0.0f);

    clear_connection_probabilities(cortex, input_layer_excitatory_idx);
    clear_connection_probabilities(cortex, input_layer_inhibitory_idx);
    clear_connection_probabilities(cortex, spiking_layer_idx);
    cortex.set_constant_connection_probability(input_layer_excitatory_idx, spiking_layer_idx, 1.0f);
    cortex.set_constant_connection_probability(input_layer_inhibitory_idx, spiking_layer_idx, 1.0f);

    cortex.build_new_synapses();

    cortex.set_constant_neuron_ln_of_excitation_decay_coef(spiking_layer_idx, -1.0f);

    spiking_neuron_guid = { spiking_layer_idx, 0U };

    cortex.neuron_ref(spiking_neuron_guid).spiking_excitation = 20.0f;

    simulatied_time = 0.0f;

    for (natural_32_bit  i = 0; i < NUM_DENDRITES_EXCITATORY; ++i)
        spike_trains_excitatory.insert_spike_train(spike_train(
                EXPECTED_SPIKING_FREQUENCY_EXCITATORY,
                VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY,
                get_random_natural_32_bit_in_range(0U, 10000U),
                simulatied_time
                ));
    for (natural_32_bit  i = 0; i < NUM_DENDRITES_INHIBITORY; ++i)
        spike_trains_inhibitory.insert_spike_train(spike_train(
                EXPECTED_SPIKING_FREQUENCY_INHIBITORY,
                VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY,
                get_random_natural_32_bit_in_range(0U, 10000U),
                simulatied_time
                ));

    spike_trains_excitatory.set_min_train_y(-spike_trains_collection::spike_radius());
    spike_trains_excitatory.set_time_window(SPIKES_TIME_WINDOW);

    spike_trains_inhibitory.set_min_train_y_to_be_after(spike_trains_excitatory);
    spike_trains_inhibitory.set_time_window(SPIKES_TIME_WINDOW);
}


void  exp_spiking_trains::network_update()
{
    simulatied_time += 1.0f / SIMULATION_FREQUENCY;

    cortex.set_constant_simulation_frequency(SIMULATION_FREQUENCY);

    cortex.clear_input_signal_of_neurons();
    cortex.update_neurons(cortex.layer_ref(input_layer_excitatory_idx));
    cortex.update_neurons(cortex.layer_ref(input_layer_inhibitory_idx));

    float_32_bit  input_sum = 0.0f;
    float_32_bit  input_sum_ex = 0.0f;
    float_32_bit  input_sum_in = 0.0f;

    spike_trains_excitatory.set_mean_spiking_frequency(EXPECTED_SPIKING_FREQUENCY_EXCITATORY);
    spike_trains_excitatory.set_spiking_frequency_variation(VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY);
    for (natural_32_bit  i = 0; i < spike_trains_excitatory.size(); ++i)
        if (spike_trains_excitatory.at(i).read_spikes_till_time(simulatied_time, SIMULATION_FREQUENCY) > 0U)
        {
            cortex.neuron_ref({input_layer_excitatory_idx, (natural_16_bit)i}).excitation = 1.0f;
            spike_trains_excitatory.insert_spike(simulatied_time, i);
            input_sum += 1.0f;
            input_sum_ex += 1.0f;
        }
    spike_trains_excitatory.erase_obsolete_spikes(simulatied_time);

    spike_trains_inhibitory.set_mean_spiking_frequency(EXPECTED_SPIKING_FREQUENCY_INHIBITORY);
    spike_trains_inhibitory.set_spiking_frequency_variation(VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY);
    for (natural_32_bit  i = 0; i < spike_trains_inhibitory.size(); ++i)
        if (spike_trains_inhibitory.at(i).read_spikes_till_time(simulatied_time, SIMULATION_FREQUENCY) > 0U)
        {
            cortex.neuron_ref({input_layer_inhibitory_idx, (natural_16_bit)i}).excitation = 1.0f;
            spike_trains_inhibitory.insert_spike(simulatied_time, i);
            input_sum -= 1.0f;
            input_sum_in += 1.0f;
        }
    spike_trains_inhibitory.erase_obsolete_spikes(simulatied_time);

    insert_to_history(input_sum_history, { simulatied_time, input_sum });
    erase_obsolete_records(input_sum_history, HISTORY_TIME_WINDOW, simulatied_time);

    insert_to_history(input_sum_excitatory_history, { simulatied_time, input_sum_ex });
    erase_obsolete_records(input_sum_excitatory_history, HISTORY_TIME_WINDOW, simulatied_time);

    insert_to_history(input_sum_inhibitory_history, { simulatied_time, input_sum_in });
    erase_obsolete_records(input_sum_inhibitory_history, HISTORY_TIME_WINDOW, simulatied_time);

    cortex.update_existing_synapses();
    cortex.update_neurons(cortex.layer_ref(spiking_layer_idx));

    insert_to_history(excitation_history, { simulatied_time, cortex.neuron_excitation(spiking_neuron_guid) });
    erase_obsolete_records(excitation_history, HISTORY_TIME_WINDOW, simulatied_time);

    insert_to_history(input_signal_history, { simulatied_time, cortex.get_neuron(spiking_neuron_guid).input_signal });
    erase_obsolete_records(input_signal_history, HISTORY_TIME_WINDOW, simulatied_time);

    if (cortex.is_neuron_spiking(spiking_neuron_guid))
        insert_to_history(output_spikes_history, { simulatied_time, 1.0f });
    erase_obsolete_records(output_spikes_history, HISTORY_TIME_WINDOW, simulatied_time);

    if (phaser.is_enabled)
        return;

    if (!is_shift_down() && is_key_just_pressed(osi::KEY_U()) != 0UL)
        NUM_DENDRITES_EXCITATORY = std::min(10000U, NUM_DENDRITES_EXCITATORY + 5U);
    if (!is_shift_down() && is_key_just_pressed(osi::KEY_I()) != 0UL)
        NUM_DENDRITES_EXCITATORY = std::max(5U, NUM_DENDRITES_EXCITATORY - 5U);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_U()) != 0UL)
        NUM_DENDRITES_INHIBITORY = std::min(10000U, NUM_DENDRITES_INHIBITORY + 5U);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_I()) != 0UL)
        NUM_DENDRITES_INHIBITORY = std::max(0U, NUM_DENDRITES_INHIBITORY - 5U);

    if (!is_shift_down() && is_key_just_pressed(osi::KEY_T()) != 0UL)
        EXPECTED_SPIKING_FREQUENCY_EXCITATORY = std::min(10000.0f, EXPECTED_SPIKING_FREQUENCY_EXCITATORY + 5.0f);
    if (!is_shift_down() && is_key_just_pressed(osi::KEY_Y()) != 0UL)
        EXPECTED_SPIKING_FREQUENCY_EXCITATORY = std::max(10.0f, EXPECTED_SPIKING_FREQUENCY_EXCITATORY - 5.0f);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_T()) != 0UL)
        EXPECTED_SPIKING_FREQUENCY_INHIBITORY = std::min(10000.0f, EXPECTED_SPIKING_FREQUENCY_INHIBITORY + 5.0f);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_Y()) != 0UL)
        EXPECTED_SPIKING_FREQUENCY_INHIBITORY = std::max(10.0f, EXPECTED_SPIKING_FREQUENCY_INHIBITORY - 5.0f);

    if (!is_shift_down() && is_key_just_pressed(osi::KEY_V()) != 0UL)
        VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY = std::min(10000.0f, VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY + 1.0f);
    if (!is_shift_down() && is_key_just_pressed(osi::KEY_B()) != 0UL)
        VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY = std::max(0.0f, VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY - 1.0f);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_V()) != 0UL)
        VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY = std::min(10000.0f, VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY + 1.0f);
    if (is_shift_down() && is_key_just_pressed(osi::KEY_B()) != 0UL)
        VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY = std::max(0.0f, VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY - 1.0f);
    VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY = std::min(EXPECTED_SPIKING_FREQUENCY_EXCITATORY, VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY);
    VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY = std::min(EXPECTED_SPIKING_FREQUENCY_INHIBITORY, VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY);

    if (is_key_just_pressed(osi::KEY_G()) != 0UL)
        SIMULATION_FREQUENCY = std::min(10000.0f, SIMULATION_FREQUENCY + 10.0f);
    if (is_key_just_pressed(osi::KEY_H()) != 0UL)
        SIMULATION_FREQUENCY = std::max(10.0f, SIMULATION_FREQUENCY - 10.0f);

    if (is_key_pressed(osi::KEY_O()) != 0UL)
        cortex.neuron_ref(spiking_neuron_guid).spiking_excitation += 1.0f * round_seconds();
    if (is_key_pressed(osi::KEY_P()) != 0UL)
        cortex.neuron_ref(spiking_neuron_guid).spiking_excitation -= 1.0f * round_seconds();

    if (is_key_pressed(osi::KEY_N()) != 0UL)
        cortex.set_constant_neuron_ln_of_excitation_decay_coef(
                spiking_layer_idx,
                std::min(
                    -0.1f,
                    cortex.get_layer_constants(spiking_layer_idx).neuron.ln_of_excitation_decay_coef + 2.0f * round_seconds()
                    )
                );
    if (is_key_pressed(osi::KEY_M()) != 0UL)
        cortex.set_constant_neuron_ln_of_excitation_decay_coef(
                spiking_layer_idx, 
                cortex.get_layer_constants(spiking_layer_idx).neuron.ln_of_excitation_decay_coef - 2.0f * round_seconds()
                );
}


void  exp_spiking_trains::scene_setup()
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

    input_sum_folder_guid = ctx.insert_folder(ctx.root_folder(), "input_sum_folder");
    ctx.insert_frame(input_sum_folder_guid, com::invalid_object_guid(), vector3_zero(), quaternion_identity());
}


void  exp_spiking_trains::scene_update()
{
    com::simulation_context&  ctx = *context();

    ctx.frame_set_origin(excitation_frame_guid, {0.0f, VALUE_SCALE * cortex.neuron_excitation(spiking_neuron_guid), 0.0f });
    ctx.frame_set_origin(input_signal_frame_guid, {0.0f, VALUE_SCALE * cortex.get_neuron(spiking_neuron_guid).input_signal, 0.0f });
    ctx.frame_set_origin(spiking_excitation_frame_guid,
                         {0.0f, VALUE_SCALE * cortex.get_neuron(spiking_neuron_guid).spiking_excitation, 0.0f });

    rebuild_history_lines_batch(
            ctx,
            excitation_batch_guid,
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

    spike_trains_excitatory.update_scene(ctx, simulatied_time);
    spike_trains_inhibitory.update_scene(ctx, simulatied_time);

    rebuild_histogram_lines_batch(
        ctx,
        input_sum_batch_guid,
        input_sum_folder_guid,
        "input_sum_batch",
        vector4{0.95f, 0.95f, 0.75f, 1.0f},
        input_sum_history,
        //10.0f / std::max(cortex.get_layer(input_layer_excitatory_idx).neurons.size(),
        //                 cortex.get_layer(input_layer_inhibitory_idx).neurons.size()),
        1.0f,
        //10.0f / SIMULATION_FREQUENCY,
        1.0f,
        false
        );
}


void  exp_spiking_trains::on_restart()
{
    excitation_history.clear();
    input_signal_history.clear();
    excitation_batch_guid = com::invalid_object_guid();
    input_signal_batch_guid = com::invalid_object_guid();
    spike_trains_excitatory.clear(*context());
    spike_trains_inhibitory.clear(*context());
    output_spikes_history.clear();
    input_sum_history.clear();
    input_sum_batch_guid = com::invalid_object_guid();
    input_sum_excitatory_history.clear();
    input_sum_inhibitory_history.clear();
}


void  exp_spiking_trains::custom_render()
{
    if (phaser.is_enabled)
        SLOG("MODE: PHASER (phase=" << (phaser.phase_index + 1U) << "/" << phaser.phases.size() << ")" << "\n");
    else
        SLOG("MODE: INTERACTIVE\n");

    SLOG(
        "NUM_DENDRITES_EXCITATORY=" << NUM_DENDRITES_EXCITATORY << " (requires restart Ctrl+R)\n"
        "NUM_DENDRITES_INHIBITORY=" << NUM_DENDRITES_INHIBITORY << " (requires restart Ctrl+R)\n"
        "EXPECTED_SPIKING_FREQUENCY_EXCITATORY=" << EXPECTED_SPIKING_FREQUENCY_EXCITATORY << "\n"
        "EXPECTED_SPIKING_FREQUENCY_INHIBITORY=" << EXPECTED_SPIKING_FREQUENCY_INHIBITORY << "\n"
        "VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY=" << VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY << "\n"
        "VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY=" << VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY << "\n"
        "SIMULATION_FREQUENCY=" << SIMULATION_FREQUENCY << "\n"
        "spiking_excitation=" << cortex.get_neuron(spiking_neuron_guid).spiking_excitation << "\n"
        "ln_of_excitation_decay_coef=" << cortex.get_layer_constants(spiking_layer_idx).neuron.ln_of_excitation_decay_coef << "\n"
        "output_spiking_frequency=" << output_spikes_history.size() / HISTORY_TIME_WINDOW << "\n"
        "radius95%=" << histogram_radius_95(input_sum_history) << "\n"
    );

    if (HISTORY_TIME_WINDOW > 0.001f)
    {
        float_32_bit  s = 0.0f;
        for (history_record const&  r : input_signal_history)
            s += r.value;
        SLOG("SUM(input_signal_history)=" << s << "\n");
        {
            float_32_bit  s = 0.0f;
            for (history_record const&  r : input_sum_excitatory_history)
                s += r.value;
            SLOG("SUM(EX)=" << s/HISTORY_TIME_WINDOW << "\n");
        }
        {
            float_32_bit  s = 0.0f;
            for (history_record const&  r : input_sum_inhibitory_history)
                s += r.value;
            SLOG("SUM(IN)=" << s/HISTORY_TIME_WINDOW << "\n");
        }
    }
}


void  exp_spiking_trains::help()
{
    SLOG(
        "\tEvaluation of spiking for excitatory and inhibitory spike trains.\n"
        "\tRed sphere & curve - neuron's excitation\n"
        "\tGreen sphere & curve - input signal\n"
        "\tBlue line - neuron's spiking excitation\n"
        "\tG/H - increase/decrease simulation frequency\n"
        "\tO/P - increase/decrease neuron's spiking excitation\n"
        "\tN/M - increase/decrease ln of neuron's excitartion decay coef\n"
        "\t### actions below will take effect after reset (Ctrl+R) ###\n"
        "\tU/I - increase/decrease number of excitatory dendrites\n"
        "\tShift+U/I - increase/decrease number of inhibitory dendrites\n"
        "\tT/Y - increase/decrease excitatory spike trains frequency\n"
        "\tShift+T/Y - increase/decrease inhibitory spike trains frequency\n"
        "\tV/B - increase/decrease variation of excitatory spike trains frequency\n"
        "\tShift+V/B - increase/decrease variation of inhibitory spike trains frequency\n"
        );
}
