#include <aiold/cortex_robot.hpp>
//#include <aiold/action_controller.hpp>
//#include <aiold/skeletal_motion_templates.hpp>
//#include <aiold/motion_desire_props.hpp>
#include <aiold/env/snapshot.hpp>
//#include <aiold/detail/guarded_motion_actions_processor.hpp>
//#include <aiold/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace sinet = netlab::simple;

namespace aiold {



cortex_robot::cortex_robot(blackboard_agent_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , snapshot_encoder(nullptr)
    , motion_desire_decoder(nullptr)
    , network(nullptr)
    , time_buffer_in_seconds(0.0f)
{}


cortex_robot::~cortex_robot()
{}


void  cortex_robot::initialise()
{
    cortex::initialise();

    snapshot_encoder = std::make_unique<env::sinet::snapshot_encoder>(get_blackboard(), 0U);

    std::vector<netlab::simple::network_layer::config::sign_and_geometry>  input_layers_configs;
    snapshot_encoder->get_layers_configs(input_layers_configs);

    motion_desire_decoder =
            std::make_unique<env::sinet::motion_desire_decoder>(m_motion_desire_props, (natural_8_bit)input_layers_configs.size());

    network = std::make_unique<netlab::simple::network>(
                sinet::network::prefab::UNIVERSAL,
                input_layers_configs,
                sinet::network::INPUTS_DISTRUBUTION_STRATEGY::LAYERS_FIRST,
                std::vector<natural_8_bit>{ (natural_8_bit)input_layers_configs.size() },   // output_layer_indices
                sinet::network::statistics::config{
                        500U,   // num_rounds_per_snapshot
                        1U,     // snapshots_history_size
                        0.1f    // ratio_of_probed_units_per_layer
                        },
                0U              // seed
                );
}


void  cortex_robot::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    //env::snapshot_const_ptr  snapshot_ptr;
    //std::unordered_set<netlab::simple::uid>  spiking_units;
    //for (time_buffer_in_seconds += time_step_in_seconds;
    //     time_buffer_in_seconds >= NETWORK_TIME_STEP_DURATION_IN_SECONDS; 
    //     time_buffer_in_seconds -= NETWORK_TIME_STEP_DURATION_IN_SECONDS)
    //{
    //    if (snapshot_ptr == nullptr)
    //        snapshot_ptr = env::create_snapshot(get_blackboard());
    //    spiking_units.clear();
    //    snapshot_encoder->next_round(NETWORK_TIME_STEP_DURATION_IN_SECONDS, *snapshot_ptr, spiking_units);
    //    for (netlab::simple::uid  uid : spiking_units)
    //        network->insert_input_event(uid);
    //    network->next_round();
    //    motion_desire_decoder->next_round(NETWORK_TIME_STEP_DURATION_IN_SECONDS, network->get_output_events());
    //}

    cortex::next_round(time_step_in_seconds); // REMOVE THIS LINE, ONCE THE PROPER IMPLEMENTATION ABOVE IS COMPLETE!!!
}


}
