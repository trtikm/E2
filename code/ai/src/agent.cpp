#include <ai/agent.hpp>
#include <ai/cortex.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_robot.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_collision_contacts.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <ai/simulator.hpp>
#include <ai/sensor_event_utils.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


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
                nullptr
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

namespace ai {


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

    // We need to synchronise motion object (located in the action controller) with the scene
    // as it can be used by any controllers in their next_round functions.
    get_blackboard()->m_action_controller->synchronise_motion_object_motion_with_scene();

    // The update order of agent's modules is important and mandatory.
    get_blackboard()->m_sensory_controller->next_round(time_step_in_seconds);
    get_blackboard()->m_cortex->next_round(time_step_in_seconds);
    get_blackboard()->m_action_controller->next_round(time_step_in_seconds);
}


void  agent::on_sensor_event(sensor const&  s)
{
    auto const  actions_it = get_blackboard()->m_sensor_actions->find(s.get_self_rid());
    ASSUMPTION(actions_it != get_blackboard()->m_sensor_actions->end());
    for (sensor_action&  action : actions_it->second)
        switch (action.kind)
        {
        case SENSOR_ACTION_KIND::BEGIN_OF_LIFE:
            get_blackboard()->m_scene->accept(create_request_merge_scene(action.props));
            break;
        case SENSOR_ACTION_KIND::END_OF_LIFE:
            get_blackboard()->m_scene->accept(create_request_erase_nodes_tree(get_blackboard()->m_self_nid));
            break;
        default: UNREACHABLE(); break;
        }
}


}
