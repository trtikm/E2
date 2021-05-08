#include <netlab/statistics.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


statistics::probe::probe()
    : num_spikes_received(0U)
    , num_spikes_produced(0U)
    , num_connected_input_sockets(0U)
    , num_connected_output_sockets(0U)
    , num_disconnected_input_sockets(0U)
    , num_disconnected_output_sockets(0U)
{}


statistics::overall::overall()
    : num_spikes(0U)
    , num_connected_sockets(0U)
    , num_disconnected_sockets(0U)
{}


statistics::statistics()
    : NUM_ROUNDS_PER_SNAPSHOT(0U)
    , SNAPSHOTS_HISTORY_SIZE(0U)
    , RATIO_OF_PROBED_UNITS_PER_LAYER(0.0f)

    , num_passed_rounds(0U)
    , num_passed_rounds_in_current_snapshot(0U)
    , probes_history()
    , overall_history({})
{
//    for (natural_8_bit  layer_idx = 0U; layer_idx < layers_configs.size(); ++layer_idx)
//    {
//        network_layer::config const&  layer_config = layers_configs.at(layer_idx);
//
//        natural_16_bit const  num_probes =
//                (natural_16_bit)(layer_config.sign_and_geometry_info.num_units * cfg.RATIO_OF_PROBED_UNITS_PER_LAYER + 0.5f);
//        if (num_probes == 0U)
//            continue;
//
//        natural_16_bit const  stride = layer_config.sign_and_geometry_info.num_units / num_probes;
//        for (natural_16_bit  unit_idx = 0U; unit_idx < layer_config.sign_and_geometry_info.num_units; unit_idx += stride)
//            probes_history.insert({ {(natural_8_bit)(num_input_layers + layer_idx), unit_idx, 0U}, { {} } });
//    }
}


void  statistics::on_next_round()
{
    TMPROF_BLOCK();

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


void  statistics::on_spike_received(uid const  id)
{
    TMPROF_BLOCK();

    if (!enabled())
        return;

    auto const  it = probes_history.find(uid::as_unit(id));
    if (it != probes_history.end())
        ++it->second.front().num_spikes_received;
}


void  statistics::on_spike_produced(uid const  id)
{
    TMPROF_BLOCK();

    if (!enabled())
        return;

    ++overall_history.front().num_spikes;

    auto const  it = probes_history.find(uid::as_unit(id));
    if (it != probes_history.end())
        ++it->second.front().num_spikes_produced;
}

void  statistics::on_connect(uid const  iid, uid const  oid)
{
    TMPROF_BLOCK();

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


void  statistics::on_disconnect(
        std::vector<input_socket> const&  inputs,
        uid const  input_socket_id,
        std::vector<output_socket> const&  outputs,
        uid const  output_socket_id
        )
{
    TMPROF_BLOCK();

    if (!enabled())
        return;

    ++overall_history.front().num_disconnected_sockets;

    uid const  iid = uid::as_unit(outputs.at(output_socket_id.socket).other);
    auto const  iit = probes_history.find(iid);
    if (iit != probes_history.end())
        ++iit->second.front().num_disconnected_input_sockets;

    uid const  oid = uid::as_unit(inputs.at(input_socket_id.socket).other);
    auto const  oit = probes_history.find(oid);
    if (oit != probes_history.end())
        ++oit->second.front().num_disconnected_output_sockets;
}


}
