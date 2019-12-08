#include <netlab/simple_network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab { namespace simple {


network::statistics::config::config(
        natural_32_bit const  num_rounds_per_snapshot,
        natural_32_bit const  snapshots_history_size,
        float_32_bit const  ratio_of_probed_units_per_layer
        )
    : NUM_ROUNDS_PER_SNAPSHOT(num_rounds_per_snapshot)
    , SNAPSHOTS_HISTORY_SIZE(snapshots_history_size)
    , RATIO_OF_PROBED_UNITS_PER_LAYER(ratio_of_probed_units_per_layer)
{
    ASSUMPTION(RATIO_OF_PROBED_UNITS_PER_LAYER >= 0.0f && RATIO_OF_PROBED_UNITS_PER_LAYER <= 1.0f);
}


network::statistics::probe::probe()
    : num_events_received(0U)
    , num_events_produced(0U)
    , num_connected_input_sockets(0U)
    , num_connected_output_sockets(0U)
    , num_disconnected_input_sockets(0U)
    , num_disconnected_output_sockets(0U)
{}


network::statistics::overall::overall()
    : num_events(0U)
    , num_connected_sockets(0U)
    , num_disconnected_sockets(0U)
{}


network::statistics::statistics(config const&  cfg, std::vector<network_layer::config> const&  layers_configs)
    : NUM_ROUNDS_PER_SNAPSHOT(cfg.NUM_ROUNDS_PER_SNAPSHOT)
    , SNAPSHOTS_HISTORY_SIZE(cfg.SNAPSHOTS_HISTORY_SIZE)
    , num_passed_rounds(0UL)
    , num_passed_rounds_in_current_snapshot(0U)
    , probes_history()
    , overall_history()
{
    ASSUMPTION(cfg.RATIO_OF_PROBED_UNITS_PER_LAYER >= 0.0f && cfg.RATIO_OF_PROBED_UNITS_PER_LAYER <= 1.0f);

    if (!enabled())
        return;

    for (natural_8_bit  layer_idx = 0U; layer_idx < layers_configs.size(); ++layer_idx)
    {
        network_layer::config const&  layer_config = layers_configs.at(layer_idx);

        natural_16_bit const  num_probes = (natural_16_bit)(layer_config.num_units * cfg.RATIO_OF_PROBED_UNITS_PER_LAYER + 0.5f);
        if (num_probes == 0U)
            continue;

        natural_16_bit const  stride = layer_config.num_units / num_probes;
        for (natural_16_bit  unit_idx = 0U; unit_idx < layer_config.num_units; unit_idx += stride)
            probes_history.insert({ {layer_idx, unit_idx, 0U}, { {} } });
    }

    overall_history.push_front({});
}


void  network::statistics::on_next_round()
{
    if (!enabled())
        return;

    ++num_passed_rounds;

    ++num_passed_rounds_in_current_snapshot;
    if (num_passed_rounds_in_current_snapshot != NUM_ROUNDS_PER_SNAPSHOT)
        return;
    num_passed_rounds_in_current_snapshot = 1U;

    for (auto&  uid_and_probes : probes_history)
    {
        uid_and_probes.second.push_front({});
        while (uid_and_probes.second.size() > SNAPSHOTS_HISTORY_SIZE + 1U)
            uid_and_probes.second.pop_back();
    }
    overall_history.push_front({});
    while (overall_history.size() > SNAPSHOTS_HISTORY_SIZE + 1U)
        overall_history.pop_back();
}


void  network::statistics::on_event_received(uid const  id)
{
    if (!enabled())
        return;

    auto const  it = probes_history.find(uid::as_unit(id));
    if (it != probes_history.end())
        ++it->second.front().num_events_received;
}


void  network::statistics::on_event_produced(uid const  id)
{
    if (!enabled())
        return;

    ++overall_history.front().num_events;

    auto const  it = probes_history.find(uid::as_unit(id));
    if (it != probes_history.end())
        ++it->second.front().num_events_produced;
}

void  network::statistics::on_connect(uid const  iid, uid const  oid)
{
    if (!enabled())
        return;

    ++overall_history.front().num_connected_sockets;

    auto const  iit = probes_history.find(uid::as_unit(iid));
    if (iit != probes_history.end())
        ++iit->second.front().num_connected_input_sockets;

    auto const  oit = probes_history.find(uid::as_unit(oid));
    if (oit != probes_history.end())
        ++oit->second.front().num_connected_output_sockets;
}


void  network::statistics::on_disconnect(
        std::vector<input_socket> const&  inputs,
        natural_16_bit const  input_socket,
        std::vector<output_socket> const&  outputs,
        natural_16_bit const  output_socket
        )
{
    if (!enabled())
        return;

    ++overall_history.front().num_disconnected_sockets;

    uid const  iid = uid::as_unit(outputs.at(output_socket).other);
    auto const  iit = probes_history.find(iid);
    if (iit != probes_history.end())
        ++iit->second.front().num_disconnected_input_sockets;

    uid const  oid = uid::as_unit(inputs.at(input_socket).other);
    auto const  oit = probes_history.find(oid);
    if (oit != probes_history.end())
        ++oit->second.front().num_disconnected_output_sockets;
}


}}
