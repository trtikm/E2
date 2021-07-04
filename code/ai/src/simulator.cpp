#include <ai/simulator.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/timeprof.hpp>

namespace ai {


simulator::simulator()
    : m_agents()
    , m_navsystem(nullptr)
    , m_naveditor(nullptr)
{}


agent_id  simulator::insert_agent(
        agent_config const  config,
        skeletal_motion_templates const  motion_templates,
        scene_binding_ptr const  binding
        )
{
    ensure_navsystem_exists(binding->context);
    return m_agents.insert(std::make_shared<agent>(config, motion_templates, m_navsystem, binding));
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


void  simulator::clear(bool const  also_navsystem)
{
    m_agents.clear();
    if (also_navsystem)
    {
        m_naveditor = nullptr;
        m_navsystem = nullptr;
    }
}


void  simulator::next_round(float_32_bit const  time_step_in_seconds, cortex::mock_input_props const* const  mock_input_ptr)
{
    TMPROF_BLOCK();

    if (m_naveditor != nullptr)
        m_naveditor->next_round(time_step_in_seconds);
    for (agent_id  id : m_agents.valid_indices())
        m_agents.at(id)->next_round(time_step_in_seconds, mock_input_ptr);
}


void  simulator::ensure_navsystem_exists(simulation_context_const_ptr const  context_)
{
    if (m_navsystem == nullptr)
    {
        ASSUMPTION(context_ != nullptr);
        m_navsystem = std::make_shared<navsystem>(context_);
        m_naveditor = std::make_shared<naveditor>(m_navsystem);
    }
}


}
