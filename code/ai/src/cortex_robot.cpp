#include <ai/cortex_robot.hpp>
//#include <ai/action_controller.hpp>
//#include <ai/skeletal_motion_templates.hpp>
//#include <ai/motion_desire_props.hpp>
#include <ai/env/snapshot.hpp>
//#include <ai/detail/guarded_motion_actions_processor.hpp>
//#include <ai/detail/ideal_velocity_buider.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace sinet = netlab::simple;

namespace ai {



cortex_robot::cortex_robot(blackboard_weak_ptr const  blackboard_)
    : cortex(blackboard_)
    , network(
        sinet::network::prefab::UNIVERSAL,        
        { // input_layers_configs
            sinet::network_layer::config::sign_and_geometry {
                true,       // is_excitatory
                100U,       // num_units
                100U        // num_sockets_per_unit
                }
            },
        sinet::network::INPUTS_DISTRUBUTION_STRATEGY::LAYERS_FIRST,
        { // output_layer_indices
            1U
            },
        sinet::network::statistics::config { // stats_config
            500U
            },
        0U // seed
        )
    , time_buffer_in_seconds(0.0f)
{}


cortex_robot::~cortex_robot()
{}


void  cortex_robot::initialise()
{
    cortex::initialise();
}


void  cortex_robot::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    cortex::next_round(time_step_in_seconds);

    //env::snapshot_const_ptr  snapshot_ptr;
    //for (time_buffer_in_seconds += time_step_in_seconds;
    //     time_buffer_in_seconds >= NETWORK_TIMES_STEP_DURATION_IN_SECONDS; 
    //     time_buffer_in_seconds -= NETWORK_TIMES_STEP_DURATION_IN_SECONDS)
    //{
    //    if (snapshot_ptr == nullptr)
    //        snapshot_ptr = env::create_snapshot(get_blackboard());

    //    network.next_round();
    //}
    //m_motion_desire_props.forward_unit_vector_in_local_space = ;
    //m_motion_desire_props.linear_velocity_unit_direction_in_local_space = ;
    //m_motion_desire_props.linear_speed = ;
    //m_motion_desire_props.angular_velocity_unit_axis_in_local_space = ;
    //m_motion_desire_props.angular_speed = ;
    //m_motion_desire_props.look_at_target_in_local_space = ;
}


}
