#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace ai {


simulator::simulator()
    : m_agents()
{}


agent_id  simulator::insert_agent(
        agent_config const  config,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
{
    return m_agents.insert(std::make_shared<agent>(config, motion_templates, binding));
}


void  simulator::erase_agent(agent_id const  id)
{
    m_agents.erase(id);
}


agent const&  simulator::get_agent(agent_id const  id) const
{
    ASSUMPTION(m_agents.valid(id) && m_agents.at(id) != nullptr);
    return *m_agents.at(id);
}


void  simulator::clear()
{
    m_agents.clear();
}


void  simulator::next_round(
        float_32_bit const  time_step_in_seconds,
        osi::keyboard_props const&  keyboard,
        osi::mouse_props const&  mouse,
        osi::window_props const&  window
        )
{
    TMPROF_BLOCK();

    for (agent_id  id : m_agents.valid_indices())
        m_agents.at(id)->next_round(time_step_in_seconds, keyboard, mouse, window);
}


}
