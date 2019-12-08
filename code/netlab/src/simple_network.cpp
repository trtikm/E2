#include <netlab/simple_network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab { namespace simple {


network::network(
        config const&  network_config,
        std::vector<network_layer::config> const&  layers_configs,
        statistics::config const&  stats_config,
        natural_32_bit const  seed
        )
    : layers()

    , events()
    , next_events()

    , open_inputs()
    , open_outputs()

    , random_generator(seed)

    , stats(stats_config, layers_configs)
{
    TMPROF_BLOCK();

    ASSUMPTION(!layers_configs.empty());

    EVENT_POTENTIAL_MAGNITUDE = config::configurations.at(network_config.config_name).EVENT_POTENTIAL_MAGNITUDE;

    layers.resize(layers_configs.size());
    for (natural_8_bit  layer_idx = 0U; layer_idx < layers_configs.size(); ++layer_idx)
    {
        network_layer::config const&  layer_config = layers_configs.at(layer_idx);
        ASSUMPTION(layer_config.num_units > 0U && layer_config.num_sockets_per_unit > 0U);

        network_layer&  layer = layers.at(layer_idx);

        {
            auto const&  cfg = network_layer::config::configurations_of_events.at(layer_config.events_config_name);
            layer.EVENT_TREASHOLD = cfg.EVENT_TREASHOLD;
            layer.EVENT_RECOVERY_POTENTIAL = cfg.EVENT_RECOVERY_POTENTIAL;
            layer.EVENT_POTENTIAL_DECAY_COEF = cfg.EVENT_POTENTIAL_DECAY_COEF;
        }
        layer.EVENT_POTENTIAL_SIGN = layer_config.is_excitatory ? 1.0f : -1.0f;
        {
            auto const&  cfg = network_layer::config::configurations_of_weights.at(layer_config.weithts_config_name);
            layer.WEIGHT_PER_POTENTIAL = cfg.WEIGHT_PER_POTENTIAL;
            layer.WEIGHT_EQUILIBRIUM_TREASHOLD = cfg.WEIGHT_EQUILIBRIUM_TREASHOLD;
        }
        {
            auto const&  cfg = network_layer::config::configurations_of_sockets.at(layer_config.sockets_config_name);
            layer.SOCKET_CONNECTION_TREASHOLD = cfg.SOCKET_CONNECTION_TREASHOLD;
            layer.SOCKET_DISCONNECTION_TREASHOLD = cfg.SOCKET_DISCONNECTION_TREASHOLD;
        }

        layer.units.resize(layer_config.num_units);
        for (natural_16_bit unit_idx = 0U; unit_idx < layer_config.num_units; ++unit_idx)
        {
            computation_unit&  unit = layer.units.at(unit_idx);
            unit.event_potential = layer.EVENT_RECOVERY_POTENTIAL;
            unit.inputs.reserve(layer_config.num_sockets_per_unit);
            unit.outputs.reserve(layer_config.num_sockets_per_unit);
        }
    }

    natural_32_bit  num_units_total, expected_remainder;
    std::vector<natural_16_bit>  num_remaining_layer_iterations;
    {
        num_units_total = 0U;
        expected_remainder = 0U;
        num_remaining_layer_iterations.reserve(layers_configs.size());
        for (network_layer::config const&  layer_config : layers_configs)
        {
            num_units_total += layer_config.num_units;
            expected_remainder += layer_config.num_sockets_per_unit;
            num_remaining_layer_iterations.push_back(layer_config.num_sockets_per_unit);
        }
    }
    open_inputs.reserve(num_units_total + expected_remainder);
    open_outputs.reserve(num_units_total + expected_remainder);
    while (true)
    {
        bool  work_lists_updated = false;
        for (natural_8_bit  i = 0U; i < layers.size(); ++i)
            if (num_remaining_layer_iterations.at(i) > 0U)
            {
                --num_remaining_layer_iterations.at(i);
                work_lists_updated = true;
                for (natural_16_bit j = 0U, n = (natural_16_bit)layers.at(i).units.size(); j < n; ++j)
                {
                    uid const  id{ i, j, 0U };
                    open_inputs.push_back(id);
                    open_outputs.push_back(id);
                }
            }
        if (!work_lists_updated)
            break;
        update_open_sockets();
    }
}


void  network::next_round()
{
    TMPROF_BLOCK();

    stats.on_next_round();

    update_computation_units();
    deliver_events();
    update_open_sockets();
}


void   network::update_computation_units()
{
    TMPROF_BLOCK();

    for (natural_8_bit  i = 0U; i < layers.size(); ++i)
    {
        network_layer&  layer = layers.at(i);
        for (natural_16_bit j = 0U; j < layer.units.size(); ++j)
        {
            computation_unit&  unit = layer.units.at(j);
            if (unit.event_potential >= layer.EVENT_TREASHOLD)
            {
                uid const  id{ i, j, 0U };
                next_events.push_back(id);
                unit.event_potential = layer.EVENT_RECOVERY_POTENTIAL;

                stats.on_event_produced(id);
            }
            else
                unit.event_potential *= layer.EVENT_POTENTIAL_DECAY_COEF;
        }
    }
}


void  network::deliver_events()
{
    TMPROF_BLOCK();

    for (uid  id : events)
    {
        network_layer&  layer = layers.at(id.layer);
        computation_unit&  unit = layer.units.at(id.unit);
        std::vector<input_socket>&  inputs = unit.inputs;
        std::vector<output_socket>&  outputs = unit.outputs;

        for (natural_16_bit  i = 0U; i < inputs.size(); ++i)
        {
            input_socket&  input = inputs.at(i);

            uid const  other_id = input.other;
            network_layer&  other_layer = layers.at(other_id.layer);
            computation_unit&  other_unit = other_layer.units.at(other_id.unit);

            if (input.weight < layer.SOCKET_DISCONNECTION_TREASHOLD)
            {
                disconnect(inputs, i, other_unit.outputs, other_id.socket);
                open_inputs.push_back(uid::as_unit(id));
                open_outputs.push_back(uid::as_unit(other_id));
            }
            else
            {
                float_32_bit const  other_excitation_level = 1.0f - (other_unit.event_potential - other_layer.EVENT_RECOVERY_POTENTIAL) /
                                                                    (other_layer.EVENT_TREASHOLD - other_layer.EVENT_RECOVERY_POTENTIAL);
                input.weight += other_layer.WEIGHT_PER_POTENTIAL * (other_excitation_level - other_layer.WEIGHT_EQUILIBRIUM_TREASHOLD);
            }
        }

        for (natural_16_bit  i = 0U; i < outputs.size(); ++i)
        {
            output_socket&  output = outputs.at(i);

            uid const  other_id = output.other;
            network_layer&  other_layer = layers.at(other_id.layer);
            computation_unit&  other_unit = other_layer.units.at(other_id.unit);
            input_socket&  other_input = other_unit.inputs.at(other_id.socket);

            if (other_input.weight < other_layer.SOCKET_DISCONNECTION_TREASHOLD)
            {
                disconnect(other_unit.inputs, other_id.socket, outputs, i);
                open_inputs.push_back(uid::as_unit(other_id));
                open_outputs.push_back(uid::as_unit(id));
            }
            else
            {
                other_unit.event_potential += EVENT_POTENTIAL_MAGNITUDE * layer.EVENT_POTENTIAL_SIGN * other_input.weight;

                float_32_bit const  other_excitation_level = (other_unit.event_potential - other_layer.EVENT_RECOVERY_POTENTIAL) /
                                                             (other_layer.EVENT_TREASHOLD - other_layer.EVENT_RECOVERY_POTENTIAL);
                other_input.weight += other_layer.WEIGHT_PER_POTENTIAL * (other_excitation_level - other_layer.WEIGHT_EQUILIBRIUM_TREASHOLD);

                stats.on_event_received(other_id);
            }
        }
    }

    events.clear();
    std::swap(events, next_events);
}


void  network::update_open_sockets()
{
    TMPROF_BLOCK();

    INVARIANT(open_inputs.size() == open_outputs.size());

    for (natural_32_bit  o_idx = 0U; o_idx < open_outputs.size(); )
    {
        natural_32_bit const  i_idx = (natural_32_bit)get_random_natural_32_bit_in_range(0U, open_inputs.size() - 1U, random_generator);

        uid&  iid = open_inputs.at(i_idx);
        uid&  oid = open_outputs.at(o_idx);
        if (connect(iid, oid))
        {
            iid = open_inputs.back();
            open_inputs.pop_back();

            oid = open_outputs.back();
            open_outputs.pop_back();
        }
        else
            ++o_idx;
    }
}


bool  network::connect(uid  iid, uid  oid)
{
    if (iid.layer == oid.layer && iid.unit == oid.unit)
        return false;

    network_layer&  input_layer = layers.at(iid.layer);
    std::vector<input_socket>&  inputs = input_layer.units.at(iid.unit).inputs;
    std::vector<output_socket>&  outputs = layers.at(oid.layer).units.at(oid.unit).outputs;

    iid.socket = inputs.size();
    oid.socket = outputs.size();

    inputs.push_back({ oid, input_layer.SOCKET_CONNECTION_TREASHOLD });
    outputs.push_back({ iid });

    stats.on_connect(iid, oid);

    return true;
}


void  network::disconnect(
        std::vector<input_socket>&  inputs,
        natural_16_bit const  input_socket,
        std::vector<output_socket>&  outputs,
        natural_16_bit const  output_socket
        )
{
    stats.on_disconnect(inputs, input_socket, outputs, output_socket);

    uid const  oid = inputs.back().other;
    layers.at(oid.layer).units.at(oid.unit).outputs.at(oid.socket).other.socket = input_socket;

    uid const  iid = outputs.back().other;
    layers.at(iid.layer).units.at(iid.unit).inputs.at(iid.socket).other.socket = output_socket;

    inputs.at(input_socket) = inputs.back();
    inputs.pop_back();

    outputs.at(output_socket) = outputs.back();
    outputs.pop_back();
}


}}
