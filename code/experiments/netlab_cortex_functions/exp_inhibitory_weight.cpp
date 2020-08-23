#include <netlab_cortex_functions/exp_inhibitory_weight.hpp>
#include <angeo/tensor_math.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>


void  exp_inhibitory_weight::network_setup()
{
    src_layer_idx = cortex.add_layer(1U, 1U, 0U, false);
    dst_layer_idx = cortex.add_layer(1U, 0U, 1U, true);
    cortex.set_constant_connection_probability(src_layer_idx, src_layer_idx, 0.0f);
    cortex.set_constant_connection_probability(src_layer_idx, dst_layer_idx, 1.0f);
    cortex.set_constant_connection_probability(dst_layer_idx, dst_layer_idx, 0.0f);
    //cortex.set_constant_synapse_weight_disconnection(src_layer_idx, dst_layer_idx, -10000.0f);
    //cortex.set_constant_synapse_weight_delta_per_second(src_layer_idx, dst_layer_idx, 0.25f);
    //cortex.set_constant_synapse_weight_decay_delta_per_second(src_layer_idx, dst_layer_idx, 0.01f);
    cortex.build_new_synapses();
}


void  exp_inhibitory_weight::network_update()
{
    cortex.update_existing_synapses(round_seconds());

    if (get_keyboard_props().keys_pressed().count(osi::KEY_UP()) != 0UL)
        cortex.layer_ref(dst_layer_idx).neurons.at(0U).excitation += 0.5f * round_seconds();
    if (get_keyboard_props().keys_pressed().count(osi::KEY_DOWN()) != 0UL)
        cortex.layer_ref(dst_layer_idx).neurons.at(0U).excitation -= 0.5f * round_seconds();
    if (get_keyboard_props().keys_pressed().count(osi::KEY_J()) != 0UL)
        cortex.layer_ref(dst_layer_idx).neurons.at(0U).excitation =
            get_random_float_32_bit_in_range(-0.5f, 0.5f, default_random_generator());
    if (get_keyboard_props().keys_pressed().count(osi::KEY_R()) != 0UL)
        cortex.synapse_ref(0U).weight = 0.5f;
}


void  exp_inhibitory_weight::scene_setup()
{
    com::simulation_context&  ctx = *context();

    com::object_guid  folder_guid = ctx.insert_folder(ctx.root_folder(), "weight_folder");
    weight_frame_guid = ctx.insert_frame(folder_guid, com::invalid_object_guid(), vector3_unit_x(), quaternion_identity());
    ctx.insert_batch_solid_sphere(folder_guid, "weight_batch", 0.1f, 5U, vector4{1.0f, 0.0f, 0.8f, 1.0f});

    folder_guid = ctx.insert_folder(ctx.root_folder(), "membrane_potential_folder");
    membrane_potential_frame_guid =
            ctx.insert_frame(folder_guid, com::invalid_object_guid(), -vector3_unit_x(), quaternion_identity());
    ctx.insert_batch_solid_sphere(folder_guid, "membrane_potential_batch", 0.1f, 5U, vector4{0.0f, 1.0f, 0.8f, 1.0f});
}


void  exp_inhibitory_weight::scene_update()
{
    com::simulation_context&  ctx = *context();

    ctx.request_relocate_frame(
            weight_frame_guid,
            {1.0f, 10.0f * cortex.get_synapse(0U).weight, 0.0f },
            quaternion_identity());
    ctx.request_relocate_frame(
            membrane_potential_frame_guid,
            {-1.0f, 10.0f * cortex.neuron_excitation({dst_layer_idx, 0U}), 0.0f },
            quaternion_identity());
}


void  exp_inhibitory_weight::help()
{
    SLOG(
        "\tEvaluation of synaptic weight from an inhibitory neuron.\n"
        "\tRed sphere - synaptic weight\n"
        "\tGreen sphere - excitation level of post-neuron\n"
        "\tUP/DOWN - increase/decrease excitation level of post-neuron\n"
        "\tJ - set excitation level of post-neuron randomly in <-0.5, 0.5>\n"
        "\tR - reset synaptic weight to 0.5\n"
        );
}
