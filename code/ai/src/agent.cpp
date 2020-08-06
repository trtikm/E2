#include <ai/agent.hpp>
#include <ai/cortex_mock.hpp>
#include <ai/cortex_random.hpp>
#include <ai/cortex_robot.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>

namespace ai {


agent::agent(
        AGENT_KIND const  agent_kind,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
    : m_agent_kind(agent_kind)
    , m_motion_templates(motion_templates)
    , m_binding(binding)
    , m_action_controller(motion_templates, binding)
    , m_sight_controller(
            sight_controller::camera_config(),
            sight_controller::ray_cast_config(),
            m_motion_templates,
            m_binding
            )
    , m_cortex()
{
    switch (m_agent_kind)
    {
    case AGENT_KIND::MOCK:
        m_cortex = std::make_shared<cortex_mock>();
        break;
    case AGENT_KIND::STATIONARY:
        m_cortex = std::make_shared<cortex>();
        break;
    case AGENT_KIND::RANDOM:
        m_cortex = std::make_shared<cortex_random>();
        break;
    case AGENT_KIND::ROBOT:
        m_cortex = std::make_shared<cortex_robot>();
        break;
    default: { UNREACHABLE(); break; }
    }
}


void  agent::next_round(
        float_32_bit const  time_step_in_seconds,
        osi::keyboard_props const&  keyboard,
        osi::mouse_props const&  mouse,
        osi::window_props const&  window
        )
{
    TMPROF_BLOCK();

    m_sight_controller.next_round(time_step_in_seconds);
    m_cortex->next_round(time_step_in_seconds, keyboard, mouse, window);
    m_action_controller.next_round(time_step_in_seconds, m_cortex->get_motion_desire_props());
}


}
