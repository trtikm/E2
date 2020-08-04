#include <aiold/agent.hpp>
#include <aiold/cortex.hpp>
#include <aiold/cortex_mock.hpp>
#include <aiold/cortex_random.hpp>
#include <aiold/cortex_robot.hpp>
#include <aiold/sensory_controller.hpp>
#include <aiold/sensory_controller_collision_contacts.hpp>
#include <aiold/sensory_controller_ray_cast_sight.hpp>
#include <aiold/action_controller.hpp>
#include <aiold/simulator.hpp>
#include <aiold/sensor_action_default_processor.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace aiold {


blackboard_agent_ptr  agent::create_blackboard(AGENT_KIND const  agent_kind)
{
    TMPROF_BLOCK();

    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
    case AGENT_KIND::STATIONARY:
    case AGENT_KIND::RANDOM:
    case AGENT_KIND::ROBOT:
        return std::make_shared<blackboard_agent>();
    default:
        UNREACHABLE();
    }
}


void  agent::create_modules(blackboard_agent_ptr const  bb, input_devices_ptr const  idev)
{
    TMPROF_BLOCK();

    switch (bb->m_agent_kind)
    {
    case AGENT_KIND::MOCK:
        bb->m_cortex = std::make_shared<cortex_mock>(bb, idev);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                std::make_shared<sensory_controller_sight>(
                        bb,
                        sensory_controller_sight::camera_config()
                        )
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::STATIONARY:
        bb->m_cortex = std::make_shared<cortex>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                nullptr
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::RANDOM:
        bb->m_cortex = std::make_shared<cortex_random>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                std::make_shared<sensory_controller_sight>(
                        bb,
                        sensory_controller_sight::camera_config()
                        )
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    case AGENT_KIND::ROBOT:
        bb->m_cortex = std::make_shared<cortex_robot>(bb);
        bb->m_sensory_controller = std::make_shared<sensory_controller>(
                bb,
                std::make_shared<sensory_controller_collision_contacts>(
                        bb,
                        sensory_controller_collision_contacts::config()
                        ),
                std::make_shared<sensory_controller_ray_cast_sight>(
                        bb,
                        sensory_controller_sight::camera_config(),
                        sensory_controller_ray_cast_sight::ray_cast_config()
                        )
                );
        bb->m_action_controller = std::make_shared<action_controller>(bb);
        break;
    default:
        UNREACHABLE();
    }
}


}

namespace aiold {


agent::agent(blackboard_agent_ptr const  blackboard_)
    : m_blackboard(blackboard_)
{
    TMPROF_BLOCK();

    // The initialisation order of agent's modules is important and mandatory.
    get_blackboard()->m_sensory_controller->initialise();
    get_blackboard()->m_action_controller->initialise();
    get_blackboard()->m_cortex->initialise();
}


agent::~agent()
{
    TMPROF_BLOCK();

    // The release order of agent's modules is important and mandatory.
    get_blackboard()->m_cortex = nullptr;
    get_blackboard()->m_action_controller = nullptr;
    get_blackboard()->m_sensory_controller = nullptr;
}


void  agent::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    // We need to synchronise motion object, called roller (and located in the action controller),
    // with the scene as it can be used by any controllers in their next_round functions.
    get_blackboard()->m_action_controller->synchronise_with_scene();

    // The update order of agent's modules is important and mandatory.
    get_blackboard()->m_sensory_controller->next_round(time_step_in_seconds);
    get_blackboard()->m_cortex->next_round(time_step_in_seconds);
    get_blackboard()->m_action_controller->next_round(time_step_in_seconds);
}


void  agent::on_sensor_event(sensor const&  s, sensor::other_object_info const&  other)
{
    auto const  actions_it = get_blackboard()->m_sensor_actions->find(
            s.get_self_rid().get_relative_to(get_blackboard()->m_self_rid.get_node_id())
            );

    // Each sensor must have an action attached to it, otherwise it is completely useless.
    ASSUMPTION(actions_it != get_blackboard()->m_sensor_actions->end());

    for (sensor_action&  action : actions_it->second)
    {
        // Here put processing of agent-specific actions

        // Here we process actions the default way (i.e. not specific to an agent)
        if (process_sensor_event_using_default_procedure(
                get_blackboard()->m_self_rid,
                action,
                other,
                get_blackboard()->m_simulator_ptr,
                get_blackboard()->m_scene
                ))
            continue;

        UNREACHABLE(); // If we get here, then we forgot to implement processing of the current action. FIX THAT!
    }
}


}