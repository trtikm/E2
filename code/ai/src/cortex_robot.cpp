#include <ai/cortex_robot.hpp>
#include <ai/agent.hpp>
#include <utility/assumptions.hpp>
#include <utility/invariants.hpp>
#include <utility/development.hpp>
#include <utility/timeprof.hpp>
#include <utility/log.hpp>
#include <limits>

namespace ai {


constexpr inline natural_32_bit  invalid_action_node_index() { return std::numeric_limits<natural_32_bit>::max(); }


cortex_robot::cortex_robot(agent const*  myself_, bool const  use_mock_)
    : cortex_mock_optional(myself_, use_mock_)
    , m_states()
    , m_parents()
    , m_children()
    , m_penalties()
    , m_leaves()
    , m_queue()
{}


void  cortex_robot::next_round(float_32_bit const  time_step_in_seconds)
{
    TMPROF_BLOCK();

    cortex::next_round(time_step_in_seconds);

    insert_initial_node(time_step_in_seconds);

    while (m_states.size() < 10UL && !m_queue.empty())
    {
        natural_32_bit const  node_index = m_queue.front();
        m_queue.pop_front();

        insert_child_nodes(node_index);
    }

    motion_desire_props_ref() = m_states.at(find_best_node_index()).desire;

    clear();
}


void  cortex_robot::insert_initial_node(float_32_bit const  time_step_in_seconds)
{
    insert_node(
            {
                get_motion_desire_props(),
                myself().get_action_controller().get_current_action(),
                myself().get_system_state(),
                myself().get_system_variables(),
                myself().get_state_variables(),
                0.0f,
                time_step_in_seconds
            },
            invalid_action_node_index()
            );
}


void  cortex_robot::insert_child_nodes(natural_32_bit const  node_index)
{
    TMPROF_BLOCK();

    std::unordered_set<std::string>  action_names;
    action_names.insert(m_states.at(node_index).action->get_name());
    for (auto const&  name_and_ignred : m_states.at(node_index).action->get_transitions())
        action_names.insert(name_and_ignred.first);

    for (auto const&  name : action_names)
    {
        state_data  state = m_states.at(node_index);
        state.action = myself().get_action_controller().get_available_actions().at(name);
        state.desire = state.action->get_desire_config().ideal;
        state.action->update_system_state(state.system_state, state.desire, state.time_step_in_seconds);
        state.action->update_system_variables(
                state.system_variables,
                state.system_state,
                state.desire,
                state.time_step_in_seconds
                );
        state.action->update_state_variables(
                state.state_variables,
                state.system_variables,
                state.desire,
                state.time_step_in_seconds
                );
        state.seconds_consumed += state.time_step_in_seconds;
        state.time_step_in_seconds = 0.25f;

        insert_node(state, node_index);
    }
}


float_32_bit  cortex_robot::compute_penalty(state_data const&  state) const
{
    TMPROF_BLOCK();

    float_32_bit  penalty = 0.0f;
    for (auto const&  name_and_data : state.state_variables)
    {
        float_32_bit  delta = name_and_data.second.get_value() - name_and_data.second.get_ideal_value();
        if (delta < -0.0001f)
            delta /= name_and_data.second.get_ideal_value() - name_and_data.second.get_min_value();
        else if (delta > 0.0001f)
            delta /= name_and_data.second.get_max_value() - name_and_data.second.get_ideal_value();
        penalty += delta * delta;
    }
    return penalty;
}


natural_32_bit  cortex_robot::find_best_node_index() const
{
    TMPROF_BLOCK();

    auto  best_it = m_leaves.cbegin();
    for (auto  it = std::next(m_leaves.cbegin()); it != m_leaves.cend(); ++it)
        if (m_penalties.at(*it) < m_penalties.at(*best_it))
            best_it = it;
    natural_32_bit  best_index = *best_it;
    while (m_parents.at(best_index) != 0U)
        best_index = m_parents.at(best_index);
    return best_index;
}


void  cortex_robot::insert_node(state_data const&  state, natural_32_bit const  parent_node_index)
{
    TMPROF_BLOCK();

    m_states.push_back(state);
    natural_32_bit const  node_index = (natural_32_bit)m_states.size() - 1U;
    m_parents.push_back(parent_node_index);
    m_children.push_back({});
    if (parent_node_index != invalid_action_node_index())
        m_children.at(parent_node_index).push_back(node_index);
    m_penalties.push_back(compute_penalty(m_states.back()));
    m_leaves.insert(node_index);
    m_leaves.erase(parent_node_index);
    m_queue.push_back(node_index);
}


void  cortex_robot::clear()
{
    m_states.clear();
    m_parents.clear();
    m_children.clear();
    m_penalties.clear();
    m_leaves.clear();
    m_queue.clear();
}


}
