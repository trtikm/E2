#include <netlab_cortex_functions/exp_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <algorithm>
#include <cmath>


void  erase_obsolete_records(history_records&  history, float_32_bit const  time_window, float_32_bit const  current_time)
{
    if (time_window < 0.0f)
    {
        history.clear();
        return;
    }
    while (!history.empty() && history.back().time_point + time_window < current_time)
        history.pop_back();
}


void  insert_to_history(history_records&  history, history_record const&  record)
{
    history.push_front(record);
}


history_lines&  history_to_lines(
        history_records const&  records,
        float_32_bit const  x0,
        float_32_bit const  dt_to_dx,
        float_32_bit const  value_scale,
        history_lines&  lines
        )
{
    lines.clear();
    float_32_bit  x = x0;
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
        )
{
    history_lines  lines;
    history_to_lines(history, x0, dt_to_dx, value_scale, lines);
    if (ctx.is_valid_batch_guid(history_batch_guid))
        ctx.erase_batch(history_batch_guid);
    history_batch_guid = lines.empty() ? com::invalid_object_guid() :
                                         ctx.insert_batch_lines3d(under_folder_guid, batch_name, lines, batch_colour);
}


void  history_to_histogram(
        history_records const&  records,
        std::unordered_map<float_32_bit, natural_32_bit>&  hist,
        std::function<float_32_bit(float_32_bit)> const&  value_to_bucket_fn
        )
{
    for (history_record const&  r : records)
    {
        float_32_bit const  b = value_to_bucket_fn(r.value);
        auto  it = hist.find(b);
        if (it == hist.end())
            it = hist.insert({b, 0U}).first;
        ++it->second;
    }
}


history_lines&  histogram_to_lines(
        std::unordered_map<float_32_bit, natural_32_bit> const&  hist,
        float_32_bit const  key_scale,
        float_32_bit const  value_scale,
        bool const  is_keys_axis_horizontal,
        history_lines&  lines
        )
{
    lines.clear();
    if (is_keys_axis_horizontal)
        for (auto const&  key_and_value : hist)
            lines.push_back({
                vector3(key_scale * key_and_value.first, 0.0f, 0.0f),
                vector3(key_scale * key_and_value.first, value_scale * key_and_value.second, 0.0f)
                });
    else
        for (auto const&  key_and_value : hist)
            lines.push_back({
                vector3(0.0f, key_scale * key_and_value.first, 0.0f),
                vector3(value_scale * key_and_value.second, key_scale * key_and_value.first, 0.0f)
                });
    return lines;
}


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
        std::function<float_32_bit(float_32_bit)> const&  value_to_bucket_fn
        )
{
    std::unordered_map<float_32_bit, natural_32_bit>  hist;
    history_to_histogram(history, hist, value_to_bucket_fn);
    history_lines  lines;
    histogram_to_lines(hist, key_scale, value_scale, is_keys_axis_horizontal, lines);
    if (ctx.is_valid_batch_guid(histogram_batch_guid))
        ctx.erase_batch(histogram_batch_guid);
    histogram_batch_guid = lines.empty() ? com::invalid_object_guid() :
                                           ctx.insert_batch_lines3d(under_folder_guid, batch_name, lines, batch_colour);
}


void  clear_connection_probabilities_from_layer(netlab::cortex&  cortex, netlab::cortex::layer_index const  index)
{
    for (netlab::cortex::layer_index  i = 0U; i < cortex.num_layers(); ++i)
        cortex.set_constant_connection_probability(index, i, 0.0f);
}


void  clear_connection_probabilities_to_layer(netlab::cortex&  cortex, netlab::cortex::layer_index const  index)
{
    for (netlab::cortex::layer_index  i = 0U; i < cortex.num_layers(); ++i)
        cortex.set_constant_connection_probability(i, index, 0.0f);
}


spike_train::spike_train(
        float_32_bit const  mean_spiking_frequency_,
        float_32_bit const  spiking_frequency_variation_,
        natural_32_bit const  seed_,
        float_32_bit const  current_time_point_
        )
    : MEAN_SPIKING_FREQUENCY(mean_spiking_frequency_)
    , SPIKING_FREQUENCY_VARIATION(spiking_frequency_variation_)
    , spike_time_points()
    , train_end_time_point(current_time_point_)
    , generator(seed_)
{
    ASSUMPTION(
            MEAN_SPIKING_FREQUENCY > 0.0f &&
            SPIKING_FREQUENCY_VARIATION >= 0.0f &&
            SPIKING_FREQUENCY_VARIATION <= MEAN_SPIKING_FREQUENCY
            );
}


natural_32_bit  spike_train::read_spikes_till_time(float_32_bit const  current_time_point)
{
    if (current_time_point >= train_end_time_point)
        generate_spikes(current_time_point);

    natural_32_bit  num_spikes = 0U;
    while (!spike_time_points.empty() && current_time_point >= spike_time_points.front())
    {
        ++num_spikes;
        spike_time_points.pop_front();
    }

    return num_spikes;
}


void  spike_train::generate_spikes(float_32_bit const  current_time_point)
{
    train_end_time_point += std::floorf(current_time_point - train_end_time_point);

    natural_32_bit const  NUM_SPIKES_TO_GENERATE = (natural_32_bit)(
            MEAN_SPIKING_FREQUENCY +
            get_random_float_32_bit_in_range(-SPIKING_FREQUENCY_VARIATION, SPIKING_FREQUENCY_VARIATION, generator) +
            0.5f
            );
    natural_32_bit  last_index = (natural_32_bit)spike_time_points.size();
    for (natural_32_bit  i = 0U; i < NUM_SPIKES_TO_GENERATE; ++i)
        spike_time_points.push_back(
                get_random_float_32_bit_in_range(train_end_time_point, train_end_time_point + 1.0f, generator)
                );
    std::sort(std::next(spike_time_points.begin(), last_index), spike_time_points.end());

    train_end_time_point += 1.0f;
}


spike_trains_collection::spike_trains_collection(
        float_32_bit const  DT_TO_DX_,
        float_32_bit const  TRAINS_Y_DIRECTION_SIGN_,
        float_32_bit const  TIME_WINDOW_,
        std::string const&  spikes_folder_name_,
        vector4 const&  spike_colour_
        )
    : DT_TO_DX(DT_TO_DX_)
    , MIN_TRAIN_Y(0.0f)
    , TRAINS_Y_DELTA(2.0f * spike_radius() * TRAINS_Y_DIRECTION_SIGN_)
    , TIME_WINDOW(TIME_WINDOW_)
    , spike_trains()
    , spikes_history()
    , spikes_folder_guid(com::invalid_object_guid())
    , folder_name(spikes_folder_name_)
    , spike_batch(gfx::create_solid_sphere(spike_radius(), 1U, spike_colour_))
{
    ASSUMPTION(
        std::fabs(DT_TO_DX) > spike_radius() &&
        std::fabs(TRAINS_Y_DELTA) >= 2.0f * spike_radius() &&
        !folder_name.empty()
        );
}


void  spike_trains_collection::clear(com::simulation_context&  ctx)
{
    if (ctx.is_valid_folder_guid(spikes_folder_guid))
        ctx.erase_non_root_folder(spikes_folder_guid);
    spikes_folder_guid = com::invalid_object_guid();
    spike_trains.clear();
    spikes_history.clear();
}


void  spike_trains_collection::set_min_train_y_to_be_after(spike_trains_collection const&  other)
{
    MIN_TRAIN_Y = other.MIN_TRAIN_Y + other.size() * other.TRAINS_Y_DELTA;
}


void  spike_trains_collection::erase_obsolete_spikes(float_32_bit const  current_time)
{
    erase_obsolete_records(spikes_history, TIME_WINDOW, current_time);    
}


void  spike_trains_collection::insert_spike(float_32_bit const  current_time, natural_32_bit const  train_index)
{
    insert_to_history(spikes_history, { current_time, MIN_TRAIN_Y + train_index * TRAINS_Y_DELTA });
}


void  spike_trains_collection::update_scene(com::simulation_context&  ctx, float_32_bit const  current_time)
{
    if (spikes_folder_guid == com::invalid_object_guid())
        spikes_folder_guid = ctx.insert_folder(ctx.root_folder(), folder_name, true);

    com::simulation_context::folder_content_type const*  fct = &ctx.folder_content(spikes_folder_guid);
    while (fct->child_folders.size() > spikes_history.size())
        ctx.erase_non_root_folder(fct->child_folders.begin()->second);

    while (ctx.folder_content(spikes_folder_guid).child_folders.size() < spikes_history.size())
    {
        // Next line may invalidate 'fct' pointer!
        com::object_guid const  folder_guid = ctx.insert_folder(spikes_folder_guid, "spike", true);
        ctx.insert_frame(folder_guid);
        ctx.insert_batch(folder_guid, "spike_batch", spike_batch);
    }

    auto  it = ctx.folder_content(spikes_folder_guid).child_folders.begin();
    for (history_record const&  r : spikes_history)
    {
        ctx.frame_set_origin(
                ctx.find_closest_frame(it->second, true),
                vector3(DT_TO_DX * (current_time - r.time_point), r.value, 0.0f)
                );
        ++it;
    }
}
