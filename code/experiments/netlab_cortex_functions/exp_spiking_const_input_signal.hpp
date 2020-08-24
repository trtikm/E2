#ifndef E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_SPIKING_CONST_INPUT_SIGNAL_HPP_INCLUDED
#   define E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_SPIKING_CONST_INPUT_SIGNAL_HPP_INCLUDED

#   include <netlab_cortex_functions/simulator_base.hpp>
#   include <netlab_cortex_functions/exp_utils.hpp>
#   include <netlab/cortex.hpp>
#   include <com/simulator.hpp>
#   include <deque>
#   include <utility>


struct  exp_spiking_const_input_signal : public simulator_base
{
    exp_spiking_const_input_signal();
    void  network_setup() override;
    void  network_update() override;
    void  scene_setup() override;
    void  scene_update() override;
    void  custom_render() override;
    void  on_restart() override;
    void  help() override;

private:
    natural_32_bit  NUM_DENDRITES;
    float_32_bit  EXPECTED_SPIKING_FREQUENCY;
    float_32_bit  EXPECTED_SIMULATION_FREQUENCY;

    float_32_bit  VALUE_SCALE;
    float_32_bit  HISTORY_TIME_WINDOW;
    float_32_bit  HISTORY_DT_TO_DX;

    natural_16_bit  layer_idx;
    netlab::cortex::neuron_guid  neuron_guid;

    com::object_guid  excitation_frame_guid;
    com::object_guid  input_signal_frame_guid;
    com::object_guid  spiking_excitation_frame_guid;

    float_32_bit  num_input_spikes_per_second;

    com::object_guid  history_folder_guid;
    com::object_guid  excitation_curve_batch_guid;
    history_records  excitation_history;
    com::object_guid  input_signal_batch_guid;
    history_records  input_signal_history;
    float_32_bit  simulatied_time;
};


#endif
