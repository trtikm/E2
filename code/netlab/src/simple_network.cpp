#include <netlab/simple_network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab { namespace simple {


network::network(
        config const&  network_config,
        std::vector<network_layer::config::sign_and_geometry> const&  input_layers_configs,
        INPUTS_DISTRUBUTION_STRATEGY const  inputs_distrubution_strategy,
        std::vector<network_layer::config> const&  layers_configs,
        std::vector<natural_8_bit> const&  output_layer_indices,
        statistics::config const&  stats_config,
        natural_32_bit const  seed
        )
    : layers()

    , events()
    , next_events()

    , open_inputs()
    , open_outputs()

    , output_events()

    , random_generator(seed)

    , stats(stats_config, (natural_8_bit)input_layers_configs.size(), layers_configs)
{
    TMPROF_BLOCK();

    ASSUMPTION(!layers_configs.empty());
    ASSUMPTION(input_layers_configs.size() + layers_configs.size() <= uid::MAX_NUM_LAYERS);

    EVENT_POTENTIAL_MAGNITUDE = config::configurations.at(network_config.config_name).EVENT_POTENTIAL_MAGNITUDE;
    NUM_INPUT_LAYERS = (natural_8_bit)input_layers_configs.size();

    natural_8_bit const  num_layers = NUM_INPUT_LAYERS + (natural_8_bit)layers_configs.size();

    OUTPUT_LAYER_INDICES.reserve(output_layer_indices.size());
    for (natural_8_bit  idx : output_layer_indices)
    {
        ASSUMPTION(idx >= NUM_INPUT_LAYERS && idx < num_layers);
        OUTPUT_LAYER_INDICES.insert(idx);
    }

    layers.resize(num_layers);

    for (natural_8_bit  input_layer_idx = 0U; input_layer_idx < NUM_INPUT_LAYERS; ++input_layer_idx)
    {
        network_layer::config::sign_and_geometry const&  input_layer_config = input_layers_configs.at(input_layer_idx);
        ASSUMPTION(input_layer_config.num_units > 0U && input_layer_config.num_sockets_per_unit > 0U);

        network_layer&  input_layer = layers.at(input_layer_idx);

        input_layer.EVENT_TREASHOLD = 0.0f;                 // Not used!
        input_layer.EVENT_RECOVERY_POTENTIAL = 0.0f;        // Not used!
        input_layer.EVENT_POTENTIAL_DECAY_COEF = 0.0f;      // Not used!

        input_layer.EVENT_POTENTIAL_SIGN = input_layer_config.is_excitatory ? 1.0f : -1.0f;

        input_layer.WEIGHT_PER_POTENTIAL = 0.0f;            // Not used!
        input_layer.WEIGHT_EQUILIBRIUM_TREASHOLD = 0.0f;    // Not used!
        input_layer.SOCKET_CONNECTION_TREASHOLD = 0.0f;     // Not used!
        input_layer.SOCKET_DISCONNECTION_TREASHOLD = 0.0f;  // Not used!

        input_layer.units.resize(input_layer_config.num_units);
        for (natural_16_bit unit_idx = 0U; unit_idx < input_layer_config.num_units; ++unit_idx)
        {
            computation_unit&  unit = input_layer.units.at(unit_idx);
            unit.event_potential = 0.0f;                    // Not used!
            unit.inputs.clear();                            // Not used!
            unit.outputs.reserve(input_layer_config.num_sockets_per_unit);
        }
    }

    for (natural_8_bit  layer_idx = 0U; layer_idx < layers_configs.size(); ++layer_idx)
    {
        network_layer::config const&  layer_config = layers_configs.at(layer_idx);
        network_layer::config::sign_and_geometry const&  sign_and_geometry_info = layer_config.sign_and_geometry_info;
        ASSUMPTION(sign_and_geometry_info.num_units > 0U && sign_and_geometry_info.num_sockets_per_unit > 0U);

        network_layer&  layer = layers.at(layer_idx);

        {
            auto const&  cfg = network_layer::config::configurations_of_events.at(layer_config.events_config_name);
            layer.EVENT_TREASHOLD = cfg.EVENT_TREASHOLD;
            layer.EVENT_RECOVERY_POTENTIAL = cfg.EVENT_RECOVERY_POTENTIAL;
            layer.EVENT_POTENTIAL_DECAY_COEF = cfg.EVENT_POTENTIAL_DECAY_COEF;
        }
        layer.EVENT_POTENTIAL_SIGN = sign_and_geometry_info.is_excitatory ? 1.0f : -1.0f;
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

        layer.units.resize(sign_and_geometry_info.num_units);
        for (natural_16_bit unit_idx = 0U; unit_idx < sign_and_geometry_info.num_units; ++unit_idx)
        {
            computation_unit&  unit = layer.units.at(unit_idx);
            unit.event_potential = layer.EVENT_RECOVERY_POTENTIAL;
            unit.inputs.reserve(sign_and_geometry_info.num_sockets_per_unit);
            unit.outputs.reserve(sign_and_geometry_info.num_sockets_per_unit);
        }
    }

    natural_32_bit  num_units_total, expected_remainder, num_output_sockets_of_all_input_layers;
    std::vector<natural_16_bit>  num_remaining_layer_iterations;
    {
        num_units_total = 0U;
        expected_remainder = 0U;
        num_output_sockets_of_all_input_layers = 0U;
        num_remaining_layer_iterations.reserve(layers.size());
        for (network_layer::config::sign_and_geometry const&  sign_and_geometry_info : input_layers_configs)
        {
            num_units_total += sign_and_geometry_info.num_units;
            expected_remainder += sign_and_geometry_info.num_sockets_per_unit;
            num_remaining_layer_iterations.push_back(sign_and_geometry_info.num_sockets_per_unit);
            num_output_sockets_of_all_input_layers +=
                    (natural_32_bit)sign_and_geometry_info.num_units * (natural_32_bit)sign_and_geometry_info.num_sockets_per_unit;
        }
        for (network_layer::config const&  layer_config : layers_configs)
        {
            num_units_total += layer_config.sign_and_geometry_info.num_units;
            expected_remainder += layer_config.sign_and_geometry_info.num_sockets_per_unit;
            num_remaining_layer_iterations.push_back(layer_config.sign_and_geometry_info.num_sockets_per_unit);
        }
    }
    open_inputs.reserve(num_units_total + expected_remainder);
    open_outputs.reserve(num_units_total + expected_remainder);
    uid  other_id{ NUM_INPUT_LAYERS, 0U, 0U };
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
                    if (i >= NUM_INPUT_LAYERS)
                        open_inputs.push_back(id);
                    else
                    {
                        open_inputs.push_back(other_id);
                        switch (inputs_distrubution_strategy)
                        {
                        case INPUTS_DISTRUBUTION_STRATEGY::RANDOM:
                            other_id.layer = (natural_8_bit)get_random_natural_32_bit_in_range(NUM_INPUT_LAYERS, (natural_32_bit)layers.size() - 1U, random_generator);
                            other_id.unit = (natural_16_bit)get_random_natural_32_bit_in_range(0U, (natural_32_bit)layers.at(other_id.layer).units.size() - 1U, random_generator);
                            break;
                        case INPUTS_DISTRUBUTION_STRATEGY::LAYERS_FIRST:
                            ++other_id.layer;
                            if (other_id.layer == num_layers)
                            {
                                other_id.layer = NUM_INPUT_LAYERS;
                                for (++other_id.unit; other_id.unit >= (natural_16_bit)layers.at(other_id.layer).units.size(); ++other_id.layer)
                                    if (other_id.layer == num_layers)
                                    {
                                        other_id.layer = NUM_INPUT_LAYERS;
                                        other_id.unit = 0U;
                                        break;
                                    }
                            }
                            break;
                        case INPUTS_DISTRUBUTION_STRATEGY::UNITS_FIRST:
                            ++other_id.unit;
                            if (other_id.unit == layers.at(other_id.layer).units.size())
                            {
                                other_id.unit = 0U;
                                ++other_id.layer;
                                if (other_id.layer == num_layers)
                                    other_id.layer = NUM_INPUT_LAYERS;
                            }
                            break;
                        default:
                            UNREACHABLE();
                        }
                    }
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

    output_events.clear();
    for (natural_8_bit  i = NUM_INPUT_LAYERS; i < layers.size(); ++i)
    {
        bool const  is_output_layer = OUTPUT_LAYER_INDICES.count(i);
        network_layer&  layer = layers.at(i);
        for (natural_16_bit j = 0U; j < layer.units.size(); ++j)
        {
            computation_unit&  unit = layer.units.at(j);
            if (unit.event_potential >= layer.EVENT_TREASHOLD)
            {
                uid const  id{ i, j, 0U };
                next_events.push_back(id);
                if (is_output_layer)
                    output_events.push_back(id);
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
        natural_32_bit const  i_idx = (natural_32_bit)get_random_natural_32_bit_in_range(0U, (natural_32_bit)open_inputs.size() - 1U, random_generator);

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


void  network::insert_input_event(uid const  input_unit_id)
{
    ASSUMPTION(input_unit_id.layer < NUM_INPUT_LAYERS && input_unit_id.unit < (natural_8_bit)layers.at(input_unit_id.layer).units.size());
    events.push_back(input_unit_id);
}


}}
