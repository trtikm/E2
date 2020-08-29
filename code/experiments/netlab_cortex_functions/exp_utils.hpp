#ifndef E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_UTILS_HPP_INCLUDED
#   define E2_EXPERIMENTS_NETLAB_CORTEX_FUNCTIONS_EXP_UTILS_HPP_INCLUDED

#   include <com/simulator.hpp>
#   include <netlab/cortex.hpp>
#   include <angeo/tensor_math.hpp>
#   include <utility/random.hpp>
#   include <vector>
#   include <deque>
#   include <string>
#   include <unordered_map>
#   include <functional>
#   include <utility>


struct  history_record
{
    float_32_bit  time_point;
    float_32_bit  value;
};

using  history_records = std::deque<history_record>;
using  history_lines = std::vector<std::pair<vector3, vector3> >;


void  erase_obsolete_records(history_records&  history, float_32_bit const  time_window, float_32_bit const  current_time);
void  insert_to_history(history_records&  history, history_record const&  record);

history_lines&  history_to_lines(
        history_records const&  records,
        float_32_bit const  x0,
        float_32_bit const  dt_to_dx,
        float_32_bit const  value_scale,
        history_lines&  lines
        );

void  rebuild_history_lines_batch(
        com::simulation_context&  ctx,
        com::object_guid&  history_batch_guid,
        com::object_guid const  under_folder_guid,
        std::string const&  batch_name,
        vector4 const&  batch_colour,
        history_records const&  history,
        float_32_bit const  x0,
        float_32_bit const  dt_to_dx,
        float_32_bit const  value_scale
        );

void  history_to_histogram(
        history_records const&  records,
        std::unordered_map<float_32_bit, natural_32_bit>&  hist,
        std::function<float_32_bit(float_32_bit)> const&  value_to_bucket_fn =
                [](float_32_bit const  x) { return std::roundf(x); }
        );

history_lines&  histogram_to_lines(
        std::unordered_map<float_32_bit, natural_32_bit> const&  hist,
        float_32_bit const  key_scale,
        float_32_bit const  value_scale,
        bool const  is_keys_axis_horizontal,
        history_lines&  lines
        );

void  rebuild_histogram_lines_batch(
        com::simulation_context&  ctx,
        com::object_guid&  histogram_batch_guid,
        com::object_guid const  under_folder_guid,
        std::string const&  batch_name,
        vector4 const&  batch_colour,
        history_records const&  history,
        float_32_bit const  key_scale,
        float_32_bit const  value_scale,
        bool const  is_keys_axis_horizontal,
        std::function<float_32_bit(float_32_bit)> const&  value_to_bucket_fn =
                [](float_32_bit const  x) { return std::roundf(x); }
        );


void  clear_connection_probabilities_from_layer(netlab::cortex&  cortex, netlab::cortex::layer_index const  index);
void  clear_connection_probabilities_to_layer(netlab::cortex&  cortex, netlab::cortex::layer_index const  index);

inline void  clear_connection_probabilities(netlab::cortex&  cortex, netlab::cortex::layer_index const  index)
{
    clear_connection_probabilities_from_layer(cortex, index);
    clear_connection_probabilities_to_layer(cortex, index);
}


struct  spike_train
{
    spike_train(
            float_32_bit const  mean_spiking_frequency_,
            float_32_bit const  spiking_frequency_variation_,
            natural_32_bit const  seed_,
            float_32_bit const  current_time_point_ = 0.0f
            );
    natural_32_bit  read_spikes_till_time(float_32_bit const  current_time_point);
private:
    void  generate_spikes(float_32_bit const  current_time_point);

    float_32_bit  MEAN_SPIKING_FREQUENCY;
    float_32_bit  SPIKING_FREQUENCY_VARIATION;
    std::deque<float_32_bit>  spike_time_points;
    float_32_bit  train_end_time_point;
    random_generator_for_natural_32_bit  generator;
};


struct  spike_trains_collection
{
    spike_trains_collection(
            float_32_bit const  DT_TO_DX_,
            float_32_bit const  TRAINS_Y_DIRECTION_SIGN_,
            float_32_bit const  TIME_WINDOW_,
            std::string const&  spikes_folder_name_,
            vector4 const&  spike_colour_
            );
    static inline constexpr float_32_bit  spike_radius() { return 0.05f; }
    void  clear(com::simulation_context&  ctx);
    void  set_min_train_y(float_32_bit const  value) { MIN_TRAIN_Y = value;}
    void  set_min_train_y_to_be_after(spike_trains_collection const&  other);
    natural_32_bit  size() const { return (natural_32_bit)spike_trains.size(); }
    spike_train&  at(natural_32_bit const  index) { return spike_trains.at(index); }
    spike_train const&  at(natural_32_bit const  index) const { return spike_trains.at(index); }
    void  insert_spike_train(spike_train const&  train) { spike_trains.push_back(train); }
    void  erase_obsolete_spikes(float_32_bit const  current_time);
    void  insert_spike(float_32_bit const  current_time, natural_32_bit const  train_index);
    void  update_scene(com::simulation_context&  ctx, float_32_bit const  current_time);
private:
    float_32_bit  DT_TO_DX;
    float_32_bit  MIN_TRAIN_Y;
    float_32_bit  TRAINS_Y_DELTA;
    float_32_bit  TIME_WINDOW;
    std::vector<spike_train>  spike_trains;
    history_records  spikes_history;
    com::object_guid  spikes_folder_guid;
    std::string  folder_name;
    gfx::batch  spike_batch;
};


#endif
