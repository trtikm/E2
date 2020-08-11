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


static sight_controller::ray_cast_config  make_ray_cast_config(AGENT_KIND const  agent_kind)
{
    switch (agent_kind)
    {
    case AGENT_KIND::MOCK:
    case AGENT_KIND::ROBOT:
        return sight_controller::ray_cast_config{
                600U,       // num_raycasts_per_second
                0.1f,       // max_ray_cast_info_life_time_in_seconds
                128U,       // num_cells_along_x_axis; must be a power of 2.
                64U         // num_cells_along_y_axis; must be a power of 2.
                };
    case AGENT_KIND::STATIONARY:
    case AGENT_KIND::RANDOM:
        return sight_controller::ray_cast_config{0U};
    default: { UNREACHABLE(); return sight_controller::ray_cast_config(); }
    }
}


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
            make_ray_cast_config(agent_kind),
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

    m_sight_controller.next_round(time_step_in_seconds, m_action_controller.get_motion_object_collider_guids());
    m_cortex->next_round(time_step_in_seconds, keyboard, mouse, window);
    m_action_controller.next_round(time_step_in_seconds, m_cortex->get_motion_desire_props());
}


}
