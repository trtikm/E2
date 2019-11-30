#ifndef AI_ACTION_REGULATOR_HPP_INCLUDED
#   define AI_ACTION_REGULATOR_HPP_INCLUDED

#   include <ai/skeletal_motion_templates.hpp>
#   include <angeo/tensor_math.hpp>
#   include <vector>


namespace ai {


struct  action_controller;


struct  action_regulator
{
    explicit  action_regulator(action_controller const* const  controller_ptr);

    natural_32_bit  choose_next_motion_action(
            natural_32_bit const  index_chosen_by_cortex,
            std::vector<skeletal_motion_templates::transition_info> const&  possibilities
            );

private:
    action_controller const*  m_controller;
};


}

#endif
