#include <netlab/simple_network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <utility/development.hpp>
#include <array>
#include <algorithm>
#include <limits>
#include <iterator>

namespace netlab { namespace simple { namespace detail {




}}}

namespace netlab { namespace simple {


void  network::next_round()
{
    TMPROF_BLOCK();

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
                next_events.push_back({ i, j, 0 });
                unit.event_potential = layer.EVENT_RECOVERY_POTENTIAL;
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
                open_inputs.push_back(id);
                open_outputs.push_back(other_id);
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
                open_inputs.push_back(other_id);
                open_outputs.push_back(id);
            }
            else
            {
                other_unit.event_potential += EVENT_POTENTIAL_MAGNITUDE * layer.EVENT_POTENTIAL_SIGN * other_input.weight;

                float_32_bit const  other_excitation_level = (other_unit.event_potential - other_layer.EVENT_RECOVERY_POTENTIAL) /
                                                             (other_layer.EVENT_TREASHOLD - other_layer.EVENT_RECOVERY_POTENTIAL);
                other_input.weight += other_layer.WEIGHT_PER_POTENTIAL * (other_excitation_level - other_layer.WEIGHT_EQUILIBRIUM_TREASHOLD);
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

    return true;
}


void  network::disconnect(
        std::vector<input_socket>&  inputs,
        natural_16_bit const  input_socket,
        std::vector<output_socket>&  outputs,
        natural_16_bit const  output_socket
        )
{
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
