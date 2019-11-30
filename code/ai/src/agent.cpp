#include <ai/agent.hpp>
#include <ai/cortex.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_robot_humanoid.hpp>
#include <ai/sensory_controller.hpp>
#include <ai/sensory_controller_collision_contacts.hpp>
#include <ai/sensory_controller_ray_cast_sight.hpp>
#include <ai/action_controller.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


blackboard_ptr  agent::create_blackboard(AGENT_KIND const  agent_kind)
{
    TMPROF_BLOCK();

    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
    case AGENT_KIND::ROBOT_HUMANOID:
        return std::make_shared<blackboard>();
    default:
        UNREACHABLE();
    }
}


void  agent::create_modules(blackboard_ptr const  bb, input_devices_ptr const  idev)
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
    case AGENT_KIND::ROBOT_HUMANOID:
        bb->m_cortex = std::make_shared<cortex_robot_humanoid>(bb);
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


agent::agent(blackboard_ptr const  blackboard_)
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


}
