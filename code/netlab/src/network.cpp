#include <netlab/network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


void  network::set_spiking_input_unit(uid const  input_unit_id)
{
    ASSUMPTION(input_unit_id.socket == 0U);

    ASSUMPTION(input_unit_id.layer < NUM_INPUT_LAYERS);
    network_layer&  layer = layers.at(input_unit_id.layer);

    ASSUMPTION(input_unit_id.unit < (natural_16_bit)layer.units.size());
    computation_unit&  unit = layer.units.at(input_unit_id.unit);

    unit.charge = layer.CHARGE_RECOVERY;

    spiking_units.push_back(input_unit_id);
}


void  network::next_round()
{
    TMPROF_BLOCK();

    stats.on_next_round();

    // NOTE: The order of called methods is highly important. Think twice before changing it!!

    update_input_sockets_of_spiking_units();
    update_output_sockets_of_spiking_units();
    propagate_charge_of_spikes();
    discharge_spiking_units();
    update_charge_of_units();
    connect_open_sockets();
}


void  network::update_input_sockets_of_spiking_units()
{
    TMPROF_BLOCK();

    // NOTE: If we have K processors/cores, then we could split spiking_units to K intervals and
    //       process each interval in a separate thread.
    for (uid  id : spiking_units)
    {
        network_layer&  layer = layers.at(id.layer);
        computation_unit&  unit = layer.units.at(id.unit);

        std::vector<input_socket>&  inputs = unit.inputs;
        for (natural_16_bit  i = 0U; i < (natural_16_bit)inputs.size(); )
        {
            input_socket&  input = inputs.at(i);

            uid const  other_id = input.other;
            network_layer&  other_layer = layers.at(other_id.layer);
            computation_unit&  other_unit = other_layer.units.at(other_id.unit);

            float_32_bit  weight_mult;
            {
                if (other_unit.charge < other_layer.WEIGHT_NEUTRAL_CHARGE)
                    weight_mult = (other_unit.charge - other_layer.WEIGHT_NEUTRAL_CHARGE) /
                                  (other_layer.WEIGHT_NEUTRAL_CHARGE - other_layer.CHARGE_RECOVERY);
                else
                    weight_mult = (other_unit.charge - other_layer.WEIGHT_NEUTRAL_CHARGE) /
                                  (other_layer.CHARGE_SPIKE - other_layer.WEIGHT_NEUTRAL_CHARGE);
                weight_mult *= other_layer.SPIKE_SIGN;
            }

            float_32_bit const  WEIGHT_DELTA_PER_SPIKE = 0.5f * (layer.WEIGHT_DELTA_PER_SPIKE + other_layer.WEIGHT_DELTA_PER_SPIKE);
            float_32_bit const  WEIGHT_MAXIMAL = 0.5f * (layer.WEIGHT_MAXIMAL + other_layer.WEIGHT_MAXIMAL);

            input.weight = std::min(input.weight + WEIGHT_DELTA_PER_SPIKE * weight_mult, WEIGHT_MAXIMAL);

            float_32_bit const  WEIGHT_DISCONNECTION = 0.5f * (layer.WEIGHT_DISCONNECTION + other_layer.WEIGHT_DISCONNECTION);

            if (input.weight <= WEIGHT_DISCONNECTION)
            {
                disconnect(inputs, uid::as_socket(id, i), other_unit.outputs, other_id);
                continue;
            }
    
            ++i;
        }
    }
}


void  network::update_output_sockets_of_spiking_units()
{
    TMPROF_BLOCK();

    // NOTE: If we have K processors/cores, then we could split spiking_units to K intervals and
    //       process each interval in a separate thread.
    for (uid  id : spiking_units)
    {
        network_layer&  layer = layers.at(id.layer);
        computation_unit&  unit = layer.units.at(id.unit);

        std::vector<output_socket>&  outputs = unit.outputs;
        for (natural_16_bit  i = 0U; i < (natural_16_bit)outputs.size(); )
        {
            output_socket&  output = outputs.at(i);

            uid const  other_id = output.other;
            network_layer&  other_layer = layers.at(other_id.layer);
            computation_unit&  other_unit = other_layer.units.at(other_id.unit);

            if (other_unit.charge == other_layer.CHARGE_SPIKE)
                continue; // This socket pair was already updated in update_input_sockets_of_spiking_units().

            std::vector<input_socket>&  other_inputs = other_unit.inputs;
            input_socket&  other_input = other_inputs.at(other_id.socket);

            float_32_bit  weight_mult;
            {
                if (other_unit.charge < other_layer.WEIGHT_NEUTRAL_CHARGE)
                    weight_mult = (other_unit.charge - other_layer.WEIGHT_NEUTRAL_CHARGE) /
                                  (other_layer.WEIGHT_NEUTRAL_CHARGE - other_layer.CHARGE_RECOVERY);
                else
                    weight_mult = (other_unit.charge - other_layer.WEIGHT_NEUTRAL_CHARGE) /
                                  (other_layer.CHARGE_SPIKE - other_layer.WEIGHT_NEUTRAL_CHARGE);
                weight_mult *= layer.SPIKE_SIGN;
            }

            float_32_bit const  WEIGHT_DELTA_PER_SPIKE = 0.5f * (layer.WEIGHT_DELTA_PER_SPIKE + other_layer.WEIGHT_DELTA_PER_SPIKE);
            float_32_bit const  WEIGHT_MAXIMAL = 0.5f * (layer.WEIGHT_MAXIMAL + other_layer.WEIGHT_MAXIMAL);

            other_input.weight = std::min(other_input.weight + WEIGHT_DELTA_PER_SPIKE * weight_mult, WEIGHT_MAXIMAL);

            float_32_bit const  WEIGHT_DISCONNECTION = 0.5f * (layer.WEIGHT_DISCONNECTION + other_layer.WEIGHT_DISCONNECTION);

            if (other_input.weight <= WEIGHT_DISCONNECTION)
            {
                disconnect(other_inputs, other_id, outputs, uid::as_socket(id, i));
                continue;
            }
    
            ++i;
        }
    }
}


void  network::propagate_charge_of_spikes()
{
    TMPROF_BLOCK();

    // NOTE: If we have K processors/cores, then we could split spiking_units to K intervals and
    //       process each interval in a separate thread.
    //       WARNING/TODO: Concurrent access to 'other_unit.charge' - synchronisation required!
    for (uid  id : spiking_units)
    {
        network_layer&  layer = layers.at(id.layer);
        computation_unit&  unit = layer.units.at(id.unit);

        std::vector<output_socket>&  outputs = unit.outputs;
        for (natural_16_bit  i = 0U; i < (natural_16_bit)outputs.size(); ++i)
        {
            output_socket&  output = outputs.at(i);

            uid const  other_id = output.other;
            network_layer&  other_layer = layers.at(other_id.layer);
            computation_unit&  other_unit = other_layer.units.at(other_id.unit);

            stats.on_spike_received(id);

            // WARNING: Concurrent access to 'other_unit.charge' - synchronisation required!
            other_unit.charge += other_layer.INPUT_SPIKE_MAGNITUDE;
        }
    }
}


void  network::discharge_spiking_units()
{
    TMPROF_BLOCK();

    // NOTE: If we have K processors/cores, then we could split spiking_units to K intervals and
    //       process each interval in a separate thread.
    for (uid  id : spiking_units)
    {
        network_layer&  layer = layers.at(id.layer);
        computation_unit&  unit = layer.units.at(id.unit);
        unit.charge = id.layer < NUM_INPUT_LAYERS ? layer.CHARGE_RECOVERY : layer.CHARGE_RECOVERY / layer.CHARGE_DECAY_COEF;
    }
}


void  network::update_charge_of_units()
{
    TMPROF_BLOCK();

    spiking_units.clear();
    spiking_output_units.clear();
    for (natural_8_bit  i = NUM_INPUT_LAYERS, n = (natural_8_bit)layers.size(); i != n; ++i)
    {
        // NOTE: Code in this block can run in a thread.

        network_layer&  layer = layers.at(i);
        for (natural_16_bit j = 0U, m = (natural_16_bit)layer.units.size(); j != m; ++j)
        {
            computation_unit&  unit = layer.units.at(j);
            unit.charge = std::min(unit.charge * layer.CHARGE_DECAY_COEF, layer.CHARGE_SPIKE);
            if (unit.charge == layer.CHARGE_SPIKE)
            {
                uid const  id{ i, j, 0U };

                stats.on_spike_produced(id);

                std::lock_guard<std::mutex> lock(spiking_mutex);
                spiking_units.push_back(id);
                if (i >= n - NUM_OUTPUT_LAYERS)
                    spiking_output_units.push_back(id);
            }
        }
    }
}


void  network::connect_open_sockets()
{
    TMPROF_BLOCK();

    INVARIANT(open_inputs.size() == open_outputs.size());

    for (natural_32_bit  o_idx = 0U; o_idx < (natural_32_bit)open_outputs.size(); )
    {
        natural_32_bit const  i_idx =
                get_random_natural_32_bit_in_range(0U, (natural_32_bit)open_inputs.size() - 1U, random_generator);

        if (!connect(i_idx, o_idx))
            ++o_idx;
    }
}


bool  network::connect(natural_32_bit const  input_idx, natural_32_bit const  output_idx)
{
    TMPROF_BLOCK();

    uid&  iid = open_inputs.at(input_idx);
    uid&  oid = open_outputs.at(output_idx);
    if (iid.layer == oid.layer && iid.unit == oid.unit)
        return false;

    network_layer&  input_layer = layers.at(iid.layer);
    std::vector<input_socket>&  inputs = input_layer.units.at(iid.unit).inputs;
    network_layer&  output_layer = layers.at(oid.layer);
    std::vector<output_socket>&  outputs = output_layer.units.at(oid.unit).outputs;

    iid.socket = inputs.size();
    oid.socket = outputs.size();

    inputs.push_back({ oid, 0.5f * (input_layer.WEIGHT_CONNECTION + output_layer.WEIGHT_CONNECTION)});
    outputs.push_back({ iid });

    iid = open_inputs.back();
    open_inputs.pop_back();

    oid = open_outputs.back();
    open_outputs.pop_back();

    stats.on_connect(iid, oid);

    return true;
}


void  network::disconnect(
        std::vector<input_socket>&  inputs,
        uid const  input_socket_id,
        std::vector<output_socket>&  outputs,
        uid const  output_socket_id
        )
{
    TMPROF_BLOCK();

    stats.on_disconnect(inputs, input_socket_id, outputs, output_socket_id);

    uid const  oid = inputs.back().other;
    layers.at(oid.layer).units.at(oid.unit).outputs.at(oid.socket).other.socket = input_socket_id.socket;

    uid const  iid = outputs.back().other;
    layers.at(iid.layer).units.at(iid.unit).inputs.at(iid.socket).other.socket = output_socket_id.socket;

    inputs.at(input_socket_id.socket) = inputs.back();
    inputs.pop_back();

    outputs.at(output_socket_id.socket) = outputs.back();
    outputs.pop_back();

    std::lock_guard<std::mutex> lock(open_io_mutex);
    open_inputs.push_back(uid::as_unit(input_socket_id));
    open_outputs.push_back(uid::as_unit(output_socket_id));
}


}
