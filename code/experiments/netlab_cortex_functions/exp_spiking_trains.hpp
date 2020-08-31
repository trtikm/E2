#ifndef E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_SPIKING_TRAINS_HPP_INCLUDED
#   define E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_SPIKING_TRAINS_HPP_INCLUDED

#   include <netlab_cortex_functions/simulator_base.hpp>
#   include <netlab_cortex_functions/exp_utils.hpp>
#   include <netlab/cortex.hpp>
#   include <com/simulator.hpp>
#   include <deque>
#   include <utility>


struct  exp_spiking_trains : public simulator_base
{
    exp_spiking_trains();
    bool  move_to_next_phase() override;
    void  network_setup() override;
    void  network_update() override;
    void  scene_setup() override;
    void  scene_update() override;
    void  custom_render() override;
    void  on_restart() override;
    void  help() override;

private:

    struct  phase_manager
    {
        struct  phase_info
        {
            phase_info(natural_32_bit  NUM_DENDRITES_EXCITATORY_, float_32_bit  SIMULATION_FREQUENCY_);
            // constsants:
            natural_32_bit  NUM_DENDRITES_EXCITATORY;
            natural_32_bit  NUM_DENDRITES_INHIBITORY;
            float_32_bit  SIMULATION_FREQUENCY;
            float_32_bit  RECORD_TIME_WINDOW;
            natural_32_bit  MAX_RECORDS;
            // data:
            float_32_bit  phase_time;
            // results:
            std::vector<float_32_bit>  r95s;
            float_32_bit  r95_average;
        };

        phase_manager();

        std::vector<phase_info>  phases;
        natural_32_bit  phase_index;
        bool  is_enabled;
    };

    natural_32_bit  NUM_DENDRITES_EXCITATORY;
    natural_32_bit  NUM_DENDRITES_INHIBITORY;
    float_32_bit  EXPECTED_SPIKING_FREQUENCY_EXCITATORY;
    float_32_bit  EXPECTED_SPIKING_FREQUENCY_INHIBITORY;
    float_32_bit  VARIATION_OF_SPIKING_FREQUENCY_EXCITATORY;
    float_32_bit  VARIATION_OF_SPIKING_FREQUENCY_INHIBITORY;
    float_32_bit  SIMULATION_FREQUENCY;

    float_32_bit  VALUE_SCALE;
    float_32_bit  HISTORY_TIME_WINDOW;
    float_32_bit  HISTORY_DT_TO_DX;

    float_32_bit  SPIKES_TIME_WINDOW;

    netlab::cortex::layer_index  input_layer_excitatory_idx;
    netlab::cortex::layer_index  input_layer_inhibitory_idx;
    netlab::cortex::layer_index  spiking_layer_idx;
    netlab::cortex::neuron_guid  spiking_neuron_guid;

    com::object_guid  excitation_frame_guid;
    com::object_guid  input_signal_frame_guid;
    com::object_guid  spiking_excitation_frame_guid;

    com::object_guid  history_folder_guid;
    com::object_guid  excitation_batch_guid;
    history_records  excitation_history;
    com::object_guid  input_signal_batch_guid;
    history_records  input_signal_history;
    float_32_bit  simulatied_time;

    spike_trains_collection  spike_trains_excitatory;
    spike_trains_collection  spike_trains_inhibitory;
    history_records  output_spikes_history;

    history_records  input_sum_history;
    com::object_guid  input_sum_folder_guid;
    com::object_guid  input_sum_batch_guid;

    phase_manager  phaser;
};


#endif
