#ifndef AI_CORTEX_ROBOT_HPP_INCLUDED
#   define AI_CORTEX_ROBOT_HPP_INCLUDED

#   include <ai/cortex_mock.hpp>
#   include <ai/agent_system_state.hpp>
#   include <ai/agent_system_variables.hpp>
#   include <ai/agent_state_variables.hpp>
#   include <ai/action_controller.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>
#   include <unordered_set>
#   include <deque>

namespace ai {


struct  cortex_robot : public cortex_mock_optional
{
    cortex_robot(agent const*  myself_, bool const  use_mock_);
    void  next_round(float_32_bit const  time_step_in_seconds) override;

private:
    struct  state_data
    {
        motion_desire_props  desire;
        agent_action_const_ptr  action;
        agent_system_state  system_state;
        agent_system_variables  system_variables;
        agent_state_variables  state_variables;
        float_32_bit  seconds_consumed;
        float_32_bit  time_step_in_seconds;
    };

    void  insert_initial_node(float_32_bit const  time_step_in_seconds);
    void  insert_child_nodes(natural_32_bit const  node_index);
    float_32_bit  compute_penalty(state_data const&  state) const;
    natural_32_bit  find_best_node_index() const;
    void  insert_node(state_data const&  state, natural_32_bit const  parent_node_index);
    void  clear();

    std::vector<state_data>  m_states;
    std::vector<natural_32_bit>  m_parents;
    std::vector<std::vector<natural_32_bit> >  m_children;
    std::vector<float_32_bit>  m_penalties;
    std::unordered_set<natural_32_bit>  m_leaves;
    std::deque<natural_32_bit>  m_queue;
};


}

#endif
