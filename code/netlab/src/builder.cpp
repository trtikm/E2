#include <netlab/builder.hpp>
#include <netlab/network.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>

namespace netlab {


builder::builder(network* const  net_)
    : net(net_)
    , layers()
{
    ASSUMPTION(net != nullptr);
}


builder&  builder::insert_layer_info(layer_info const&  info)
{
    ASSUMPTION(info.num_units > 0U && info.num_sockets_per_unit > 0U && info.layer.units.empty());
    network_layer const&  layer = info.layer;
    ASSUMPTION(layer.SPIKE_SIGN == 1.0f || layer.SPIKE_SIGN == -1.0f);
    ASSUMPTION(layer.CHARGE_SPIKE > 0.0f);
    ASSUMPTION(layer.CHARGE_RECOVERY >= 0.0f && layer.CHARGE_RECOVERY < layer.CHARGE_SPIKE);
    ASSUMPTION(layer.CHARGE_DECAY_COEF > 0.0f && layer.CHARGE_DECAY_COEF < 1.0f);
    ASSUMPTION(layer.WEIGHT_NEUTRAL_CHARGE > layer.CHARGE_RECOVERY && layer.WEIGHT_NEUTRAL_CHARGE < layer.CHARGE_SPIKE);
    ASSUMPTION(layer.WEIGHT_DELTA_PER_SPIKE > 0.0f);
    ASSUMPTION(layer.WEIGHT_CONNECTION > 0.0f);
    ASSUMPTION(layer.WEIGHT_DISCONNECTION >= 0.0f && layer.WEIGHT_DISCONNECTION < layer.WEIGHT_CONNECTION);
    ASSUMPTION(layer.WEIGHT_MAXIMAL > layer.WEIGHT_CONNECTION);
    ASSUMPTION(layer.INPUT_SPIKE_MAGNITUDE > 0.0f);
    layers.push_back(info);
    return *this;
}


void  builder::run()
{
    if (layers.empty())
        setup_minimal_net();

    net->layers.resize(layers.size());
    net->open_inputs.clear();
    net->open_outputs.clear();
    net->spiking_units.clear();
    net->spiking_output_units.clear();
    for (natural_8_bit  i = 0U; i != (natural_8_bit)layers.size(); ++i)
    {
        layer_info const&  info = layers.at(i);
        network_layer& nl = net->layers.at(i);
        nl = info.layer;
        nl.units.resize(info.num_units);
        for (natural_16_bit  j = 0U; j != info.num_units; ++j)
        {
            computation_unit&  cu = nl.units.at(j);
            cu.charge = nl.CHARGE_RECOVERY;
            cu.inputs.clear();
            cu.outputs.clear();
            uid const  unit_id{ i, j, 0U };
            for (natural_16_bit  k = 0U; k != info.num_sockets_per_unit; ++k)
            {
                if (i >= net->NUM_INPUT_LAYERS)
                    net->open_inputs.push_back(unit_id);
                if (i < (natural_8_bit)layers.size() - net->NUM_INPUT_LAYERS)
                    net->open_outputs.push_back(unit_id);
            }
        }
    }
    net->connect_open_sockets();
}


void  builder::setup_minimal_net()
{
    network_layer const  layer_template {
            1.0f, // SPIKE_SIGN
            1.0f, // CHARGE_SPIKE
            0.0f, // CHARGE_RECOVERY
            0.1f, // CHARGE_DECAY_COEF
            0.5f, // WEIGHT_NEUTRAL_CHARGE
            0.1f, // WEIGHT_DELTA_PER_SPIKE
            0.5f, // WEIGHT_CONNECTION
            0.0f, // WEIGHT_DISCONNECTION
            1.0f, // WEIGHT_MAXIMAL
            1.0f, // INPUT_SPIKE_MAGNITUDE
            { // units
                computation_unit {
                    0.0f,   // charge
                    {},     // inputs
                    {}      // outputs
                    }
                }
            };
    layer_info const  info_template {
            1U, // num_units
            0U, // num_sockets_per_unit
            layer_template // layer
            };
    layers.resize((natural_16_bit)net->NUM_INPUT_LAYERS + (natural_16_bit)net->NUM_OUTPUT_LAYERS, info_template);
    natural_8_bit const  total_sockets = std::max(net->NUM_INPUT_LAYERS, net->NUM_OUTPUT_LAYERS);
    natural_8_bit  num_sockets;
    natural_8_bit  layer_idx;
    if (net->NUM_INPUT_LAYERS > 0U)
        for (num_sockets = 0U, layer_idx = 0U; num_sockets < total_sockets; ++num_sockets)
        {
            layer_info&  info = layers.at(layer_idx);
            ++info.num_sockets_per_unit;
            layer_idx = (layer_idx + 1U) % net->NUM_INPUT_LAYERS;
        }
    if (net->NUM_OUTPUT_LAYERS > 0U)
        for (num_sockets = 0U, layer_idx = 0U; num_sockets < total_sockets; ++num_sockets)
        {
            layer_info&  info = layers.at((natural_8_bit)layers.size() - net->NUM_OUTPUT_LAYERS + layer_idx);
            ++info.num_sockets_per_unit;
            layer_idx = (layer_idx + 1U) % net->NUM_OUTPUT_LAYERS;
        }
}


}
